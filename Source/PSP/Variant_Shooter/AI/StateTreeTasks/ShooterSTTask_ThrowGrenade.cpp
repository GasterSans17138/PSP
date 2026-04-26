#include "ShooterSTTask_ThrowGrenade.h"
#include "ShooterAIController.h"
#include "ShooterAICharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "StateTreeExecutionContext.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterCoverPoint.h"
#include "ShooterSquadComponent.h"

AShooterAIController* FShooterSTTask_ThrowGrenade::GetController(FStateTreeExecutionContext& Context) const
{
	return Cast<AShooterAIController>(Context.GetOwner());
}

AShooterAICharacter* FShooterSTTask_ThrowGrenade::GetAICharacter(FStateTreeExecutionContext& Context) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		return Cast<AShooterAICharacter>(Controller->GetPawn());
	}

	return nullptr;
}

EStateTreeRunStatus FShooterSTTask_ThrowGrenade::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult&) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* Controller = GetController(Context);
	AShooterAICharacter* AIChar = GetAICharacter(Context);

	if (!IsValid(Controller) || !IsValid(AIChar))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!Data.GrenadeClass)
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.StateEnterTime = Controller->GetWorld()->GetTimeSeconds();
	Data.bGrenadeThrown = false;

	Controller->StopMovement();
	Controller->SetFireEnabled(false);

	if (AActor* Target = AIChar->GetCachedOrder().TargetActor)
	{
		Controller->SetFocus(Target);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_ThrowGrenade::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	AShooterAIController* Controller = GetController(Context);
	AShooterAICharacter* AIChar = GetAICharacter(Context);

	if (!IsValid(Controller) || !IsValid(AIChar))
	{
		return EStateTreeRunStatus::Failed;
	}

	AIChar->RefreshAIState();

	if (!AIChar->bOrderIsThrowGrenade)
	{
		Controller->SetFireEnabled(false);
		return EStateTreeRunStatus::Succeeded;
	}

	const float Time = Controller->GetWorld()->GetTimeSeconds();
	const float Elapsed = Time - Data.StateEnterTime;

	if (!Data.bGrenadeThrown && Elapsed >= Data.ThrowDelay)
	{
		const bool bSpawned = SpawnGrenade(Controller, AIChar, Data);
		Data.bGrenadeThrown = true;

		if (bSpawned)
		{
			Controller->MarkGrenadeThrown();

			if (UShooterSquadComponent* SquadComp = Controller->GetControlledSquadComponent())
			{
				SquadComp->MarkSquadGrenadeThrown();
			}
		}

		Controller->SetCoverCombatPhase(EShooterCoverCombatPhase::ReturnToCover);
		AIChar->RefreshAIState();

		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FShooterSTTask_ThrowGrenade::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult&) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		Controller->SetFireEnabled(false);
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

bool FShooterSTTask_ThrowGrenade::SpawnGrenade(
	AShooterAIController* Controller,
	AShooterAICharacter* AIChar,
	const FInstanceDataType& Data) const
{
	if (!IsValid(Controller) || !IsValid(AIChar) || !Data.GrenadeClass)
	{
		return false;
	}

	UWorld* World = Controller->GetWorld();
	if (!World)
	{
		return false;
	}

	AActor* TargetActor = AIChar->GetCachedOrder().TargetActor;
	if (!IsValid(TargetActor))
	{
		TargetActor = Controller->GetCombatTarget();
	}

	if (!IsValid(TargetActor))
	{
		return false;
	}

	FVector ThrowOrigin = AIChar->GetActorLocation();

	if (AShooterCoverPoint* CoverPoint = Controller->GetCurrentCoverPoint())
	{
		ThrowOrigin = CoverPoint->GetPeekLocation();
	}

	// Slightly before the player so grenades don't overshoot too much.
	FVector TargetLocation = TargetActor->GetActorLocation();

	const FVector FromTargetToAI =
		(AIChar->GetActorLocation() - TargetLocation).GetSafeNormal2D();

	if (!FromTargetToAI.IsNearlyZero())
	{
		TargetLocation += FromTargetToAI * Data.TargetShortOffset;
	}

	TargetLocation.Z += 40.0f;

	const FVector Forward =
		(TargetLocation - ThrowOrigin).GetSafeNormal2D();

	const FVector SpawnLocation =
		ThrowOrigin +
		(Forward * Data.SpawnForwardOffset) +
		FVector(0.0f, 0.0f, Data.SpawnUpOffset);

	// REAL ballistic solve instead of fixed throw force.
	FVector LaunchVelocity = FVector::ZeroVector;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AIChar);
	ActorsToIgnore.Add(Controller->GetPawn());

	const bool bHasValidBallisticArc = UGameplayStatics::SuggestProjectileVelocity(
		World,
		LaunchVelocity,
		SpawnLocation,
		TargetLocation,
		Data.ThrowSpeed,
		false,
		0.0f,
		0.0f,
		ESuggestProjVelocityTraceOption::DoNotTrace,
		FCollisionResponseParams::DefaultResponseParam,
		ActorsToIgnore,
		false
	);

	if (!bHasValidBallisticArc)
	{
		// Fallback if solver fails
		FVector ToTarget = TargetLocation - SpawnLocation;
		FVector Horizontal = ToTarget;
		Horizontal.Z = 0.0f;

		const FVector HorizontalDir = Horizontal.GetSafeNormal();

		LaunchVelocity =
			(HorizontalDir * Data.ThrowSpeed) +
			FVector(0.0f, 0.0f, Data.ThrowUpBoost);
	}

	if (LaunchVelocity.Size() > Data.MaxThrowSpeed)
	{
		LaunchVelocity =
			LaunchVelocity.GetSafeNormal() * Data.MaxThrowSpeed;
	}

	// Safety check: prevent self-cover collision
	if (Data.bCheckThrowPathBeforeThrow)
	{
		FPredictProjectilePathParams PathParams;
		PathParams.StartLocation = SpawnLocation;
		PathParams.LaunchVelocity = LaunchVelocity;
		PathParams.ProjectileRadius = Data.ThrowPathProjectileRadius;
		PathParams.MaxSimTime = Data.ThrowPathCheckTime;
		PathParams.SimFrequency = Data.ThrowPathSimFrequency;
		PathParams.TraceChannel = ECC_Visibility;
		PathParams.bTraceWithCollision = true;
		PathParams.bTraceWithChannel = true;

		PathParams.ActorsToIgnore.Add(AIChar);
		PathParams.ActorsToIgnore.Add(Controller->GetPawn());

		FPredictProjectilePathResult PathResult;

		const bool bPathBlocked =
			UGameplayStatics::PredictProjectilePath(
				World,
				PathParams,
				PathResult);

		if (bPathBlocked)
		{
			AActor* HitActor = PathResult.HitResult.GetActor();

			// If we instantly hit something that is not the player,
			// cancel the throw.
			if (HitActor != TargetActor)
			{
				return false;
			}
		}
	}

	const FRotator SpawnRotation =
		LaunchVelocity.GetSafeNormal().Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AIChar;
	SpawnParams.Instigator = AIChar;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Grenade =
		World->SpawnActor<AActor>(
			Data.GrenadeClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams);

	if (!IsValid(Grenade))
	{
		return false;
	}

	if (UProjectileMovementComponent* ProjectileMovement =
		Grenade->FindComponentByClass<UProjectileMovementComponent>())
	{
		ProjectileMovement->Velocity = LaunchVelocity;
		ProjectileMovement->Activate(true);
	}

	return true;
}