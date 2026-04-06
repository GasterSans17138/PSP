#include "ShooterSTTask_GetWeapon.h"
#include "ShooterAIController.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"

AShooterAIController* FShooterSTTask_GetWeapon::GetController(FStateTreeExecutionContext& Context) const
{
	return Cast<AShooterAIController>(Context.GetOwner());
}

EStateTreeRunStatus FShooterSTTask_GetWeapon::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AShooterAIController* Controller = GetController(Context);
	if (!IsValid(Controller))
	{
		return EStateTreeRunStatus::Failed;
	}

	Controller->SetFireEnabled(false);
	Controller->RefreshControlledAIState();

	if (Controller->HasUsableWeapon())
	{
		Controller->ClearWeaponTarget();
		return EStateTreeRunStatus::Succeeded;
	}

	AActor* WeaponPickup = Controller->FindBestWeaponPickup(InstanceData.SearchRadius);
	if (!IsValid(WeaponPickup))
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FShooterSTTask_GetWeapon::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AShooterAIController* Controller = GetController(Context);
	if (!IsValid(Controller))
	{
		return EStateTreeRunStatus::Failed;
	}

	Controller->SetFireEnabled(false);
	Controller->RefreshControlledAIState();

	// Success condition: overlap pickup already granted a weapon.
	if (Controller->HasUsableWeapon())
	{
		Controller->ClearWeaponTarget();
		return EStateTreeRunStatus::Succeeded;
	}

	AActor* WeaponTarget = Controller->GetWeaponTarget();

	const bool bTargetInvalid = !IsValid(WeaponTarget) || WeaponTarget->IsHidden();

	if (bTargetInvalid)
	{
		if (!InstanceData.bRetrySearchIfTargetInvalid)
		{
			return EStateTreeRunStatus::Failed;
		}

		WeaponTarget = Controller->FindBestWeaponPickup(InstanceData.SearchRadius);
		if (!IsValid(WeaponTarget))
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	if (!Controller->MoveToWeaponTarget(InstanceData.MoveAcceptanceRadius))
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

void FShooterSTTask_GetWeapon::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (AShooterAIController* Controller = GetController(Context))
	{
		Controller->SetFireEnabled(false);
		Controller->ClearWeaponTarget();
		Controller->RefreshControlledAIState();
	}
}