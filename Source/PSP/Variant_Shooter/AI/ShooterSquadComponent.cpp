#include "ShooterSquadComponent.h"
#include "ShooterSquadSubsystem.h"
#include "Engine/World.h"

UShooterSquadComponent::UShooterSquadComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UShooterSquadComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UShooterSquadSubsystem* Subsystem = GetSquadSubsystem())
	{
		Subsystem->RegisterMember(SquadId, this);
	}
}

void UShooterSquadComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UShooterSquadSubsystem* Subsystem = GetSquadSubsystem())
	{
		Subsystem->UnregisterMember(SquadId, this);
	}

	Super::EndPlay(EndPlayReason);
}

void UShooterSquadComponent::BroadcastTarget(AActor* NewTarget)
{
	if (UShooterSquadSubsystem* Subsystem = GetSquadSubsystem())
	{
		Subsystem->SetSquadTarget(SquadId, NewTarget);
	}
}

FShooterSquadOrder UShooterSquadComponent::GetCurrentOrder() const
{
	if (const UShooterSquadSubsystem* Subsystem = GetSquadSubsystem())
	{
		return Subsystem->BuildOrder(SquadId, this);
	}

	return FShooterSquadOrder();
}

UShooterSquadSubsystem* UShooterSquadComponent::GetSquadSubsystem() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetSubsystem<UShooterSquadSubsystem>();
	}

	return nullptr;
}
