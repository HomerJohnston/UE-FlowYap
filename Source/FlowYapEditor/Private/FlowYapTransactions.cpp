#include "FlowYapTransactions.h"

#include "Editor/TransBuffer.h"

void FFlowYapTransactions::BeginModify(FText TransactionText, UObject* Object)
{
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

void FFlowYapTransactions::EndModify()
{
	if (GEditor && GEditor->Trans)
	{
		UTransBuffer* TransBuffer = CastChecked<UTransBuffer>(GEditor->Trans);
		if (TransBuffer != nullptr)
			TransBuffer->End();
	}
}
