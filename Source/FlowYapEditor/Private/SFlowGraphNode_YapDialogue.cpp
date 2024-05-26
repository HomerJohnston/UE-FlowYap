// 2022 The Enby Witch

#include "SFlowGraphNode_YapDialogue.h"

#include "AkAudioEvent.h"
#include "EditorStyleSet.h"
#include "FlowEditorStyle.h"
#include "FlowGraphNode_YapDialogue.h"
#include "GraphEditorSettings.h"
#include "ImageUtils.h"
#include "PropertyCustomizationHelpers.h"
#include "SCommentBubble.h"
#include "SLevelOfDetailBranchNode.h"
#include "SlateOptMacros.h"
#include "TutorialMetaData.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "FlowYap/FlowYapEngineUtils.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "FlowYapEditorSubsystem.h"
#include "FlowYap/FlowYapLog.h"
#include "Widgets/SCanvas.h"

#define LOCTEXT_NAMESPACE "PGFlowEditor"

void SFlowGraphNode_YapDialogue::UpdateGraphNode()
{
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
	
	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("FlowGraphNode"));
	PopulateMetaTag(&TagMeta);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);


	const TSharedRef<SWidget> TitleAreaWidget = 
		SNew(SLevelOfDetailBranchNode)
			//.UseLowDetailSlot(this, &SFlowGraphNode::UseLowDetailNodeTitles)
			.LowDetail()
			[
				DefaultTitleAreaWidget
			]
			.HighDetail()
			[
				DefaultTitleAreaWidget
			];
	
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
}

bool SFlowGraphNode_YapDialogue::HasSingleOutput() const
{
	return OutputPins.Num() == 1;
}

bool SFlowGraphNode_YapDialogue::HasMultipleOutput() const
{
	return !HasSingleOutput();
}

int SFlowGraphNode_YapDialogue::GetSingleFlowMode() const
{
}

void SFlowGraphNode_YapDialogue::HandleSingleFlowModeChanged(int I)
{
}

int SFlowGraphNode_YapDialogue::GetMultiFlowMode() const
{
}

void SFlowGraphNode_YapDialogue::HandleMultiFlowModeChanged(int I)
{
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogue::CreateDialogueContentArea()
{
	const FFlowYapFragment SpeechElement = FlowNode_YapDialogue->GetFragment();

	FText TitleText = SpeechElement.TitleText;

	TSharedPtr<SSegmentedControl<FName>> PortraitSelector;

	SAssignNew(PortraitSelector, SSegmentedControl<FName>)
	.SupportsMultiSelection(false)
	.Value(this, &SFlowGraphNode_YapDialogue::GetPortraitKey)
	.OnValueChanged(this, &SFlowGraphNode_YapDialogue::HandlePortraitKeyChanged)
	.UniformPadding(0.f);

	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();

	FName SelectedPortraitKey = SFlowGraphNode_YapDialogue::GetPortraitKey();

	for (const FName& PortraitKey : ProjectSettings->GetPortraitKeys())
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
	
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(0.f, 5.f, 5.f, 0.f)
			// ===================
			// PORTRAIT IMAGE
			// ===================
			[
				SNew(SBox)
				.WidthOverride(78)
				.HeightOverride(102)
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
							.Image(this, &SFlowGraphNode_YapDialogue::GetPortraitBrush)
						]
						+ SOverlay::Slot()
						[
							SNew(STextBlock)
							.RenderTransformPivot(FVector2D(0.5, 0.5))
							.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-30.0f))))
							.Visibility(this, &SFlowGraphNode_YapDialogue::GetVisibilityForMissingPortraitText)
							.Text(INVTEXT("MISSING"))
						]
					]
				]
			]
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
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					[
						PortraitSelector.ToSharedRef()
					]
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
			// TITLE TEXT
			// ===================
			+ SVerticalBox::Slot()
			[
				SNew(SEditableTextBox)
				.Text(this, &SFlowGraphNode_YapDialogue::GetTitleText)
				.OnTextCommitted(this, &SFlowGraphNode_YapDialogue::HandleTitleTextCommitted)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.HintText(INVTEXT("Enter optional title text"))
				//.ToolTip(INVTEXT("Title text is only used for selectable dialogue.\nWhen supplied, the game's UI will display selectable repsonses using their title text representation.\nWhen not supplied, the game's UI will display the full dialogue below for selectable responses."))
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
			[
				SNew(SObjectPropertyEntryBox)
				.DisplayBrowse(true)
				.DisplayThumbnail(true)
				.AllowedClass(UAkAudioEvent::StaticClass())
				.EnableContentPicker(true)
				.ObjectPath(this, &SFlowGraphNode_YapDialogue::GetSelectedDialogueAudioAssetPath)
				.OnObjectChanged(this, &SFlowGraphNode_YapDialogue::HandleDialogueAudioAssetChanged)
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
			// FLOW MODE SELECTORS
			// ===================
			+ SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				
				+ SHorizontalBox::Slot()
				[
					SNew(SSegmentedControl<int>)
					.IsEnabled(this, &SFlowGraphNode_YapDialogue::HasSingleOutput)
					.Value(this, &SFlowGraphNode_YapDialogue::GetSingleFlowMode)
					.OnValueChanged(this, &SFlowGraphNode_YapDialogue::HandleSingleFlowModeChanged)
					+ SSegmentedControl<int32>::Slot(0)
					.Icon(FAppStyle::Get().GetBrush("Icons.box-perspective"))
					+SSegmentedControl<int32>::Slot(1)
					.Icon(FAppStyle::Get().GetBrush("Icons.cylinder"))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SSegmentedControl<int>)
					.IsEnabled(this, &SFlowGraphNode_YapDialogue::HasMultipleOutput)
					.Value(this, &SFlowGraphNode_YapDialogue::GetMultiFlowMode)
					.OnValueChanged(this, &SFlowGraphNode_YapDialogue::HandleMultiFlowModeChanged)
					+ SSegmentedControl<int32>::Slot(0)
					.Icon(FAppStyle::Get().GetBrush("Icons.box-perspective"))
					+SSegmentedControl<int32>::Slot(1)
					.Icon(FAppStyle::Get().GetBrush("Icons.cylinder"))
				]
			]
		];
}

