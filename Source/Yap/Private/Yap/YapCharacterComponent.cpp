// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "Yap/YapCharacterComponent.h"

#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "Yap"

void UYapCharacterComponent::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorld()->GetSubsystem<UYapSubsystem>()->RegisterCharacterComponent(this);
}

void UYapCharacterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetSubsystem<UYapSubsystem>()->UnregisterCharacterComponent(this);
	
	Super::EndPlay(EndPlayReason);
}

#undef LOCTEXT_NAMESPACE