

#include "Yap/NodeWidgets/SFlowYapGraphPinExec.h"

#include "SLevelOfDetailBranchNode.h"
#include "SPinTypeSelector.h"

void SFlowYapGraphPinExec::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	SGraphPinExec::Construct(SGraphPinExec::FArguments(), InPin);
	bUsePinColorForText = true;

	/*
	//bUsePinColorForText = 
	this->SetCursor(EMouseCursor::Default);

	//SetVisibility(MakeAttributeSP(this, &SGraphPin::GetPinVisiblity));

	GraphPinObj = InPin;
	check(GraphPinObj != NULL);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	checkf(
		Schema, 
		TEXT("Missing schema for pin: %s with outer: %s of type %s"), 
		*(GraphPinObj->GetName()),
		GraphPinObj->GetOuter() ? *(GraphPinObj->GetOuter()->GetName()) : TEXT("NULL OUTER"), 
		GraphPinObj->GetOuter() ? *(GraphPinObj->GetOuter()->GetClass()->GetName()) : TEXT("NULL OUTER")
	);

	const bool bIsInput = (GetDirection() == EGPD_Input);

	// Create the pin icon widget
	TSharedRef<SWidget> PinWidgetRef = SPinTypeSelector::ConstructPinTypeImage(
		MakeAttributeSP(this, &SGraphPin::GetPinIcon ),
		MakeAttributeSP(this, &SGraphPin::GetPinColor),
		MakeAttributeSP(this, &SGraphPin::GetSecondaryPinIcon),
		MakeAttributeSP(this, &SGraphPin::GetSecondaryPinColor));
	PinImage = PinWidgetRef;

	PinWidgetRef->SetCursor( 
		TAttribute<TOptional<EMouseCursor::Type> >::Create (
			TAttribute<TOptional<EMouseCursor::Type> >::FGetter::CreateRaw( this, &SGraphPin::GetPinCursor )
		)
	);

	// Create the pin indicator widget (used for watched values)
	static const FName NAME_NoBorder("NoBorder");
	TSharedRef<SWidget> PinStatusIndicator =
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), NAME_NoBorder)
		.Visibility(this, &SGraphPin::GetPinStatusIconVisibility)
		.ContentPadding(0)
		.OnClicked(this, &SGraphPin::ClickedOnPinStatusIcon)
		[
			SNew(SImage)
			.Image(this, &SGraphPin::GetPinStatusIcon)
		];

	// Create the widget used for the pin body (status indicator, label, and value)
	LabelAndValue =
		SNew(SWrapBox)
		.PreferredSize(150.f);

	if (!bIsInput)
	{
		LabelAndValue->AddSlot()
			.VAlign(VAlign_Center)
			[
				PinStatusIndicator
			];
	}
	else
	{
		ValueWidget = GetDefaultValueWidget();

		if (ValueWidget != SNullWidget::NullWidget)
		{
			TSharedPtr<SBox> ValueBox;
			LabelAndValue->AddSlot()
				.Padding(bIsInput ? FMargin(0, 0, 0, 0) : FMargin(0, 0, 0, 0))
				.VAlign(VAlign_Center)
				[
					SAssignNew(ValueBox, SBox)
					.Padding(0.0f)
					[
						ValueWidget.ToSharedRef()
					]
				];

			if (!DoesWidgetHandleSettingEditingEnabled())
			{
				ValueBox->SetEnabled(TAttribute<bool>(this, &SGraphPin::IsEditingEnabled));
			}
		}

		LabelAndValue->AddSlot()
			.VAlign(VAlign_Center)
			[
				PinStatusIndicator
			];
	}

	TSharedPtr<SHorizontalBox> PinContent;
	if (bIsInput)
	{
		// Input pin
		FullPinHorizontalRowWidget = PinContent = 
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 0, 0)
			[
				PinWidgetRef
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				LabelAndValue.ToSharedRef()
			];
	}
	else
	{
		// Output pin
		FullPinHorizontalRowWidget = PinContent = SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				LabelAndValue.ToSharedRef()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 0, 0)
			[
				PinWidgetRef
			];
	}

	// Set up a hover for pins that is tinted the color of the pin.
	
	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &SGraphPin::GetPinBorder)
		.BorderBackgroundColor(this, &SGraphPin::GetHighlightColor)
		.OnMouseButtonDown(this, &SGraphPin::OnPinNameMouseDown)
		[
			SNew(SBorder)
			.BorderImage(CachedImg_Pin_DiffOutline)
			.BorderBackgroundColor(this, &SGraphPin::GetPinDiffColor)
			[
				SNew(SLevelOfDetailBranchNode)
				.UseLowDetailSlot(this, &SGraphPin::UseLowDetailPinNames)
				.LowDetail()
				[
					//@TODO: Try creating a pin-colored line replacement that doesn't measure text / call delegates but still renders
					PinWidgetRef
				]
				.HighDetail()
				[
					PinContent.ToSharedRef()
				]
			]
		]
	);

	TSharedPtr<IToolTip> TooltipWidget = SNew(SToolTip)
		.Text(this, &SGraphPin::GetTooltipText);

	SetToolTip(TooltipWidget);
	*/
	const FSlateBrush* Temp = FAppStyle::Get().GetBrush("Icons.Minus");
	
	Test.ImageSize = FVector2D(12, 4);

	Test.SetResourceObject(Temp->GetResourceObject());
	//PortraitBrush.SetUVRegion(FBox2D(FVector2D(0.125,0), FVector2D(0.875,1)));
	Test.SetUVRegion(FBox2D(FVector2D(0,0), FVector2D(1,1)));
	Test.DrawAs = ESlateBrushDrawType::Border;
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

const FSlateBrush* SFlowYapGraphPinExec::GetPinIcon() const
{
	// TODO Test
	return SFlowGraphPinExec::GetPinIcon();
}

