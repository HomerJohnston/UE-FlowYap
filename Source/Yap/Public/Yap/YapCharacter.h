#pragma once

#include "YapEntity.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"

#include "YapCharacter.generated.h"

enum class EFlowYapCharacterMood : uint8;

// TODO data validation - on packaging make sure the portraits key tags list matches project settings
// TODO add a bool to portrait keys to make it allowable for them to be unset (on packaging, any unset textures should, by default, log a warning message)
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
	const FSlateBrush& GetPortraitBrush(const FGameplayTag& MoodKey) const;
	
#if WITH_EDITORONLY_DATA
protected:
	TMap<FName, FSlateBrush> PortraitBrushes;
#endif
	
#if WITH_EDITOR
public:
	void PostLoad() override;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	const TMap<FName, FSlateBrush>& GetPortraitBrushes();

	void RefreshPortraitList();
	
private:
	void RebuildPortraitBrushes();
#endif

};
