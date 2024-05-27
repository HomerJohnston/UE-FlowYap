// 2022 The Enby Witch

#include "Widgets/SFlowGraphNode_YapSpeechBaseWidget.h"

#include "AkAudioEvent.h"
#include "GraphNodes/FlowGraphNode_YapSpeechBase.h"
#include "PropertyCustomizationHelpers.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "FlowYapEditorSubsystem.h"
#include "GraphEditorSettings.h"
#include "IDocumentation.h"
#include "FlowYap/FlowYapLog.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

#define LOCTEXT_NAMESPACE "PGFlowEditor"

void SFlowGraphNode_YapSpeechBaseWidget::UpdateGraphNode()
{
	SFlowGraphNode::UpdateGraphNode();
	/*
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	//	     ______________________
	//	    |      TITLE AREA      |
	//	    +-------+------+-------+
	//	    | (>) L |      | R (>) |
	//	    | (>) E |      | I (>) |
	//	    | (>) F |      | G (>) |
	//	    | (>) T |      | H (>) |
	//	    |       |      | T (>) |
	//	    |_______|______|_______|
	//
	TSharedPtr<SVerticalBox> MainVerticalBox;
	SetupErrorReporting();

	const TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	// Get node icon
	IconColor = FLinearColor::White;
	const FSlateBrush* IconBrush = nullptr;
	if (GraphNode && GraphNode->ShowPaletteIconOnNode())
	{
		IconBrush = GraphNode->GetIconAndTint(IconColor).GetOptionalIcon();
	}

	const TSharedRef<SOverlay> DefaultTitleAreaWidget = SNew(SOverlay)
	+ SOverlay::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Center)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FFlowEditorStyle::GetBrush("Flow.Node.Title"))
			// The extra margin on the right is for making the color spill stretch well past the node title
			.Padding(FMargin(10, 5, 30, 3))
			.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 0.f, 4.f, 0.f))
				.AutoWidth()
				[
					SNew(SImage)
					.Image(IconBrush)
					.ColorAndOpacity(this, &SGraphNode::GetNodeTitleIconColor)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						CreateTitleWidget(NodeTitle)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						NodeTitle.ToSharedRef()
					]
				]
			]
		]
	];

	SetDefaultTitleAreaWidget(DefaultTitleAreaWidget);
	
	const TSharedRef<SWidget> TitleAreaWidget = 
		SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(this, &SFlowGraphNode::UseLowDetailNodeTitles)
			.LowDetail()
			[
				SNew(SBorder)
				.BorderImage(FFlowEditorStyle::GetBrush("Flow.Node.Title"))
				.Padding(FMargin(75.0f, 22.0f)) // Saving enough space for a 'typical' title so the transition isn't quite so abrupt
				.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
			]
			.HighDetail()
			[
				DefaultTitleAreaWidget
			];
	
	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("FlowGraphNode"));
	PopulateMetaTag(&TagMeta);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	const TSharedPtr<SVerticalBox> InnerVerticalBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.Padding(Settings->GetNonPinNodeBodyPadding())
			[
				TitleAreaWidget
			]
		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			[
				CreateNodeContentArea()
			];

	const TSharedPtr<SWidget> EnabledStateWidget = GetEnabledStateWidget();
	if (EnabledStateWidget.IsValid())
	{
		InnerVerticalBox->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.Padding(FMargin(2, 0))
		[
			EnabledStateWidget.ToSharedRef()
		];
	}

	InnerVerticalBox->AddSlot()
	.AutoHeight()
	.Padding(Settings->GetNonPinNodeBodyPadding())
	[
		ErrorReporting->AsWidget()
	];

	this->GetOrAddSlot(ENodeZone::Center)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SAssignNew(MainVerticalBox, SVerticalBox)
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.AutoHeight()
		[
			SNew(SOverlay)
			.AddMetaData<FGraphNodeMetaData>(TagMeta)
			+ SOverlay::Slot()
			.Padding(Settings->GetNonPinNodeBodyPadding())
			[
				SNew(SImage)
				.Image(GetNodeBodyBrush())
				.ColorAndOpacity(this, &SGraphNode::GetNodeBodyColor)
			]
			+ SOverlay::Slot()
			[
				InnerVerticalBox.ToSharedRef()
			]
		]
	];

	if (GraphNode && GraphNode->SupportsCommentBubble())
	{
		// Create comment bubble
		TSharedPtr<SCommentBubble> CommentBubble;
		const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

		SAssignNew(CommentBubble, SCommentBubble)
			.GraphNode(GraphNode)
			.Text(this, &SGraphNode::GetNodeComment)
			.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
			.OnToggled(this, &SGraphNode::OnCommentBubbleToggled)
			.ColorAndOpacity(CommentColor)
			.AllowPinning(true)
			.EnableTitleBarBubble(true)
			.EnableBubbleCtrls(true)
			.GraphLOD(this, &SGraphNode::GetCurrentLOD)
			.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

		GetOrAddSlot(ENodeZone::TopCenter)
			.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
			.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
			.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
			.VAlign(VAlign_Top)
			[
				CommentBubble.ToSharedRef()
			];
	}

	CreateBelowWidgetControls(MainVerticalBox);
	CreatePinWidgets();
	CreateInputSideAddButton(LeftNodeBox);
	CreateOutputSideAddButton(RightNodeBox);
	CreateBelowPinControls(InnerVerticalBox);
	CreateAdvancedViewArrow(InnerVerticalBox);
	*/
}

