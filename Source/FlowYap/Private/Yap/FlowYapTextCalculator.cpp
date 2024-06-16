

#include "Yap/FlowYapTextCalculator.h"

#include "Yap/FlowYapProjectSettings.h"

int16 UFlowYapTextCalculator::CalculateWordCount(const FText& Text)
{
	FString TextAsString = Text.ToString();

	bool bInSpace = false;
	bool bInNewParagraph = false;

	// Since the system only clicks a word after a space or return line, "Ok." won't be counted. Simple workaround.
	int32 WordCount = 1;

	for (int i = 0; i < TextAsString.Len(); ++i)
	{
		TCHAR* Char = &TextAsString.GetCharArray().GetData()[i];

		switch (*Char)
		{
		case TCHAR('\n'):
		case TCHAR('\r'):
			{
				if (!bInNewParagraph)
				{
					WordCount += 2;
					bInNewParagraph = true;
				}
				break;
			}
		case TCHAR(' '):
			{
				if (!bInSpace)
				{
					++WordCount;
					bInSpace = true;
				}
				break;
			}
		default:
			{
				bInSpace = false;
				bInNewParagraph = false;
				break;
			}
		}
	}

	return WordCount;
}

double UFlowYapTextCalculator::CalculateTextTime(int16 WordCount)
{
	int32 TWPM = UFlowYapProjectSettings::Get()->GetTextWordsPerMinute();
	double Min = UFlowYapProjectSettings::Get()->GetMinimumAutoTextTimeLength();
	double SecondsPerWord = 60.0 / (double)TWPM;

	return FMath::Max(WordCount * SecondsPerWord, Min);
}
