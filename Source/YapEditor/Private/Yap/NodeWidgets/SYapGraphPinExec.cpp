

#include "Yap/NodeWidgets/SYapGraphPinExec.h"

void SYapGraphPinExec::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	SGraphPinExec::Construct(SGraphPinExec::FArguments(), InPin);
	bUsePinColorForText = true;

	// TODO
	
	const FSlateBrush* Temp = FAppStyle::Get().GetBrush("Icons.Minus");
	
	Test.ImageSize = FVector2D(12, 4);

	Test.SetResourceObject(Temp->GetResourceObject());
	//PortraitBrush.SetUVRegion(FBox2D(FVector2D(0.125,0), FVector2D(0.875,1)));
	Test.SetUVRegion(FBox2D(FVector2D(0,0), FVector2D(1,1)));
	Test.DrawAs = ESlateBrushDrawType::Image;
	Test.Margin = 2;
	
	CachedImg_Pin_ConnectedHovered = FAppStyle::GetBrush(TEXT("Icons.Minus"/*"Graph.ExecPin.ConnectedHovered"*/));
	CachedImg_Pin_Connected = FAppStyle::GetBrush(TEXT("Icons.Minus"/*"Graph.ExecPin.Connected"*/));
	CachedImg_Pin_DisconnectedHovered = FAppStyle::GetBrush(TEXT("Icons.Minus"/*"Graph.ExecPin.DisconnectedHovered"*/));
	CachedImg_Pin_Disconnected = FAppStyle::GetBrush(TEXT("Icons.Minus"/*"Graph.ExecPin.Disconnected"*/));

	
	CachedImg_Pin_ConnectedHovered = &Test;
	CachedImg_Pin_Connected = &Test;
	CachedImg_Pin_DisconnectedHovered = &Test;
	CachedImg_Pin_Disconnected = &Test;
}

const FSlateBrush* SYapGraphPinExec::GetPinIcon() const
{
	// TODO Test
	return SFlowGraphPinExec::GetPinIcon();
}

