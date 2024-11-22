#pragma once
#include "Widgets/SUserWidget.h"
#include "Yap/YapColors.h"
#include "Yap/YapEditorStyle.h"

class UFlowNode_YapDialogue;
struct FGameplayTag;
class UYapCondition;

class FYapWidgetHelper
{
public:
	
	static TSharedRef<SWidget> CreateConditionWidget(UFlowNode_YapDialogue* Dialogue, UYapCondition* Condition);

	static TSharedRef<SWidget> CreateTagPreviewWidget(TAttribute<FText> Text, TAttribute<EVisibility> Visibility);

	static TSharedRef<SWidget> CreateFilteredTagWidget(TAttribute<FGameplayTag> Tag, FString FilterString, TDelegate<void(const FGameplayTag)> OnTagChanged, FText ToolTipText);
};

class SActivationCounterWidget : public SCompoundWidget
{
	public:
	SLATE_USER_ARGS( SActivationCounterWidget )
		: _FontHeight(8) {}

	SLATE_ATTRIBUTE(int32, ActivationCount)
	SLATE_ATTRIBUTE(int32, ActivationLimit)
	SLATE_ARGUMENT(int32, FontHeight)
	
	SLATE_END_ARGS()

	TAttribute<int32> ActivationCount;
	TAttribute<int32> ActivationLimit;
	int32 FontHeight;
	
	FText NumeratorText() const;
	FText DenominatorText() const;

	FSlateColor NumeratorColor() const;
	FSlateColor DenominatorColor() const;

	TSharedPtr<SEditableText> Denominator;
	
	virtual void Construct(const FArguments& InArgs, FOnTextCommitted OnTextCommitted);
};
