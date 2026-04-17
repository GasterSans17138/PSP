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
		ReassignRoles(Squad);
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
		else
		{
			ReassignRoles(*Squad);
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

	if (const AActor* RequesterActor = Requester->GetOwner())
	{
		Order.bReachedMoveLocation =
			!Order.MoveLocation.IsNearlyZero() &&
			FVector::Dist(RequesterActor->GetActorLocation(), Order.MoveLocation) <= 300.0f;
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

void UShooterSquadSubsystem::ReassignRoles(FShooterSquadRuntime& Squad) const
{
	TArray<UShooterSquadComponent*> ValidMembers;
	ValidMembers.Reserve(Squad.Members.Num());

	for (UShooterSquadComponent* Member : Squad.Members)
	{
		if (IsValid(Member))
		{
			ValidMembers.Add(Member);
		}
	}

	ValidMembers.Sort([](const UShooterSquadComponent& A, const UShooterSquadComponent& B)
	{
		const AActor* OwnerA = A.GetOwner();
		const AActor* OwnerB = B.GetOwner();

		const FString NameA = IsValid(OwnerA) ? OwnerA->GetName() : FString();
		const FString NameB = IsValid(OwnerB) ? OwnerB->GetName() : FString();
		return NameA < NameB;
	});

	for (int32 Index = 0; Index < ValidMembers.Num(); ++Index)
	{
		ValidMembers[Index]->SetRole(AssignRoleForMemberIndex(Index));
	}
}

EShooterSquadRole UShooterSquadSubsystem::AssignRoleForMemberIndex(int32 MemberIndex) const
{
	switch (MemberIndex)
	{
	case 0:
		return EShooterSquadRole::Suppressor;

	case 1:
		return EShooterSquadRole::Breacher;

	case 2:
		return EShooterSquadRole::Flanker;

	default:
		return EShooterSquadRole::Assaulter;
	}
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

	if (ValidMembers.IsEmpty())
	{
		return EShooterTacticalOrder::None;
	}

	ValidMembers.Sort([](const UShooterSquadComponent& A, const UShooterSquadComponent& B)
	{
		const AActor* OwnerA = A.GetOwner();
		const AActor* OwnerB = B.GetOwner();

		const FString NameA = IsValid(OwnerA) ? OwnerA->GetName() : FString();
		const FString NameB = IsValid(OwnerB) ? OwnerB->GetName() : FString();
		return NameA < NameB;
	});

	AActor* SharedTarget = nullptr;
	if (const FShooterSquadRuntime* Squad = Squads.Find(Requester->GetSquadId()))
	{
		SharedTarget = Squad->SharedTarget;
	}

	return ComputeDynamicTacticalOrder(ValidMembers, Requester, SharedTarget);
}

EShooterTacticalOrder UShooterSquadSubsystem::ComputeDynamicTacticalOrder(
	const TArray<UShooterSquadComponent*>& ValidMembers,
	const UShooterSquadComponent* Requester,
	AActor* TargetActor) const
{
	if (!Requester || !IsValid(TargetActor))
	{
		return EShooterTacticalOrder::Hold;
	}

	const FShooterSquadMemberRuntimeState& MyState = Requester->GetRuntimeState();

	switch (Requester->GetRole())
	{
	case EShooterSquadRole::Flanker:
	{
		if (MyState.bReachedTacticalMoveLocation && MyState.bHasLineOfSight)
		{
			return EShooterTacticalOrder::Suppress;
		}

		return EShooterTacticalOrder::FlankLeft;
	}

	case EShooterSquadRole::Breacher:
	{
		if (MyState.DistanceToTarget > 550.0f)
		{
			return EShooterTacticalOrder::Push;
		}

		if (MyState.bHasLineOfSight)
		{
			return EShooterTacticalOrder::Hold;
		}

		return EShooterTacticalOrder::Push;
	}

	case EShooterSquadRole::Suppressor:
		{
			if (!MyState.bHasLineOfSight)
			{
				return EShooterTacticalOrder::TakeCover;
			}
			return EShooterTacticalOrder::Suppress;
		}

	case EShooterSquadRole::Assaulter:
	default:
	{
		if (MyState.DistanceToTarget > 1100.0f)
		{
			return EShooterTacticalOrder::Push;
		}

		if (MyState.bHasLineOfSight)
		{
			return EShooterTacticalOrder::Suppress;
		}

		return EShooterTacticalOrder::Hold;
	}
	}
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

	FVector ToRequester = (RequesterActor->GetActorLocation() - TargetLocation).GetSafeNormal();
	if (ToRequester.IsNearlyZero())
	{
		ToRequester = FVector::BackwardVector;
	}

	const FVector LocalRight = FVector::CrossProduct(FVector::UpVector, ToRequester).GetSafeNormal();

	// Keep player forward only for flank logic if you want the flank to react to player facing.
	FVector TargetForward = TargetActor->GetActorForwardVector().GetSafeNormal();
	if (TargetForward.IsNearlyZero())
	{
		TargetForward = FVector::ForwardVector;
	}

	const FVector TargetRight = FVector::CrossProduct(FVector::UpVector, TargetForward).GetSafeNormal();

	const float BaseDist = Requester->GetPreferredEngagementDistance();

	switch (TacticalOrder)
	{
	case EShooterTacticalOrder::Push:
		// Close frontal position from current approach side.
		return TargetLocation + (ToRequester * FMath::Max(250.0f, BaseDist * 0.45f));

	case EShooterTacticalOrder::Suppress:
		// Farther frontal support, slightly offset from current side.
		return TargetLocation + (ToRequester * (BaseDist * 1.0f)) + (LocalRight * 180.0f);

	case EShooterTacticalOrder::Hold:
		// Mid frontal support, opposite offset from current side.
		return TargetLocation + (ToRequester * (BaseDist * 0.8f)) - (LocalRight * 180.0f);

	case EShooterTacticalOrder::FlankLeft:
		return TargetLocation - (TargetRight * (BaseDist * 0.55f)) - (TargetForward * (BaseDist * 0.25f));

	case EShooterTacticalOrder::FlankRight:
		return TargetLocation + (TargetRight * (BaseDist * 0.55f)) - (TargetForward * (BaseDist * 0.25f));

	case EShooterTacticalOrder::Regroup:
		return TargetLocation + (ToRequester * (BaseDist * 1.3f));

	default:
		return TargetLocation + (ToRequester * BaseDist);
	}
}