// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapFragment.h"

#include "Yap/YapCharacter.h"
#include "Yap/YapCondition.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapStreamableManager.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapLoadContext.h"
#include "Yap/Enums/YapMissingAudioErrorLevel.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

FYapFragment::FYapFragment()
{
	Guid = FGuid::NewGuid();
	TimeMode = EYapTimeMode::Default;
}

bool FYapFragment::CheckConditions() const
{
	for (TObjectPtr<UYapCondition> Condition : Conditions)
	{
		if (!IsValid(Condition))
		{
			UE_LOG(LogYap, Warning, TEXT("Ignoring null condition. Clean this up!")); // TODO more info
			continue;
		}
		if (!Condition->EvaluateCondition())
		{
			return false;
		}
	}
	
	return true;
}

void FYapFragment::ResetOptionalPins()
{
	bShowOnStartPin = false;
	bShowOnEndPin = false;
}

void FYapFragment::PreloadContent(EYapMaturitySetting MaturitySetting, EYapLoadContext LoadContext)
{
	ResolveMaturitySetting(MaturitySetting);
	
	switch (LoadContext)
	{
		case EYapLoadContext::Async:
		{
			SpeakerHandle = FYapStreamableManager::Get().RequestAsyncLoad(SpeakerAsset.ToSoftObjectPath());;
			DirectedAtHandle = FYapStreamableManager::Get().RequestAsyncLoad(DirectedAtAsset.ToSoftObjectPath());;
			break;
		}
		case EYapLoadContext::AsyncEditorOnly:
		{
			FYapStreamableManager::Get().RequestAsyncLoad(SpeakerAsset.ToSoftObjectPath());;
			FYapStreamableManager::Get().RequestAsyncLoad(DirectedAtAsset.ToSoftObjectPath());;
			break;
		}
		case EYapLoadContext::Sync:
		{
			SpeakerHandle = FYapStreamableManager::Get().RequestSyncLoad(SpeakerAsset.ToSoftObjectPath());;
			DirectedAtHandle = FYapStreamableManager::Get().RequestSyncLoad(DirectedAtAsset.ToSoftObjectPath());;
			break;
		}
	}
	
	// TODO I need some way for Yap to act upon the user changing their maturity setting. Broker needs an "OnMaturitySettingChanged" delegate?
	
	if (MaturitySetting == EYapMaturitySetting::ChildSafe && bEnableChildSafe)
	{
		ChildSafeBit.LoadContent(LoadContext);
	}
	else
	{
		MatureBit.LoadContent(LoadContext);
	}
}

const UYapCharacter* FYapFragment::GetSpeaker(EYapLoadContext LoadContext)
{
	return GetCharacter_Internal(SpeakerAsset, SpeakerHandle, LoadContext);
}

const UYapCharacter* FYapFragment::GetDirectedAt(EYapLoadContext LoadContext)
{
	return GetCharacter_Internal(DirectedAtAsset, DirectedAtHandle, LoadContext);
}

const UYapCharacter* FYapFragment::GetCharacter_Internal(const TSoftObjectPtr<UYapCharacter>& CharacterAsset, TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext)
{
	if (CharacterAsset.IsNull())
	{
		return nullptr;
	}

	if (CharacterAsset.IsValid())
	{
		return CharacterAsset.Get();
	}

	switch (LoadContext)
	{
		case EYapLoadContext::Async:
		{
			Handle = FYapStreamableManager::Get().RequestAsyncLoad(CharacterAsset.ToSoftObjectPath());
			break;
		}
		case EYapLoadContext::AsyncEditorOnly:
		{
			FYapStreamableManager::Get().RequestAsyncLoad(CharacterAsset.ToSoftObjectPath());
			break;
		}
		case EYapLoadContext::Sync:
		{
			Handle = FYapStreamableManager::Get().RequestSyncLoad(CharacterAsset.ToSoftObjectPath());
			break;
		}
	}

	return CharacterAsset.Get();
}

const FYapBit& FYapFragment::GetBit() const
{
	return GetBit(UYapSubsystem::GetCurrentMaturitySetting());
}

const FYapBit& FYapFragment::GetBit(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);

	if (MaturitySetting == EYapMaturitySetting::ChildSafe)
	{
		return ChildSafeBit;
	}
	
	return MatureBit;
}

TOptional<float> FYapFragment::GetTime() const
{
	return GetTime(UYapSubsystem::GetCurrentMaturitySetting(), EYapLoadContext::Sync);
}

TOptional<float> FYapFragment::GetTime(EYapMaturitySetting MaturitySetting, EYapLoadContext LoadContext) const
{
	EYapTimeMode EffectiveTimeMode = GetTimeMode(MaturitySetting);
	return GetBit(MaturitySetting).GetTime(EffectiveTimeMode, LoadContext);
}

float FYapFragment::GetPaddingToNextFragment() const
{
	if (IsTimeModeNone())
	{
		return 0;
	}
	
	if (PaddingToNextFragment < 0)
	{
		return UYapProjectSettings::GetDefaultFragmentPaddingTime();
	}
	
	return FMath::Max(PaddingToNextFragment, 0);
}

void FYapFragment::IncrementActivations()
{
	ActivationCount++;
}

