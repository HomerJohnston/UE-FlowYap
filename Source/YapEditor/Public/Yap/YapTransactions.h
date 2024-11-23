#pragma once

/** Helper class. I ran into a few odd issues using FScopedTransaction so I made this, seems to work more reliably. */
class FYapTransactions
{
public:
	static void BeginModify(FText TransactionText, UObject* Object);

	static void EndModify();
};