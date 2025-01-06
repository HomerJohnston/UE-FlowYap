// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#define LOCTEXT_NAMESPACE "Yap"

#include "Yap/YapTransactions.h"

#include "Editor/TransBuffer.h"

void FYapTransactions::BeginModify(FText TransactionText, UObject* Object)
{
	// TODO change all this old shit to GEditor->BeginTransaction, EndTransaction
	if (GEditor && GEditor->Trans)
	{
		UTransBuffer* TransBuffer = CastChecked<UTransBuffer>(GEditor->Trans);
		if (TransBuffer != nullptr)
			TransBuffer->Begin(*FString("Yap"), TransactionText);
	}
	
	if (IsValid(Object))
	{
		Object->Modify();
	}
}

void FYapTransactions::EndModify()
{
	if (GEditor && GEditor->Trans)
	{
		UTransBuffer* TransBuffer = CastChecked<UTransBuffer>(GEditor->Trans);
		if (TransBuffer != nullptr)
			TransBuffer->End();
	}
}

#undef LOCTEXT_NAMESPACE