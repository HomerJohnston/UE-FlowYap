// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapCharacter.h"

#include "Yap/YapProjectSettings.h"

#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "Yap"

UYapCharacter::UYapCharacter()
{
}

const TMap<FName, TObjectPtr<UTexture2D>>& UYapCharacter::GetPortraits() const
{
	return Portraits;
}

const UTexture2D* UYapCharacter::GetPortraitTexture(const FGameplayTag& MoodTag) const
{
	FGameplayTag MoodTagToFind = MoodTag.IsValid() ? MoodTag : UYapProjectSettings::GetDefaultMoodTag();

	const TObjectPtr<UTexture2D>* TexturePtr = Portraits.Find(MoodTagToFind.GetTagName());

	return TexturePtr ? *TexturePtr : nullptr;
}

#if WITH_EDITOR
void UYapCharacter::PostLoad()
{
	Super::PostLoad();
}
#endif

#if WITH_EDITOR
void UYapCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#if WITH_EDITOR
// TODO I need validation code to check if the character's portrait keys array matches the project or not to throw warnings during packaging?
void UYapCharacter::RefreshPortraitList()
{
	FGameplayTagContainer MoodKeys = UYapProjectSettings::GetMoodTags();

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
}
#endif

#undef LOCTEXT_NAMESPACE