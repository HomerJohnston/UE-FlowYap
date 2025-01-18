// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapTextCalculator.h"

#include "Internationalization/BreakIterator.h"
#include "Yap/YapProjectSettings.h"

#define LOCTEXT_NAMESPACE "Yap"

int32 UYapTextCalculator::CalculateWordCount(const FText& Text)
{
	// Utility to count the number of words within a string (we use a line-break iterator to avoid counting the whitespace between the words)
	TSharedRef<IBreakIterator> LineBreakIterator = FBreakIterator::CreateLineBreakIterator();
	auto CountWords = [&LineBreakIterator](const FString& InTextToCount) -> int32
	{
		int32 NumWords = 0;
		LineBreakIterator->SetString(InTextToCount);

		int32 PreviousBreak = 0;
		int32 CurrentBreak;

		while ((CurrentBreak = LineBreakIterator->MoveToNext()) != INDEX_NONE)
		{
			if (CurrentBreak > PreviousBreak)
			{
				++NumWords;
			}
			PreviousBreak = CurrentBreak;
		}

		LineBreakIterator->ClearString();
		return NumWords;
	};

	return CountWords(Text.ToString());
}

double UYapTextCalculator::CalculateTextTime(int16 WordCount)
{
	int32 TWPM = UYapProjectSettings::GetTextWordsPerMinute();
	double Min = UYapProjectSettings::GetMinimumAutoTextTimeLength();
	double SecondsPerWord = 60.0 / (double)TWPM;

	return FMath::Max(WordCount * SecondsPerWord, Min);
}

#undef LOCTEXT_NAMESPACE