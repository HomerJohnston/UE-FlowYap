#pragma once
#include "Engine/StreamableManager.h"

class FYapStreamableManager
{
public:
	static FStreamableManager& Get()
	{
		static FStreamableManager Instance;
		return Instance;
	}
};
