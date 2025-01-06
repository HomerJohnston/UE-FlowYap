// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapCharacter.h"

#include "Yap/YapProjectSettings.h"

#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UYapCharacter::UYapCharacter()
{
}

const TMap<FName, TObjectPtr<UTexture2D>>& UYapCharacter::GetPortraits() const
{
	return Portraits;
}

const FSlateBrush& UYapCharacter::GetPortraitBrush(const FGameplayTag& MoodKey) const
{
	FGameplayTag MoodKeyToUse = MoodKey.IsValid() ? MoodKey : UYapProjectSettings::Get()->GetDefaultMoodTag();
	
	FName MoodKeyAsName = MoodKeyToUse.GetTagName();
	
	const FSlateBrush* Brush = PortraitBrushes.Find(MoodKeyAsName);

	if (Brush)
	{
		return *Brush;
	}

	return UYapProjectSettings::Get()->GetMissingPortraitBrush();
}

#if WITH_EDITOR
void UYapCharacter::PostLoad()
{
	Super::PostLoad();

	RebuildPortraitBrushes();
}
#endif

#if WITH_EDITOR
void UYapCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, Portraits))
	{
		RebuildPortraitBrushes();
	}
}
#endif

#if WITH_EDITOR
const TMap<FName, FSlateBrush>& UYapCharacter::GetPortraitBrushes()
{
	return PortraitBrushes;
}
#endif

#if WITH_EDITOR
// TODO I need validation code to check if the character's portrait keys array matches the project or not to throw warnings during packaging?
void UYapCharacter::RefreshPortraitList()
{
	const UYapProjectSettings* Settings = UYapProjectSettings::Get();

	FGameplayTagContainer MoodKeys = Settings->GetMoodTags();

	TSet<FName> MoodKeysAsNames;
	TSet<FName> CharacterMoodKeysAsNames;

	for (const FGameplayTag& Tag : MoodKeys)
	{
		MoodKeysAsNames.Add(Tag.GetTagName());
	}
	
	// Iterate through all existing keys. Remove any which are no longer in use.
	for (auto It = Portraits.CreateIterator(); It; ++It)
	{
		FName MoodKey = It.Key();
		
		if (!MoodKeysAsNames.Contains(MoodKey))
		{
			It.RemoveCurrent();
		}
	}

	// Iterate through all project keys. Add any which are missing.
	for (FName MoodKey : MoodKeysAsNames)
	{
		if (!Portraits.Contains(MoodKey))
		{
			Portraits.Add(MoodKey, nullptr);
		}
	}

	// Sort the map for better display.
	Portraits.KeySort(FNameLexicalLess());
	/*
	Portraits.KeySort([](const FName& A, const FName& B)
	{
		return A.Compare(B);
	});
	*/
	/*
	{
		FGameplayTagContainer MoodKeys = Settings->GetMoodTags();
		
		for (const FGameplayTag& MoodKey : MoodKeys)
		{
			FName MoodKeyAsName = MoodKey.GetTagName();

			if (!MoodKey.IsValid())
			{
				UE_LOG(LogYap, Warning, TEXT("Warning: Portrait keys contains a 'NONE' entry. Clean this up!"));
				continue;
			}
			
			Portraits.Add(MoodKeyAsName, nullptr);
		}
	}
	*/
}
#endif

#if WITH_EDITOR
void UYapCharacter::RebuildPortraitBrushes()
{
	PortraitBrushes.Empty(Portraits.Num());
	
	for (const TPair<FName, TObjectPtr<UTexture2D>>& PortraitsKVP : Portraits)
	{
		FName MoodKeyAsName = PortraitsKVP.Key;

		UTexture2D* Portrait = PortraitsKVP.Value;

		FSlateBrush PortraitBrush;
		PortraitBrush.ImageSize = FVector2D(64,64);
		
		if (Portrait)
		{
			PortraitBrush.SetResourceObject(Portrait);
			//PortraitBrush.SetUVRegion(FBox2D(FVector2D(0.125,0), FVector2D(0.875,1)));
			PortraitBrush.SetUVRegion(FBox2D(FVector2D(0,0), FVector2D(1,1)));
			PortraitBrush.DrawAs = ESlateBrushDrawType::Box;
			PortraitBrush.Margin = 0;
		}
		else
		{
			// TODO
			PortraitBrush.TintColor = FLinearColor::Transparent;
		}

		PortraitBrushes.Add(MoodKeyAsName, PortraitBrush);
	}
}
#endif

#undef LOCTEXT_NAMESPACE