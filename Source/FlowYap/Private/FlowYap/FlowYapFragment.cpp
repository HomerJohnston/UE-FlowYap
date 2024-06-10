// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYap/FlowYapFragment.h"

#define LOCTEXT_NAMESPACE "FlowYap"

#undef LOCTEXT_NAMESPACE
bool FFlowYapFragment::TryActivate()
{
	bool bResult = true;
	
	if (ActivationLimit > 0)
	{
		bResult = ActivationCount < ActivationLimit;
		ActivationCount++;
	}

	return bResult;
}
