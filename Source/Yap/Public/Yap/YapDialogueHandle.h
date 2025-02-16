// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "CoreMinimal.h"

#include "YapDialogueHandle.generated.h"

struct FInstancedStruct;
class UObject;
class UFlowNode_YapDialogue;

// ================================================================================================

/** Since I can't store handles in structs by ref, I pass around a simpler version of it containing only the GUID and look up the actual handle (which could become "large" with more data) from the subsystem as needed. */
USTRUCT(BlueprintType)
struct YAP_API FYapDialogueHandleRef
{
	GENERATED_BODY()

	// ==========================================
	// CONSTRUCTION
	// ==========================================
public:
	
	FYapDialogueHandleRef()
	{
		Guid.Invalidate();
	}

	FYapDialogueHandleRef(FGuid InGuid) : Guid(InGuid)
	{
	}

	// ==========================================
	// STATE
	// ==========================================
private:
	
	UPROPERTY(Transient, meta = (IgnoreForMemberInitializationTest))
	FGuid Guid;

	// ==========================================
	// API
	// ==========================================
public:

	bool IsValid() const { return Guid.IsValid(); }

	const FGuid& GetGuid() const { return Guid; }

	bool SkipDialogue();

	void AddReactor(UObject* Reactor);
	
	const TArray<FInstancedStruct>& GetFragmentData();
	
	bool operator== (const FYapDialogueHandleRef& Other) const;
};

FORCEINLINE uint32 GetTypeHash(const FYapDialogueHandleRef& Struct)
{
	return GetTypeHash(Struct.GetGuid());
}

// ================================================================================================

/** Handle for a piece of running dialogue. Used to track individual running dialogues. Can be used to instruct the Yap Subsystem to interrupt a piece of dialogue, unless it is marked not skippable. */
USTRUCT(BlueprintType)
struct YAP_API FYapDialogueHandle
{
	GENERATED_BODY()

	// ==========================================
	// CONSTRUCTION
	// ==========================================
public:
	
	FYapDialogueHandle() { };

	FYapDialogueHandle(UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex);

	static FYapDialogueHandle& InvalidHandle() { return _InvalidHandle; }

	// ==========================================
	// STATE
	// ==========================================
private:
	
	static FYapDialogueHandle _InvalidHandle;
	
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<UFlowNode_YapDialogue> DialogueNode = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess))
	uint8 FragmentIndex = 0;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess, IgnoreForMemberInitializationTest))
	FGuid Guid;
	
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UObject>> Reactors; 
	
	// ==========================================
	// API
	// ==========================================
public:

	UFlowNode_YapDialogue* GetDialogueNode() const { return DialogueNode; }

	uint8 GetFragmentIndex() const { return FragmentIndex; }

	const FGuid& GetGuid() const { return Guid; }
	
	void OnSpeakingEnds() const;
	
	void Invalidate();
	
	bool IsValid() const { return Guid.IsValid(); }

	void AddReactor(UObject* Reactor);
	
	bool operator== (const FYapDialogueHandle& Other) const;
};

FORCEINLINE uint32 GetTypeHash(const FYapDialogueHandle& Struct)
{
	return GetTypeHash(Struct.GetGuid());
}