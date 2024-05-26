#include "FlowYapEditorSubsystem.h"

#include "FlowYap/FlowYapProjectSettings.h"
#include "ImageUtils.h"
#include "FlowYap/FlowYapCharacter.h"
#include "FlowYap/FlowYapEngineUtils.h"

#if WITH_EDITOR
void UFlowYapEditorSubsystem::UpdatePortraitKeyIconsMap()
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();

	const TArray<FName>& PortraitKeys = ProjectSettings->GetPortraitKeys();

	PortraitKeyIcons.Empty(PortraitKeys.Num());

	for (const FName& PortraitKey : PortraitKeys)
	{
		if (PortraitKey.IsNone())
		{
			continue;
		}
		
		FString IconPath = ProjectSettings->GetPortraitIconPath(PortraitKey);
		UTexture2D* PortraitKeyIcon = FImageUtils::ImportFileAsTexture2D(IconPath);

		if (!IsValid(PortraitKeyIcon))
		{
			continue;
		}

		PortraitKeyIcons.Add(PortraitKey, PortraitKeyIcon);
	}
}
#endif

#if WITH_EDITOR
UTexture2D* UFlowYapEditorSubsystem::GetPortraitKeyIcon(FName PortraitKey)
{
	UTexture2D** Texture = PortraitKeyIcons.Find(PortraitKey);

	if (Texture)
	{
		return *Texture;
	}

	return nullptr;
}
#endif

void UFlowYapEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UFlowYapProjectSettings* ProjectSettings = GetMutableDefault<UFlowYapProjectSettings>();
	ProjectSettings->OnPortraitKeysChanged.AddUObject(this, &ThisClass::UpdatePortraitKeyIconsMap);

	UpdatePortraitKeyIconsMap();
}

void UFlowYapEditorSubsystem::Deinitialize()
{
	UFlowYapProjectSettings* ProjectSettings = GetMutableDefault<UFlowYapProjectSettings>();
	ProjectSettings->OnPortraitKeysChanged.RemoveAll(this);
	
	Super::Deinitialize();
}