TSharedRef<SWidget> SFlowGraphNode_YapSpeechBaseWidget::CreateDialogueContentArea()
{
	const FFlowYapFragment SpeechElement = FlowNode_YapSpeechBase->GetFragment();

	FText TitleText = SpeechElement.TitleText;

	return SNew(SHorizontalBox)
		// ===================
		// PORTRAIT IMAGE (LEFT SIDE)
		// ===================
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.AutoWidth()
		.Padding(2.f, 5.f, 5.f, 2.f)
		[
			GetPortraitWidget()
		]
		// ===================
		// DIALOGUE SETTINGS (RIGHT SIDE)
		// ===================
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.FillWidth(1.0f)
		.Padding(5.f)
		[
			SNew(SVerticalBox)
			// ===================
			// PORTRAIT KEY (MOOD) SELECTOR
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				GetPortraitKeySelectorWidget()
			]
			// ===================
			// SPACER
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(4.f)
			]
			// ===================
			// TITLE TEXT
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.MaxWidth(270)
				[
					SNew(SEditableTextBox)
					.Text(this, &SFlowGraphNode_YapSpeechBaseWidget::GetTitleText)
					.OnTextCommitted(this, &SFlowGraphNode_YapSpeechBaseWidget::HandleTitleTextCommitted)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.MinDesiredWidth(270)
					.HintText(INVTEXT("Enter optional title text"))
					//.ToolTip(INVTEXT("Title text is only used for selectable dialogue.\nWhen supplied, the game's UI will display selectable repsonses using their title text representation.\nWhen not supplied, the game's UI will display the full dialogue below for selectable responses."))
				]
			]
			// ===================
			// SPACER
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(4.f)
			]
			// ===================
			// AUDIO ASSET SELECTOR
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SObjectPropertyEntryBox)
				.DisplayBrowse(true)
				.DisplayThumbnail(true)
				.AllowedClass(UAkAudioEvent::StaticClass())
				.EnableContentPicker(true)
				.ObjectPath(this, &SFlowGraphNode_YapSpeechBaseWidget::GetSelectedDialogueAudioAssetPath)
				.OnObjectChanged(this, &SFlowGraphNode_YapSpeechBaseWidget::HandleDialogueAudioAssetChanged)
			]
			// ===================
			// SPACER
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(4.f)
			]
			// ===================
			// ADDITIONAL OPTIONS
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				GetAdditionalOptionsWidget()
			]
		];
}

