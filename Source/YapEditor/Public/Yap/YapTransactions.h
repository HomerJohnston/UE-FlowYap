// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

/** Helper class. I ran into a few odd issues using FScopedTransaction so I made this, seems to work more reliably. */
class FYapTransactions
{
public:
	static void BeginModify(FText TransactionText, UObject* Object);

	static void EndModify();
};