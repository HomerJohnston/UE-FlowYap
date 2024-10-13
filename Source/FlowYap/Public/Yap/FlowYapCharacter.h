#pragma once

#include "FlowYapEntity.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"

#include "FlowYapCharacter.generated.h"

enum class EFlowYapCharacterMood : uint8;

// TODO data validation - on packaging make sure the portraits key tags list matches project settings
// TODO add a bool to portrait keys to make it allowable for them to be unset (on packaging, any unset textures should, by default, log a warning message)
UCLASS()
class FLOWYAP_API UFlowYapCharacter : public UFlowYapEntity
{
	GENERATED_BODY()
public:
	UFlowYapCharacter();
	
	
protected:
	/** Avatar icons to use in dialogue UI, the "calm" value can be considered as default */ // TODO add EditFixedSize and build details customization with a button to reset the list to match project settings
	UPROPERTY(EditAnywhere, /*EditFixedSize,*/ meta=(ReadOnlyKeys))
	TMap<FGameplayTag, TObjectPtr<UTexture2D>> Portraits;

public:
	const TMap<FGameplayTag, TObjectPtr<UTexture2D>>& GetPortraits() const;
	
#if WITH_EDITORONLY_DATA
protected:
	TMap<FGameplayTag, FSlateBrush> PortraitBrushes;
#endif
	
#if WITH_EDITOR
public:
	void PostLoad() override;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	const TMap<FGameplayTag, FSlateBrush>& GetPortraitBrushes();

	const FSlateBrush* GetPortraitBrush(const FGameplayTag& MoodKey) const;

private:
	void RebuildPortraitBrushes();
#endif

};