TSharedRef<SBox> SFlowGraphNode_YapSpeechBaseWidget::GetPortraitWidget()
{
	TSharedPtr<SBox> PortraitPreview;
	
	SAssignNew(PortraitPreview, SBox)
	.WidthOverride(100)
	.HeightOverride(100)
	[
		SNew(SBorder)
		.ForegroundColor(FLinearColor::White)
		.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(2)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(this, &SFlowGraphNode_YapSpeechBaseWidget::GetPortraitBrush)
			]
			+ SOverlay::Slot()
			[
				SNew(STextBlock)
				.RenderTransformPivot(FVector2D(0.5, 0.5))
				.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-30.0f))))
				.Visibility(this, &SFlowGraphNode_YapSpeechBaseWidget::GetVisibilityForMissingPortraitText)
				.Text(INVTEXT("MISSING"))
			]
		]
	];

	return PortraitPreview.ToSharedRef();
}

TSharedRef<SBox> SFlowGraphNode_YapSpeechBaseWidget::GetPortraitKeySelectorWidget()
{
	TSharedPtr<SBox> Box;
	
	TSharedPtr<SSegmentedControl<FName>> PortraitSelector;

	SAssignNew(Box, SBox)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(PortraitSelector, SSegmentedControl<FName>)
			.SupportsMultiSelection(false)
			.Value(this, &SFlowGraphNode_YapSpeechBaseWidget::GetPortraitKey)
			.OnValueChanged(this, &SFlowGraphNode_YapSpeechBaseWidget::HandlePortraitKeyChanged)
			.UniformPadding(2.f)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer)
		]
	];
	
	FName SelectedPortraitKey = SFlowGraphNode_YapSpeechBaseWidget::GetPortraitKey();

	for (const FName& PortraitKey : GetDefault<UFlowYapProjectSettings>()->GetPortraitKeys())
	{
		if (PortraitKey == NAME_None)
		{
			UE_LOG(FlowYap, Warning, TEXT("Warning: Portrait keys contains a 'NONE' entry. Clean this up!"));
			continue;
		}
		
		bool bSelected = PortraitKey == SelectedPortraitKey;
		PortraitSelector->AddSlot(PortraitKey, true)
		.AttachWidget(CreatePortraitKeyButton(PortraitKey, bSelected));
	}

	return Box.ToSharedRef();
}

TSharedRef<SBox> SFlowGraphNode_YapSpeechBaseWidget::CreatePortraitKeyButton(FName InIconName, bool bSelected, const FText& InLabel, FName InTextStyle)
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();
		
	TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);

	TSharedPtr<SImage> PortraitIconImage;
		
	FString IconPath = ProjectSettings->GetPortraitIconPath(InIconName);

	// TODO this is going to become slow for large graphs, since it's loading the same thing every time. Need to load all portrait icons up front, build grayscale/colorized variants, store and use cached versions.
	UTexture2D* PortraitKeyIcon = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetPortraitKeyIcon(InIconName);
	
	FSlateBrush Brush;
	Brush.ImageSize = FVector2D(16, 16);
	Brush.SetResourceObject(PortraitKeyIcon);
		
	TSharedRef<FDeferredCleanupSlateBrush> PortraitKeyBrush = FDeferredCleanupSlateBrush::CreateBrush(Brush);
		
	if (!InIconName.IsNone())
	{
		HBox->AddSlot()
		.AutoWidth()
		.Padding(0.f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.Padding(1.0, 0.0)
			[
				SAssignNew(PortraitIconImage, SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(TAttribute<const FSlateBrush*>::Create(
						TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
			]
		];
	}
		
	if (!InLabel.IsEmpty())
	{
		HBox->AddSlot()	
		 .VAlign(VAlign_Center)
		.Padding(0.f, 0.5f, 0.f, 0.f)
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle( &FAppStyle::Get().GetWidgetStyle< FTextBlockStyle >( InTextStyle ))
			.Justification(ETextJustify::Center)
			.Text(InLabel)
		];
	}

	return SNew(SBox)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		HBox.ToSharedRef()
	];
}

