// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapDialogueText.h"

#include "Yap/YapProjectSettings.h"

#if WITH_EDITOR
void FYapDialogueText::Set(const FText& InText)
{
	Text = InText;

	// TODO I have this setting... but if it's turned off, there's no way to set the word count. Add a way to configure word count.
	if (UYapProjectSettings::CacheFragmentWordCountAutomatically())
	{
		UpdateInternalWordCount();
	}
}
#endif

#if WITH_EDITOR
void FYapDialogueText::UpdateInternalWordCount()
{
	int32 NewWordCount = -1;

	if (Text.IsEmptyOrWhitespace())
	{
		WordCount = 0;
		return;
	}
	
	const UYapBroker* Broker = UYapProjectSettings::GetEditorBrokerDefault();

	if (IsValid(Broker))
	{
		NewWordCount = Broker->CalculateWordCount(Text);
	}

	if (NewWordCount < 0)
	{
		UE_LOG(LogYap, Error, TEXT("Could not calculate word count!"));
	}

	WordCount = NewWordCount;
}

void FYapDialogueText::Clear()
{
	Text = FText::GetEmpty();
	WordCount = 0;
}
#endif
