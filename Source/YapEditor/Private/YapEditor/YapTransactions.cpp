// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapTransactions.h"

#include "Editor/TransBuffer.h"
#include "Nodes/FlowNodeBase.h"
#include "YapEditor/GraphNodes/FlowGraphNode_YapBase.h"
#include "YapEditor/YapLogEditor.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FYapTransactions::BeginModify(const FText& TransactionText, UObject* Object)
{
	// TODO change all this old shit to GEditor->BeginTransaction, EndTransaction
	if (GEditor && GEditor->Trans)
	{
		UTransBuffer* TransBuffer = CastChecked<UTransBuffer>(GEditor->Trans);
		if (TransBuffer != nullptr)
			TransBuffer->Begin(TEXT("Yap"), TransactionText);
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

// ================================================================================================

FYapScopedTransaction::FYapScopedTransaction(FName InEvent, const FText& TransactionText, UObject* Object)
{
	GEngine->BeginTransaction(TEXT("Yap"), TransactionText, Object);
}

FYapScopedTransaction::~FYapScopedTransaction()
{
	GEngine->EndTransaction();
}

#undef LOCTEXT_NAMESPACE
