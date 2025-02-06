// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapFragment.h"

#include "Yap/YapCharacter.h"
#include "Yap/YapCondition.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapStreamableManager.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapMissingAudioErrorLevel.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

#define YAP_ASYNC_LOAD(ASSET, HANDLE)\
if (ASSET.IsPending())\
HANDLE = FYapStreamableManager::Get().RequestAsyncLoad(ASSET.ToSoftObjectPath());\

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

void FYapFragment::PreloadContent(UFlowNode_YapDialogue* OwningContext)
{
	YAP_ASYNC_LOAD(SpeakerAsset, SpeakerHandle);
	
	YAP_ASYNC_LOAD(DirectedAtAsset, DirectedAtHandle);

	UWorld* World = OwningContext->GetWorld();

	// TODO I need some way for Yap to act upon the user changing their maturity setting. Broker needs an "OnMaturitySettingChanged" delegate?
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
	{
		EYapMaturitySetting MaturitySetting = UYapSubsystem::GetCurrentMaturitySetting();

		if (MaturitySetting == EYapMaturitySetting::ChildSafe && bEnableChildSafe)
		{
			ChildSafeBit.AsyncLoadContent(OwningContext);
		}
		else
		{
			MatureBit.AsyncLoadContent(OwningContext);
		}
		
	}
}

const UYapCharacter* FYapFragment::GetSpeaker()
{
	return GetCharacter_Internal(SpeakerAsset, SpeakerHandle);
}

const UYapCharacter* FYapFragment::GetDirectedAt()
{
	return GetCharacter_Internal(DirectedAtAsset, DirectedAtHandle);
}

const UYapCharacter* FYapFragment::GetCharacter_Internal(const TSoftObjectPtr<UYapCharacter>& CharacterAsset, TSharedPtr<FStreamableHandle>& Handle)
{
	if (CharacterAsset.IsNull())
	{
		return nullptr;
	}

	if (CharacterAsset.IsValid())
	{
		return CharacterAsset.Get();
	}
	
	// If we're mid-game, force a sync load
	TWeakObjectPtr<UWorld> World = UYapSubsystem::GetStaticWorld();
	
	if (World.IsValid() && World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE)
	{
		if (Handle->IsLoadingInProgress())
		{
			UE_LOG(LogYap, Warning, TEXT("Interrupting async load to get yap character asset"));
		}
		
		Handle = FYapStreamableManager::Get().RequestSyncLoad(CharacterAsset.ToSoftObjectPath());
		UE_LOG(LogYap, Warning, TEXT("Synchronously loaded character: %s"), *CharacterAsset->GetName());
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
	return GetTime(UYapSubsystem::GetCurrentMaturitySetting());
}

TOptional<float> FYapFragment::GetTime(EYapMaturitySetting MaturitySetting) const
{
	return GetBit(MaturitySetting).GetTime(GetTimeMode());
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
			UE_LOG(LogYap, Error, TEXT("UYapSubsystem was invalid in FYapBit::ResolveMaturitySetting. This should not happen! Please contact plugin author. Defaulting to mature."));
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

/*
#if WITH_EDITOR
TArray<FFlowPin> FYapFragment::GetOutputPins() const
{
	TArray<FFlowPin> OutPins;
	
	if (Owner.IsValid() && Owner->IsPlayerPrompt())
	{
		OutPins.Add(PromptPin);
	}
	
	if (UsesEndPin())
	{
		OutPins.Add(EndPin);
	}
	
	if (UsesStartPin())
	{
		OutPins.Add(StartPin);
	}
	
	return OutPins;
}
#endif
*/

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