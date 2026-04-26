#pragma once

#include "CoreMinimal.h"
#include "ShooterCharacter.h"
#include "ShooterSquadTypes.h"
#include "ShooterAICharacter.generated.h"

class UShooterSquadComponent;
class AShooterAIController;
class AShooterWeapon;

UCLASS()
class PSP_API AShooterAICharacter : public AShooterCharacter
{
	GENERATED_BODY()

public:
	AShooterAICharacter();

	UFUNCTION(BlueprintCallable, Category = "AI|Squad")
	void RefreshSquadOrder();

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void RefreshAIState();

	UFUNCTION(BlueprintCallable, Category = "AI|Suppression")
	void AddSuppression(float Amount);

	UFUNCTION(BlueprintCallable, Category = "AI|Suppression")
	void ClearSuppression();

	UFUNCTION(BlueprintPure, Category = "AI|Suppression")
	float GetSuppressionValue() const { return SuppressionValue; }

	UFUNCTION(BlueprintPure, Category = "AI|Suppression")
	float GetSuppressionAlpha() const;

	UFUNCTION(BlueprintPure, Category = "AI|Squad")
	const FShooterSquadOrder& GetCachedOrder() const { return CachedOrder; }

	UFUNCTION(BlueprintPure, Category = "Weapons")
	AShooterWeapon* GetCurrentWeaponActor() const { return CurrentWeapon.Get(); }

	virtual void PlayFiringMontage(UAnimMontage* Montage) override;
	virtual FVector GetWeaponTargetLocation() override;

	virtual float TakeDamage(
		float Damage,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	virtual void Tick(float DeltaTime) override;

	virtual bool UsesFirstPersonPresentation() const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bHasWeapon = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bHasWeaponTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bHasCombatTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bOrderIsTakeCover = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bOrderIsPeek = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bOrderIsThrowGrenade = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	EShooterTacticalOrder CurrentTacticalOrder = EShooterTacticalOrder::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	FVector CurrentSquadMoveLocation = FVector::ZeroVector;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UShooterSquadComponent> SquadComponent;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Squad")
	FShooterSquadOrder CachedOrder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Suppression", meta = (ClampMin = 0))
	float MaxSuppressionValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Suppression", meta = (ClampMin = 0))
	float SuppressionDecayPerSecond = 18.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Suppression")
	float SuppressionValue = 0.0f;
};