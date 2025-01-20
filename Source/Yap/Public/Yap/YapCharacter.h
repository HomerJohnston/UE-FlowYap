// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapEntity.h"

#include "YapCharacter.generated.h"

enum class EFlowYapCharacterMood : uint8;

#define LOCTEXT_NAMESPACE "Yap"

// TODO data validation - on packaging make sure the portraits key tags list matches project settings
// TODO add validation warning to the details customization
// TODO add a "skip warning" bool to portrait entries, to make it allowable for them to be unset (on packaging, any unset textures should, by default, log a warning message)
UCLASS(meta = (DataAssetCategory = "LOLBALLS"))
class YAP_API UYapCharacter : public UYapEntity
{
#if WITH_EDITOR
	friend class FDetailCustomization_YapCharacter;
#endif

	GENERATED_BODY()
public:
	UYapCharacter();

protected:
	/** Avatar icons to use in dialogue UI, the "calm" value can be considered as default */ // TODO add EditFixedSize and build details customization with a button to reset the list to match project settings
	UPROPERTY(EditAnywhere, EditFixedSize, meta=(ReadOnlyKeys, ForceInlineRow))
	TMap<FName, TObjectPtr<UTexture2D>> Portraits;
	
public:
	const TMap<FName, TObjectPtr<UTexture2D>>& GetPortraits() const;

	UFUNCTION(BlueprintCallable)
	const UTexture2D* GetPortraitTexture(const FGameplayTag& MoodTag) const;
	
#if WITH_EDITOR
public:
	void PostLoad() override;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void RefreshPortraitList();
#endif

};

#undef LOCTEXT_NAMESPACE