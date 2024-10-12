#pragma once

#include "FlowYapEntity.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"

#include "FlowYapCharacter.generated.h"

enum class EFlowYapCharacterMood : uint8;

UCLASS()
class FLOWYAP_API UFlowYapCharacter : public UFlowYapEntity
{
	GENERATED_BODY()
public:
	UFlowYapCharacter();

protected:
	/** Avatar icons to use in dialogue UI, the "calm" value can be considered as default */
	UPROPERTY(EditAnywhere, EditFixedSize, meta=(ReadOnlyKeys))
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