FVector2D SFlowGraphNode_YapSpeechBaseWidget::CalculateOutputPinVerticalOffset() const
{
	int NumPins = FlowGraphNode_YapSpeechBase->OutputPins.Num();
	int FreePins = 3;

	NumPins = FMath::Max(NumPins - FreePins, 0);

	int BaseOffset = 43;
	int OffsetPerPin = 24;

	int Offset = FMath::Max(BaseOffset - NumPins * OffsetPerPin, 0);
	
	return FVector2D(0, Offset);
}

TSharedRef<SWidget> SFlowGraphNode_YapSpeechBaseWidget::CreateNodeContentArea()
{
	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("NoBorder"))
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SAssignNew(LeftNodeBox, SVerticalBox)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.FillWidth(1.0f)
				[
					CreateDialogueContentArea()
				]
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				.MaxDesiredHeight(100.f)
				.MaxDesiredWidth(421.f)
				.Padding(4.f)
				[
					SNew(SMultiLineEditableTextBox)
					.Text(this, &SFlowGraphNode_YapSpeechBaseWidget::GetDialogueText)
					.OnTextCommitted(this, &SFlowGraphNode_YapSpeechBaseWidget::HandleDialogueTextCommitted)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.HintText(INVTEXT("Enter full dialogue"))
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(this, &SFlowGraphNode_YapSpeechBaseWidget::CalculateOutputPinVerticalOffset)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(RightNodeBox, SVerticalBox)
			]
		]
	];
}

#pragma region Getters
FText SFlowGraphNode_YapSpeechBaseWidget::GetTitleText() const
{
	return FlowGraphNode_YapSpeechBase->GetTitleText();
}

FText SFlowGraphNode_YapSpeechBaseWidget::GetDialogueText() const
{
	return FlowGraphNode_YapSpeechBase->GetDialogueText();
}

FString SFlowGraphNode_YapSpeechBaseWidget::GetSelectedDialogueAudioAssetPath() const
{
	UAkAudioEvent* Asset = FlowNode_YapSpeechBase->GetDialogueAudio();

	if (!Asset)
	{
		return "";
	}

	FSoftObjectPath Path(Asset);

	return Path.GetAssetPathString();
}

FName SFlowGraphNode_YapSpeechBaseWidget::GetPortraitKey() const
{
	return FlowNode_YapSpeechBase->GetPortraitKey();
}

const FSlateBrush* SFlowGraphNode_YapSpeechBaseWidget::GetPortraitBrush() const
{
	FSlateBrush* Brush = FlowNode_YapSpeechBase->GetSpeakerPortraitBrush(GetPortraitKey());

	if (Brush && Brush->GetResourceObject())
	{
		return Brush;
	}

	return nullptr;
}

EVisibility SFlowGraphNode_YapSpeechBaseWidget::GetVisibilityForMissingPortraitText() const
{
	FSlateBrush* Brush = FlowNode_YapSpeechBase->GetSpeakerPortraitBrush(GetPortraitKey());

	if (Brush)
	{
		return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
	}

	return EVisibility::Visible;
}
#pragma endregion

#pragma region Input Handling
void SFlowGraphNode_YapSpeechBaseWidget::HandleTitleTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	// TODO: transaction undo handling - see what they did in wwise?
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		FlowNode_YapSpeechBase->SetTitleText(CommittedText);
	}
}

void SFlowGraphNode_YapSpeechBaseWidget::HandleDialogueTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		FlowNode_YapSpeechBase->SetDialogueText(CommittedText);
	}
}

void SFlowGraphNode_YapSpeechBaseWidget::HandleDialogueAudioAssetChanged(const FAssetData& InAssetData)
{
	FlowNode_YapSpeechBase->SetDialogueAudioAsset(InAssetData);
}

