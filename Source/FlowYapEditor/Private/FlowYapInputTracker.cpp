#include "FlowYapInputTracker.h"

FFlowYapInputTracker::FFlowYapInputTracker(UFlowYapEditorSubsystem* InOwner)
{
	Owner = InOwner;
}

void FFlowYapInputTracker::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
}

bool FFlowYapInputTracker::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::LeftShift)
	{
		bLeftShift = true;
	}
	else if (InKeyEvent.GetKey() == EKeys::RightShift)
	{
		bRightShift = true;
	}
	else if (InKeyEvent.GetKey() == EKeys::LeftControl)
	{
		bLeftControl = true;
	}
	else if (InKeyEvent.GetKey() == EKeys::RightControl)
	{
		bRightControl = true;
	}
	
	return false;
}

bool FFlowYapInputTracker::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::LeftShift)
	{
		bLeftShift = false;
	}
	else if (InKeyEvent.GetKey() == EKeys::RightShift)
	{
		bRightShift = false;
	}
	else if (InKeyEvent.GetKey() == EKeys::LeftControl)
	{
		bLeftControl = false;
	}
	else if (InKeyEvent.GetKey() == EKeys::RightControl)
	{
		bRightControl = false;
	}
	
	return false;
}

bool FFlowYapInputTracker::GetShiftPressed() const
{
	return bLeftShift || bRightShift;
}

bool FFlowYapInputTracker::GetControlPressed() const
{
	return bLeftControl || bRightControl;
}