/*
void FYapFragment::ReplaceBit(EYapMaturitySetting MaturitySetting, const FYapBitReplacement& ReplacementBit)
{
	// TODO: I'd rather use some sort of layers system rather than bulldoze the original data.
	//Bit = ReplacementBit;
}
*/

FFlowPin FYapFragment::GetPromptPin() const
{
	if (!PromptPin.IsValid())
	{
		FYapFragment* MutableThis = const_cast<FYapFragment*>(this);
		MutableThis->PromptPin = FName("Prompt_" + Guid.ToString());
		MutableThis->PromptPin.PinToolTip = "Out";
	}
	
	return PromptPin;
}

FFlowPin FYapFragment::GetEndPin() const
{
	if (!EndPin.IsValid())
	{
		FYapFragment* MutableThis = const_cast<FYapFragment*>(this);
		MutableThis->EndPin = FName("End_" + Guid.ToString());
		MutableThis->PromptPin.PinToolTip = "Runs before end-padding time begins";
	}
	
	return EndPin;
}

FFlowPin FYapFragment::GetStartPin() const
{
	if (!StartPin.IsValid())
	{
		FYapFragment* MutableThis = const_cast<FYapFragment*>(this);
		MutableThis->StartPin = FName("Start_" + Guid.ToString());
		MutableThis->StartPin.PinToolTip = "Runs when fragment starts playback";
	}
	
	return StartPin;
}

void FYapFragment::ResolveMaturitySetting(EYapMaturitySetting& MaturitySetting) const
{
	if (!bEnableChildSafe)
	{
		MaturitySetting = EYapMaturitySetting::Mature;
		return;
	}
	
	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		if (IsValid(UYapSubsystem::Get()))
		{
			MaturitySetting = UYapSubsystem::GetCurrentMaturitySetting();
		}
		else
		{
			MaturitySetting = EYapMaturitySetting::Mature;
		}
	}
}

bool FYapFragment::GetSkippable(bool Default) const
{
	return Skippable.Get(Default);
}

bool FYapFragment::GetAutoAdvance(bool Default) const
{
	if (TimeMode == EYapTimeMode::None || (TimeMode == EYapTimeMode::Default && UYapProjectSettings::GetDefaultTimeModeSetting() == EYapTimeMode::None))
	{
		return false;
	}
	
	return AutoAdvance.Get(Default);
}

EYapTimeMode FYapFragment::GetTimeMode() const
{
	return GetTimeMode(UYapSubsystem::GetCurrentMaturitySetting());
}

EYapTimeMode FYapFragment::GetTimeMode(EYapMaturitySetting MaturitySetting) const
{
	EYapTimeMode EffectiveTimeMode = (TimeMode == EYapTimeMode::Default) ? UYapProjectSettings::GetDefaultTimeModeSetting() : TimeMode;

	if (EffectiveTimeMode == EYapTimeMode::AudioTime)
	{
		if (!GetBit(MaturitySetting).HasAudioAsset())
		{
			EYapMissingAudioErrorLevel MissingAudioBehavior = UYapProjectSettings::GetMissingAudioBehavior();
			
			if (MissingAudioBehavior == EYapMissingAudioErrorLevel::Error)
			{
				// Help force developers to notice and assign missing audio assets, by hindering or preventing dialogue progression
				EffectiveTimeMode = EYapTimeMode::None;
			}
			else if (MissingAudioBehavior == EYapMissingAudioErrorLevel::Warning)
			{
				EffectiveTimeMode = EYapTimeMode::TextTime;
			}
			else
			{
				EffectiveTimeMode = EYapTimeMode::TextTime;
			}
		}
	}

	return EffectiveTimeMode;
}

bool FYapFragment::IsTimeModeNone() const
{
	return TimeMode == EYapTimeMode::None;
}

bool FYapFragment::HasAudio() const
{
	return MatureBit.HasAudioAsset() || ChildSafeBit.HasAudioAsset();
}

#if WITH_EDITOR

FYapBit& FYapFragment::GetBitMutable(EYapMaturitySetting MaturitySetting)
{
	if (MaturitySetting == EYapMaturitySetting::ChildSafe && bEnableChildSafe)
	{
		return ChildSafeBit;
	}
	else
	{
		return MatureBit;
	}
}

void FYapFragment::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString)
{
	if (!PropertyHandle || PropertyHandle->GetProperty()->GetFName() != GET_MEMBER_NAME_CHECKED(FYapFragment, FragmentTag))
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* Object : OuterObjects)
	{
		const UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(Object);

		if (!DialogueNode)
		{
			continue;
		}
		
		if (DialogueNode->IsPlayerPrompt())
		{
			MetaString = DialogueNode->GetDialogueTag().ToString();
		}
	}
}
#endif

#if WITH_EDITOR
void FYapFragment::InvalidateFragmentTag(UFlowNode_YapDialogue* OwnerNode)
{
	FragmentTag = FGameplayTag::EmptyTag;
}
#endif


#if WITH_EDITOR
void FYapFragment::SetSpeaker(TSoftObjectPtr<UYapCharacter> InCharacter)
{
	SpeakerAsset = InCharacter;
	SpeakerHandle = nullptr;
}

void FYapFragment::SetDirectedAt(TSoftObjectPtr<UYapCharacter> InDirectedAt)
{
	DirectedAtAsset = InDirectedAt;
	DirectedAtHandle = nullptr;
}
#endif

#undef LOCTEXT_NAMESPACE