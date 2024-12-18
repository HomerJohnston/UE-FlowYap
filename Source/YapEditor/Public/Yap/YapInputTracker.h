#pragma once

#include "Framework/Application/IInputProcessor.h"

class UYapEditorSubsystem;

class FYapInputTracker : public IInputProcessor
{
protected:
	UYapEditorSubsystem* Owner;

	// STATE
protected:
	bool bLeftShift = false;
	bool bRightShift = false;
	
	bool bLeftControl = false;
	bool bRightControl = false;
	
public:
	FYapInputTracker(UYapEditorSubsystem* InOwner);

	void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
	
	bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

public:
	bool GetShiftPressed() const; 

	bool GetControlPressed() const; 
};

