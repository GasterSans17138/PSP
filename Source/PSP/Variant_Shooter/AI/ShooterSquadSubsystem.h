#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ShooterSquadTypes.h"
#include "ShooterSquadSubsystem.generated.h"

class UShooterSquadComponent;

USTRUCT()
struct FShooterSquadRuntime
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UShooterSquadComponent>> Members;

	UPROPERTY()
	TObjectPtr<AActor> SharedTarget = nullptr;

	UPROPERTY()
	float LastSquadGrenadeTime = -10000.0f;
};

UCLASS()
class PSP_API UShooterSquadSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterMember(FName SquadId, UShooterSquadComponent* Member);
	void UnregisterMember(FName SquadId, UShooterSquadComponent* Member);

	UFUNCTION(BlueprintCallable, Category = "Squad")
	void SetSquadTarget(FName SquadId, AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Squad")
	FShooterSquadOrder BuildOrder(FName SquadId, const UShooterSquadComponent* Requester) const;

	UFUNCTION(BlueprintCallable, Category = "Squad")
	bool CanSquadThrowGrenade(FName SquadId, float Cooldown) const;

	UFUNCTION(BlueprintCallable, Category = "Squad")
	void MarkSquadGrenadeThrown(FName SquadId);

private:
	UPROPERTY(Transient)
	TMap<FName, FShooterSquadRuntime> Squads;

private:
	void CleanupNullMembers(FShooterSquadRuntime& Squad) const;
	void ReassignRoles(FShooterSquadRuntime& Squad) const;

	EShooterSquadRole AssignRoleForMemberIndex(int32 MemberIndex) const;

	EShooterTacticalOrder ComputeBaseTacticalOrder(
		const TArray<UShooterSquadComponent*>& ValidMembers,
		const UShooterSquadComponent* Requester,
		AActor* TargetActor) const;

	EShooterTacticalOrder ComputeCoverTacticalOrder(
		const UShooterSquadComponent* Requester) const;

	FVector ComputeMoveLocation(
		const UShooterSquadComponent* Requester,
		const AActor* TargetActor,
		EShooterTacticalOrder TacticalOrder) const;

	bool HasAnyOtherMemberPeeking(
		const TArray<UShooterSquadComponent*>& ValidMembers,
		const UShooterSquadComponent* Requester) const;
};