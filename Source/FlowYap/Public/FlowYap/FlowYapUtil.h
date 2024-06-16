#pragma once

struct FlowYapUtil
{
	static inline FString NodeCategory = "Yap";

public:
	static FString GetFilteredSubTag(const FGameplayTag& ParentContainer, const FGameplayTag& PropertyTag)
	{
		if (ParentContainer.IsValid() && ParentContainer != FGameplayTag::EmptyTag && PropertyTag.MatchesTag(ParentContainer))
		{
			return GetFilteredSubTag(ParentContainer.ToString(), PropertyTag);
		}

		return PropertyTag.ToString();
	}
	
	static FString GetFilteredSubTag(const FString& Filter, const FGameplayTag& PropertyTag)
	{
		return PropertyTag.ToString().RightChop(Filter.Len() + 1); // +1 for the '.' TODO check if last char is actually a . or not
	}
};