TSharedRef<SBox> SFlowGraphNode_YapDialogue::CreatePortraitKeyButton(FName InIconName, bool bSelected, const FText& InLabel, FName InTextStyle)
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();
		
	TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);
	float Space = InIconName.IsNone() ? 0.0f : 2.0f;

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
			SAssignNew(PortraitIconImage, SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(TAttribute<const FSlateBrush*>::Create(
					TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
		];
	}
		
	if (!InLabel.IsEmpty())
	{
		HBox->AddSlot()	
		 .VAlign(VAlign_Center)
		.Padding(0.f, 0.5f, 0.f, 0.f)  // Compensate down for the baseline since we're using all caps
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle( &FAppStyle::Get().GetWidgetStyle< FTextBlockStyle >( InTextStyle ))
			.Justification(ETextJustify::Center)
			.Text(InLabel)
		];
	}

	return SNew(SBox)
	.MinDesiredHeight(24.f)
	.MaxDesiredHeight(24.f)
	.MinDesiredWidth(24.f)
	.MaxDesiredWidth(24.f)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		//HBox.ToSharedRef()
		
		SAssignNew(PortraitIconImage, SImage)
		.ColorAndOpacity(FSlateColor::UseForeground())

		.Image(TAttribute<const FSlateBrush*>::Create(
				TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
	];
}

/*
TSharedRef<SBox> SFlowGraphNode_YapDialogue::CreateOutputFlowSelector(FName InIconName)
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();
		
	TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);
	float Space = InIconName.IsNone() ? 0.0f : 2.0f;

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
			SAssignNew(PortraitIconImage, SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(TAttribute<const FSlateBrush*>::Create(
					TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
		];
	}
		
	if (!InLabel.IsEmpty())
	{
		HBox->AddSlot()	
		 .VAlign(VAlign_Center)
		.Padding(0.f, 0.5f, 0.f, 0.f)  // Compensate down for the baseline since we're using all caps
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle( &FAppStyle::Get().GetWidgetStyle< FTextBlockStyle >( InTextStyle ))
			.Justification(ETextJustify::Center)
			.Text(InLabel)
		];
	}

	return SNew(SBox)
	.MinDesiredHeight(24.f)
	.MaxDesiredHeight(24.f)
	.MinDesiredWidth(24.f)
	.MaxDesiredWidth(24.f)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		//HBox.ToSharedRef()
		
		SAssignNew(PortraitIconImage, SImage)
		.ColorAndOpacity(FSlateColor::UseForeground())

		.Image(TAttribute<const FSlateBrush*>::Create(
				TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
	];
}
*/


