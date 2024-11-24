#include "Yap/YapCharacterComponent.h"

#include "Yap/YapSubsystem.h"

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
