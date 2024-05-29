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

	PortraitKeyIconTextures.Empty(PortraitKeys.Num());

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

		PortraitKeyIconTextures.Add(PortraitKey, PortraitKeyIcon);

		TSharedPtr<FSlateBrush> PortraitKeyBrush = MakeShareable(new FSlateBrush);

		PortraitKeyBrush->SetResourceObject(PortraitKeyIcon);
		PortraitKeyBrush->SetImageSize({16, 16});
		
		PortraitKeyBrushes.Add(PortraitKey, PortraitKeyBrush);
	}
}
#endif

#if WITH_EDITOR
UTexture2D* UFlowYapEditorSubsystem::GetPortraitKeyIcon(FName PortraitKey)
{
	UTexture2D** Texture = PortraitKeyIconTextures.Find(PortraitKey);

	if (Texture)
	{
		return *Texture;
	}

	return nullptr;
}

const FSlateBrush* UFlowYapEditorSubsystem::GetPortraitKeyBrush(FName Name)
{
	TSharedPtr<FSlateBrush>* Brush = PortraitKeyBrushes.Find(Name);

	return Brush ? Brush->Get() : nullptr;
}

UTexture2D* UFlowYapEditorSubsystem::GetDialogueTimerIco()
{
	return DialogueTimerIco;
}

UTexture2D* UFlowYapEditorSubsystem::GetDialogueUserInterruptIco()
{
	return DialogueUserInterruptIco;
}
#endif

void UFlowYapEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UFlowYapProjectSettings* ProjectSettings = GetMutableDefault<UFlowYapProjectSettings>();
	ProjectSettings->OnPortraitKeysChanged.AddUObject(this, &ThisClass::UpdatePortraitKeyIconsMap);

	UpdatePortraitKeyIconsMap();

	FString DialogueNode_IconTimerPath = FFlowYapEngineUtils::GetFlowYapPluginDir() / "Resources/DialogueNodeIcons/Icon_Timer_16x16.png";
	DialogueTimerIco = FImageUtils::ImportFileAsTexture2D(DialogueNode_IconTimerPath);
	
	FString DialogueNode_IconUserInterruptPath = FFlowYapEngineUtils::GetFlowYapPluginDir() / "Resources/DialogueNodeIcons/Icon_UserInterrupt_16x16.png";
	DialogueUserInterruptIco = FImageUtils::ImportFileAsTexture2D(DialogueNode_IconUserInterruptPath);
}

void UFlowYapEditorSubsystem::Deinitialize()
{
	UFlowYapProjectSettings* ProjectSettings = GetMutableDefault<UFlowYapProjectSettings>();
	ProjectSettings->OnPortraitKeysChanged.RemoveAll(this);
	
	Super::Deinitialize();
}