TSharedRef<SWidget> SFlowGraphNode_YapDialogue::CreateNodeContentArea()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SAssignNew(LeftNodeBox, SVerticalBox)
			]
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.FillWidth(1.0f)
			[
				CreateDialogueContentArea()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				SAssignNew(RightNodeBox, SVerticalBox)
			]
		];
}

void SFlowGraphNode_YapDialogue::CreateBelowPinControls(TSharedPtr<SVerticalBox> MainBox)
{
	MainBox->AddSlot()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.FillWidth(1.0f)
		.Padding(10.f)
		.MaxWidth(600.f)
			[
				SNew(SEditableTextBox)
				.Text(this, &SFlowGraphNode_YapDialogue::GetDialogueText)
				.OnTextCommitted(this, &SFlowGraphNode_YapDialogue::HandleDialogueTextCommitted)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.HintText(INVTEXT("Enter full dialogue"))
			]
	];
}

void SFlowGraphNode_YapDialogue::HandleTitleTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		FlowNode_YapDialogue->SetTitleText(CommittedText);
	}
}

void SFlowGraphNode_YapDialogue::HandleDialogueTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		FlowNode_YapDialogue->SetDialogueText(CommittedText);
	}
}

void SFlowGraphNode_YapDialogue::HandleDialogueAudioAssetChanged(const FAssetData& InAssetData)
{
	FlowNode_YapDialogue->SetDialogueAudioAsset(InAssetData);
}

void SFlowGraphNode_YapDialogue::HandlePortraitKeyChanged(FName NewValue)
{
	FlowNode_YapDialogue->SetPortraitKey(NewValue);
	Invalidate(EInvalidateWidgetReason::Layout);
}

FText SFlowGraphNode_YapDialogue::GetHeadTitle() const
{
	return INVTEXT("TESTTESTTEST");
}

FText SFlowGraphNode_YapDialogue::GetTitleText() const
{
	return FlowGraphNode_YapDialogue->GetTitleText();
}

FText SFlowGraphNode_YapDialogue::GetDialogueText() const
{
	return FlowGraphNode_YapDialogue->GetDialogueText();
}

FString SFlowGraphNode_YapDialogue::GetSelectedDialogueAudioAssetPath() const
{
	UAkAudioEvent* Asset = FlowNode_YapDialogue->GetDialogueAudio();

	if (!Asset)
	{
		return "";
	}

	FSoftObjectPath Path(Asset);

	return Path.GetAssetPathString();
}

FName SFlowGraphNode_YapDialogue::GetPortraitKey() const
{
	return FlowNode_YapDialogue->GetPortraitKey();
}

const FSlateBrush* SFlowGraphNode_YapDialogue::GetPortraitBrush() const
{
	FSlateBrush* Brush = FlowNode_YapDialogue->GetSpeakerPortraitBrush(GetPortraitKey());

	if (Brush->GetResourceObject())
	{
		//bHasValidPortraitBrush = true;
		return Brush;
	}

	//bHasValidPortraitBrush = false;
	return nullptr;
}

EVisibility SFlowGraphNode_YapDialogue::GetVisibilityForMissingPortraitText() const
{
	FSlateBrush* Brush = FlowNode_YapDialogue->GetSpeakerPortraitBrush(GetPortraitKey());

	return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
}

void SFlowGraphNode_YapDialogue::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	SpeakerTextColor = FLinearColor(0.f, 0.f, 0.f, 1.f);
	PronounTextColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	// TODO: I might actually want to remove these checks and instead handle null pointers as valid logic paths so that I can clean up dead graph objects?
	GraphNode = InNode;
	FlowGraphNode = InNode;
	FlowGraphNode_YapDialogue = Cast<UFlowGraphNode_YapDialogue>(InNode);
	check(GraphNode);
	check(FlowGraphNode);
	check(FlowGraphNode_YapDialogue);

	FlowNode_YapDialogue = Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNode());
	check(FlowNode_YapDialogue);

	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

#undef LOCTEXT_NAMESPACE