void SFlowGraphNode_YapSpeechBaseWidget::HandlePortraitKeyChanged(FName NewValue)
{
	FlowNode_YapSpeechBase->SetPortraitKey(NewValue);
	Invalidate(EInvalidateWidgetReason::Layout);
}

void SFlowGraphNode_YapSpeechBaseWidget::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = (PinObj != nullptr) && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility( TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced) );
	}

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		LeftNodeBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(10, 0, 2, 0))
		[
			PinToAdd
		];
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		RightNodeBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2, 0, 10, 0))
		[
			PinToAdd
		];
		OutputPins.Add(PinToAdd);
	}
}

void SFlowGraphNode_YapSpeechBaseWidget::CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox)
{
	if (FlowGraphNode->CanUserAddOutput())
	{
		TSharedPtr<SWidget> AddPinWidget;
		SAssignNew(AddPinWidget, SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(7,0,0,0)
		[
			SNew(SImage)
			.Image(FAppStyle::GetBrush(TEXT("Icons.PlusCircle")))
		];

		AddPinButton(OutputBox, AddPinWidget.ToSharedRef(), EGPD_Output);
	}
}

void SFlowGraphNode_YapSpeechBaseWidget::AddPinButton(TSharedPtr<SVerticalBox> OutputBox, const TSharedRef<SWidget> ButtonContent, const EEdGraphPinDirection Direction, const FString DocumentationExcerpt, const TSharedPtr<SToolTip> CustomTooltip)
{
	const FText PinTooltipText = (Direction == EEdGraphPinDirection::EGPD_Input) ? LOCTEXT("FlowNodeAddPinButton_InputTooltip", "Adds an input pin") : LOCTEXT("FlowNodeAddPinButton_OutputTooltip", "Adds an output pin");
	TSharedPtr<SToolTip> Tooltip;

	if (CustomTooltip.IsValid())
	{
		Tooltip = CustomTooltip;
	}
	else if (!DocumentationExcerpt.IsEmpty())
	{
		Tooltip = IDocumentation::Get()->CreateToolTip(PinTooltipText, nullptr, GraphNode->GetDocumentationLink(), DocumentationExcerpt);
	}

	const TSharedRef<SButton> AddPinButton = SNew(SButton)
	.ContentPadding(0.0f)
	.ButtonStyle(FAppStyle::Get(), "NoBorder")
	.OnClicked(this, &SFlowGraphNode_YapSpeechBaseWidget::OnAddFlowPin, Direction)
	.IsEnabled(this, &SFlowGraphNode_YapSpeechBaseWidget::IsNodeEditable)
	.ToolTipText(PinTooltipText)
	.ToolTip(Tooltip)
	.Visibility(this, &SFlowGraphNode_YapSpeechBaseWidget::IsAddPinButtonVisible)
	[
		ButtonContent
	];

	AddPinButton->SetCursor(EMouseCursor::Hand);

	FMargin AddPinPadding = (Direction == EEdGraphPinDirection::EGPD_Input) ? Settings->GetInputPinPadding() : Settings->GetOutputPinPadding();
	AddPinPadding.Top += 0.0f;

	OutputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(AddPinPadding)
		[
			AddPinButton
		];
}
#pragma endregion

void SFlowGraphNode_YapSpeechBaseWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	// TODO: I might actually want to remove these checks and instead handle null pointers as valid logic paths so that I can clean up dead graph objects?
	GraphNode = InNode;
	FlowGraphNode = InNode;
	FlowGraphNode_YapSpeechBase = Cast<UFlowGraphNode_YapSpeechBase>(InNode);
	check(GraphNode);
	check(FlowGraphNode);
	check(FlowGraphNode_YapSpeechBase);

	FlowNode_YapSpeechBase = Cast<UFlowNode_YapSpeechBase>(FlowGraphNode->GetFlowNode());
	check(FlowNode_YapSpeechBase);

	//SetCursor(EMouseCursor::CardinalCross);
}

#undef LOCTEXT_NAMESPACE
