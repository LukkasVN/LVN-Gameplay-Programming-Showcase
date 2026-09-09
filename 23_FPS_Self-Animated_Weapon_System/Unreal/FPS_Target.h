#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPS_Hittable.h"
#include "FPS_Target.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFPS_OnTargetHit, int32, HitCount, FVector, HitPoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFPS_OnTargetDestroyed);

/**
 * Damage is received and ignored on purpose this target is a simple pop up target for testing.
 */
UCLASS()
class MECHANICS_TEST_LVN_API AFPS_Target : public AActor, public IFPS_Hittable
{
	GENERATED_BODY()

public:
	AFPS_Target();

	virtual void Tick(float DeltaSeconds) override;

	virtual void TakeHit_Implementation(float Damage, FVector HitPoint, FVector HitDirection) override;

	UPROPERTY(BlueprintAssignable, Category = "FPS|Target|Events")
	FFPS_OnTargetHit OnTargetHit;

	UPROPERTY(BlueprintAssignable, Category = "FPS|Target|Events")
	FFPS_OnTargetDestroyed OnTargetDestroyed;

	UFUNCTION(BlueprintPure, Category = "FPS|Target")
	int32 GetHitCount() const { return HitCount; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS|Target")
	TObjectPtr<USceneComponent> TargetRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Target", meta = (ClampMin = "1"))
	int32 HitsToDestroy = 5;


	UPROPERTY(EditAnywhere, Category = "FPS|Target|Pop", meta = (ClampMin = "0.01"))
	float PopUpScale = 1.6f;

	UPROPERTY(EditAnywhere, Category = "FPS|Target|Pop", meta = (ClampMin = "0.01"))
	float PopUpDuration = 0.06f;

	UPROPERTY(EditAnywhere, Category = "FPS|Target|Pop", meta = (ClampMin = "0.01"))
	float PopDownScale = 0.85f;

	UPROPERTY(EditAnywhere, Category = "FPS|Target|Pop", meta = (ClampMin = "0.01"))
	float PopDownDuration = 0.08f;

	UPROPERTY(EditAnywhere, Category = "FPS|Target|Pop", meta = (ClampMin = "0.01"))
	float PopReturnDuration = 0.12f;

	UPROPERTY(EditAnywhere, Category = "FPS|Target")
	float DestroyDelay = 0.05f;

private:
	enum class EPopPhase : uint8
	{
		Idle,
		Up,
		Down,
		Return
	};

	void HandleDestroy();
	void StartPhase(EPopPhase NewPhase);
	float GetPhaseDuration() const;
	float GetPhaseTargetScale() const;

	FVector BaseScale = FVector::OneVector;
	int32 HitCount = 0;
	FTimerHandle DestroyTimerHandle;

	EPopPhase Phase = EPopPhase::Idle;
	float PhaseElapsed = 0.0f;

	float CurrentScale = 1.0f;
	float PhaseStartScale = 1.0f;
};
