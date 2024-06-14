// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYap/FlowYapCharacter.h"

#include "FlowYap/FlowYapProjectSettings.h"
#include "Engine/Texture2D.h"

#include "FlowYap/FlowYapLog.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowYapCharacter::UFlowYapCharacter()
{
	const UFlowYapProjectSettings* Settings = UFlowYapProjectSettings::Get();

	// TODO I need a details customization to add a button to "rebuild" the portraits array
	// TODO I need validation code to check if the character's portrait keys array matches the project or not
	if (Portraits.Num() == 0)
	{
		for (FName MoodKey : Settings->GetMoodKeys())
		{
			if (MoodKey == NAME_None)
			{
				UE_LOG(FlowYap, Warning, TEXT("Warning: Portrait keys contains a 'NONE' entry. Clean this up!"));
				continue;
			}
			
			Portraits.Add(MoodKey, nullptr);
		}
	}
}

const TMap<FName, TObjectPtr<UTexture2D>> UFlowYapCharacter::GetPortraits()
{
	return Portraits;
}

#if WITH_EDITOR
void UFlowYapCharacter::PostLoad()
{
	Super::PostLoad();

	RebuildPortraitBrushes();
}

void UFlowYapCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property->GetFName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, Portraits))
	{
		RebuildPortraitBrushes();
	}
}

TMap<FName, FSlateBrush> UFlowYapCharacter::GetPortraitBrushes()
{
	return PortraitBrushes;
}

const FSlateBrush* UFlowYapCharacter::GetPortraitBrush(const FName& MoodKey) const
{
	const FSlateBrush* Brush = PortraitBrushes.Find(MoodKey);

	if (Brush)
	{
		return Brush;
	}

	// TODO This should only occur when the character is out of date, should I return a default or a null texture? Maybe a project setting to determine what to do?
	
	return nullptr;
}

void UFlowYapCharacter::RebuildPortraitBrushes()
{
	PortraitBrushes.Empty(Portraits.Num());
	
	for (const TPair<FName, TObjectPtr<UTexture2D>>& PortraitsKVP : Portraits)
	{
		const FName& MoodKey = PortraitsKVP.Key;
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

		PortraitBrushes.Add(MoodKey, PortraitBrush);
	}
}
#endif

#undef LOCTEXT_NAMESPACE