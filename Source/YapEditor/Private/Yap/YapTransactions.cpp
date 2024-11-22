// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#define LOCTEXT_NAMESPACE "FlowYap"

#include "Yap/YapTransactions.h"

#include "Editor/TransBuffer.h"

void FYapTransactions::BeginModify(FText TransactionText, UObject* Object)
{
	// TODO change all this old shit to GEditor->BeginTransaction, EndTransaction
	if (GEditor && GEditor->Trans)
	{
		UTransBuffer* TransBuffer = CastChecked<UTransBuffer>(GEditor->Trans);
		if (TransBuffer != nullptr)
			TransBuffer->Begin(*FString("FlowYap"), TransactionText);
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