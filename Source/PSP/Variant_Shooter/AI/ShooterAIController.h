#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShooterAIController.generated.h"

class UStateTreeAIComponent;
class UShooterSquadComponent;
class AShooterCoverPoint;

UENUM(BlueprintType)
enum class EShooterCoverCombatPhase : uint8
{
	None,
	TakingCover,
	Peek,
	ThrowGrenade,
	ReturnToCover
};

UCLASS()
class PSP_API AShooterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AShooterAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	bool bStartLogicOnPossess = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;

public:
	UFUNCTION(BlueprintPure, Category = "AI|Squad")
	UShooterSquadComponent* GetControlledSquadComponent() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	APawn* AcquirePlayerTarget();

	UFUNCTION(BlueprintPure, Category = "AI|Combat")
	APawn* GetCombatTarget() const { return CombatTarget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool MoveToCombatTarget(float AcceptanceRadius = 550.0f, bool bCanStrafe = true);

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void SetFireEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void UpdateFocusOnCombatTarget();

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool HasLineOfSightToActor(AActor* TargetActor) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool MoveToTacticalLocation(const FVector& Location, float AcceptanceRadius = 100.0f, bool bCanStrafe = true);

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	bool HasUsableWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	AActor* FindBestWeaponPickup(float SearchRadius = 3000.0f);

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	bool MoveToWeaponTarget(float AcceptanceRadius = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	void ClearWeaponTarget();

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	AActor* GetWeaponTarget() const { return WeaponTarget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void RefreshControlledAIState();

	UFUNCTION(BlueprintPure, Category = "AI|Cover")
	AShooterCoverPoint* GetCurrentCoverPoint() const { return CurrentCoverPoint.Get(); }

	UFUNCTION(BlueprintCallable, Category = "AI|Cover")
	void SetCurrentCoverPoint(AShooterCoverPoint* NewCoverPoint);

	UFUNCTION(BlueprintCallable, Category = "AI|Cover")
	void ClearCurrentCoverPoint();

	UFUNCTION(BlueprintCallable, Category = "AI|Cover")
	bool MoveToCoverPoint(float AcceptanceRadius = 100.0f, bool bCanStrafe = true);

	UFUNCTION(BlueprintCallable, Category = "AI|Cover")
	bool MoveToPeekLocation(float AcceptanceRadius = 75.0f, bool bCanStrafe = true);

	UFUNCTION(BlueprintCallable, Category = "AI|Cover")
	void SetCoverCombatPhase(EShooterCoverCombatPhase NewPhase);

	UFUNCTION(BlueprintPure, Category = "AI|Cover")
	EShooterCoverCombatPhase GetCoverCombatPhase() const;

	UFUNCTION(BlueprintPure, Category = "AI|Grenade")
	bool CanThrowGrenade(float Cooldown) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Grenade")
	void MarkGrenadeThrown();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "AI|Combat")
	TWeakObjectPtr<APawn> CombatTarget;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Weapon")
	TWeakObjectPtr<AActor> WeaponTarget;

	UPROPERTY(Transient)
	TWeakObjectPtr<AShooterCoverPoint> CurrentCoverPoint;

	UPROPERTY(Transient)
	EShooterCoverCombatPhase CoverCombatPhase = EShooterCoverCombatPhase::None;

	UPROPERTY(Transient)
	float LastGrenadeThrowTime = -10000.0f;
};