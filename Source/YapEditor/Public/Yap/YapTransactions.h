#pragma once

class FYapTransactions
{
public:
	static void BeginModify(FText TransactionText, UObject* Object);

	static void EndModify();
};