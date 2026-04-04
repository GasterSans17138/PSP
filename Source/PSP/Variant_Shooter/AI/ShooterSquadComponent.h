#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterSquadTypes.h"
#include "ShooterSquadComponent.generated.h"

class UShooterSquadSubsystem;

/**
 * Squad component consumed by State Tree tasks/evaluators.
 * It exposes a light API so BP State Trees can query and react to squad orders.
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class PSP_API UShooterSquadComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UShooterSquadComponent();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Stable identifier for one AI team. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad")
	FName SquadId = TEXT("EnemySquadA");

	/** Combat role used to compute the order position around target. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad")
	EShooterSquadRole Role = EShooterSquadRole::Assaulter;

	/** Preferred radius to keep around the target (centimeters). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad", meta = (ClampMin = 100, ClampMax = 5000, Units = "cm"))
	float PreferredEngagementDistance = 750.0f;

public:

	UFUNCTION(BlueprintCallable, Category = "Squad")
	void BroadcastTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Squad")
	FShooterSquadOrder GetCurrentOrder() const;

	FORCEINLINE FName GetSquadId() const { return SquadId; }
	FORCEINLINE EShooterSquadRole GetRole() const { return Role; }
	FORCEINLINE float GetPreferredEngagementDistance() const { return PreferredEngagementDistance; }

private:

	UShooterSquadSubsystem* GetSquadSubsystem() const;
};
