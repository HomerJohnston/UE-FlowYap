#pragma once

class FFlowYapTransactions
{
public:
	static void BeginModify(FText TransactionText, UObject* Object);

	static void EndModify();
};