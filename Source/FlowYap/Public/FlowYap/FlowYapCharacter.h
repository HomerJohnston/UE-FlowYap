#pragma once

#include "FlowYapEntity.h"

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
	TMap<FName, TObjectPtr<UTexture2D>> Portraits;

public:
	const TMap<FName, TObjectPtr<UTexture2D>> GetPortraits();
	

#if WITH_EDITORONLY_DATA
protected:
	TMap<FName, FSlateBrush> PortraitBrushes;
#endif
	
#if WITH_EDITOR
public:
	void PostLoad() override;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	TMap<FName, FSlateBrush> GetPortraitBrushes();

	FSlateBrush* GetPortraitBrush(const FName& PortraitKey);

private:
	void RebuildPortraitBrushes();
#endif

};