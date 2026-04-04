#include "ShooterSquadSubsystem.h"
#include "ShooterSquadComponent.h"
#include "GameFramework/Actor.h"

void UShooterSquadSubsystem::RegisterMember(FName SquadId, UShooterSquadComponent* Member)
{
	if (!Member || SquadId.IsNone())
	{
		return;
	}

	FShooterSquadRuntime& Squad = Squads.FindOrAdd(SquadId);
	CleanupNullMembers(Squad);

	if (!Squad.Members.Contains(Member))
	{
		Squad.Members.Add(Member);
	}
}

void UShooterSquadSubsystem::UnregisterMember(FName SquadId, UShooterSquadComponent* Member)
{
	if (!Member || SquadId.IsNone())
	{
		return;
	}

	if (FShooterSquadRuntime* Squad = Squads.Find(SquadId))
	{
		Squad->Members.Remove(Member);
		CleanupNullMembers(*Squad);

		if (Squad->Members.IsEmpty())
		{
			Squads.Remove(SquadId);
		}
	}
}

void UShooterSquadSubsystem::SetSquadTarget(FName SquadId, AActor* NewTarget)
{
	if (SquadId.IsNone())
	{
		return;
	}

	FShooterSquadRuntime& Squad = Squads.FindOrAdd(SquadId);
	Squad.SharedTarget = NewTarget;
}

FShooterSquadOrder UShooterSquadSubsystem::BuildOrder(FName SquadId, const UShooterSquadComponent* Requester) const
{
	FShooterSquadOrder Order;

	if (!Requester || SquadId.IsNone())
	{
		return Order;
	}

	const FShooterSquadRuntime* Squad = Squads.Find(SquadId);
	if (!Squad)
	{
		return Order;
	}

	Order.TargetActor = Squad->SharedTarget;
	Order.bHasTarget = IsValid(Order.TargetActor);
	Order.RecommendedRole = Requester->GetRole();

	if (!Order.bHasTarget)
	{
		return Order;
	}

	const FVector TargetLocation = Order.TargetActor->GetActorLocation();
	AActor* RequesterActor = Requester->GetOwner();
	if (!IsValid(RequesterActor))
	{
		Order.AttackLocation = TargetLocation;
		return Order;
	}

	const FVector Right = FVector::CrossProduct(FVector::UpVector, RequesterActor->GetActorForwardVector()).GetSafeNormal();
	const float Dist = Requester->GetPreferredEngagementDistance();

	switch (Requester->GetRole())
	{
	case EShooterSquadRole::Flanker:
		Order.AttackLocation = TargetLocation + (Right * Dist);
		break;
	case EShooterSquadRole::Suppressor:
		Order.AttackLocation = TargetLocation - (RequesterActor->GetActorForwardVector() * Dist);
		break;
	case EShooterSquadRole::Breacher:
		Order.AttackLocation = TargetLocation + FVector(0.0f, 0.0f, 50.0f);
		break;
	default:
		Order.AttackLocation = TargetLocation;
		break;
	}

	return Order;
}

void UShooterSquadSubsystem::CleanupNullMembers(FShooterSquadRuntime& Squad) const
{
	Squad.Members.RemoveAll([](const TObjectPtr<UShooterSquadComponent>& Member)
	{
		return !IsValid(Member);
	});
}
