// Fill out your copyright notice in the Description page of Project Settings.


#include "YapBlueprintFunctionLibrary.h"

#include "Yap/YapPromptHandle.h"

void UYapBlueprintFunctionLibrary::SelectPrompt(FYapPromptHandle Handle)
{
	Handle.RunPrompt(GEditor->PlayWorld);
}
