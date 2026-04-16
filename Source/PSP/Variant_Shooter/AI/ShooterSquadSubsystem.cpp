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
		Order.TacticalOrder = EShooterTacticalOrder::Hold;
		return Order;
	}

	Order.TacticalOrder = ComputeTacticalOrder(Squad->Members, Requester);
	Order.MoveLocation = ComputeMoveLocation(Requester, Order.TargetActor, Order.TacticalOrder);
	Order.AttackLocation = Order.TargetActor->GetActorLocation();

	return Order;
}

void UShooterSquadSubsystem::CleanupNullMembers(FShooterSquadRuntime& Squad) const
{
	Squad.Members.RemoveAll([](const TObjectPtr<UShooterSquadComponent>& Member)
	{
		return !IsValid(Member);
	});
}

EShooterTacticalOrder UShooterSquadSubsystem::ComputeTacticalOrder(
	const TArray<TObjectPtr<UShooterSquadComponent>>& Members,
	const UShooterSquadComponent* Requester) const
{
	if (!Requester)
	{
		return EShooterTacticalOrder::None;
	}

	TArray<UShooterSquadComponent*> ValidMembers;
	ValidMembers.Reserve(Members.Num());

	for (UShooterSquadComponent* Member : Members)
	{
		if (IsValid(Member))
		{
			ValidMembers.Add(Member);
		}
	}

	if (ValidMembers.Num() == 0)
	{
		return EShooterTacticalOrder::None;
	}

	// Sort to make role assignment stable and deterministic.
	ValidMembers.Sort([](const UShooterSquadComponent& A, const UShooterSquadComponent& B)
	{
		const AActor* OwnerA = A.GetOwner();
		const AActor* OwnerB = B.GetOwner();

		const FString NameA = IsValid(OwnerA) ? OwnerA->GetName() : FString();
		const FString NameB = IsValid(OwnerB) ? OwnerB->GetName() : FString();
		return NameA < NameB;
	});

	const int32 MemberIndex = ValidMembers.IndexOfByKey(Requester);
	if (MemberIndex == INDEX_NONE)
	{
		return EShooterTacticalOrder::None;
	}

	// Profile role can override default slot behavior.
	switch (Requester->GetRole())
	{
	case EShooterSquadRole::Suppressor:
		return EShooterTacticalOrder::Suppress;

	case EShooterSquadRole::Flanker:
		return (MemberIndex % 2 == 0)
			? EShooterTacticalOrder::FlankLeft
			: EShooterTacticalOrder::FlankRight;

	case EShooterSquadRole::Breacher:
		return EShooterTacticalOrder::Push;

	default:
		break;
	}

	// Generic fallback distribution.
	if (MemberIndex == 0)
	{
		return EShooterTacticalOrder::Suppress;
	}
	if (MemberIndex == 1)
	{
		return EShooterTacticalOrder::Push;
	}
	if (MemberIndex == 2)
	{
		return EShooterTacticalOrder::FlankLeft;
	}
	if (MemberIndex == 3)
	{
		return EShooterTacticalOrder::FlankRight;
	}

	return EShooterTacticalOrder::Hold;
}

FVector UShooterSquadSubsystem::ComputeMoveLocation(
	const UShooterSquadComponent* Requester,
	const AActor* TargetActor,
	EShooterTacticalOrder TacticalOrder) const
{
	if (!Requester || !IsValid(TargetActor))
	{
		return FVector::ZeroVector;
	}

	const AActor* RequesterActor = Requester->GetOwner();
	if (!IsValid(RequesterActor))
	{
		return TargetActor->GetActorLocation();
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();

	FVector Forward = TargetActor->GetActorForwardVector().GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::ForwardVector;
	}

	const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();

	const float BaseDist = Requester->GetPreferredEngagementDistance();
	const float PushDist = FMath::Max(150.0f, BaseDist * 0.45f);
	const float SuppressDist = BaseDist * 1.15f;
	const float FlankForwardDist = BaseDist * 0.85f;
	const float FlankSideDist = BaseDist * 0.9f;

	switch (TacticalOrder)
	{
	case EShooterTacticalOrder::Push:
		return TargetLocation + (Forward * PushDist);

	case EShooterTacticalOrder::Suppress:
		return TargetLocation + (Forward * SuppressDist);

	case EShooterTacticalOrder::FlankLeft:
		return TargetLocation - (Right * FlankSideDist);

	case EShooterTacticalOrder::FlankRight:
		return TargetLocation + (Right * FlankSideDist);

	case EShooterTacticalOrder::Hold:
		return RequesterActor->GetActorLocation();

	case EShooterTacticalOrder::Regroup:
		return TargetLocation + (Forward * (BaseDist * 1.3f));

	default:
		return TargetLocation + (Forward * BaseDist);
	}
}