#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterSquadTypes.h"
#include "ShooterSquadComponent.generated.h"

class UShooterSquadSubsystem;

USTRUCT(BlueprintType)
struct FShooterSquadMemberRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bHasWeapon = false;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bHasLineOfSight = false;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bReachedTacticalMoveLocation = false;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	float DistanceToTarget = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bIsPeeking = false;
};

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class PSP_API UShooterSquadComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShooterSquadComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad")
	FName SquadId = TEXT("EnemySquadA");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Squad")
	EShooterSquadRole Role = EShooterSquadRole::Assaulter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad", meta = (ClampMin = 100, ClampMax = 5000, Units = "cm"))
	float PreferredEngagementDistance = 750.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "Squad")
	void BroadcastTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Squad")
	FShooterSquadOrder GetCurrentOrder() const;

	UFUNCTION(BlueprintCallable, Category = "Squad")
	void SetRole(EShooterSquadRole NewRole) { Role = NewRole; }

	UFUNCTION(BlueprintPure, Category = "Squad")
	FName GetSquadId() const { return SquadId; }

	UFUNCTION(BlueprintPure, Category = "Squad")
	EShooterSquadRole GetRole() const { return Role; }

	UFUNCTION(BlueprintPure, Category = "Squad")
	float GetPreferredEngagementDistance() const { return PreferredEngagementDistance; }

	UFUNCTION(BlueprintCallable, Category = "Squad")
	void SetRuntimeState(const FShooterSquadMemberRuntimeState& NewState) { RuntimeState = NewState; }

	UFUNCTION(BlueprintPure, Category = "Squad")
	const FShooterSquadMemberRuntimeState& GetRuntimeState() const { return RuntimeState; }

	UFUNCTION(BlueprintCallable, Category = "Squad")
	bool CanThrowSquadGrenade(float Cooldown) const;

	UFUNCTION(BlueprintCallable, Category = "Squad")
	void MarkSquadGrenadeThrown();

private:
	UPROPERTY(Transient)
	FShooterSquadMemberRuntimeState RuntimeState;

	UShooterSquadSubsystem* GetSquadSubsystem() const;
};