#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FPS_Spring.h"
#include "FPS_WeaponTypes.h"
#include "FPS_ViewmodelFeedbackComponent.generated.h"

class AFPS_Character;
class UFPS_WeaponHolderComponent;

/**
 * Lives on the weapon actor and drives the mesh pivot's relative transform.
 *
 * Reads only generic state: movement state, move input, reload state, and the
 * holder's delegates. It never asks what weapon it is on.
 *
 * The holder moves the attach offset and this moves the mesh inside the actor,
 * so the two never fight over one transform. Preserve that separation.
 */
UCLASS(ClassGroup = (FPS), meta = (BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API UFPS_ViewmodelFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFPS_ViewmodelFeedbackComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Component tag on the scene component this drives. The weapon actor tags its mesh pivot with it. */
	UPROPERTY(EditAnywhere, Category = "FPS|Feedback")
	FName PivotTag = TEXT("ViewmodelPivot");

	// Springs

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Spring")
	float LocationStiffness = 220.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Spring")
	float LocationDamping = 18.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Spring")
	float RotationStiffness = 200.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Spring")
	float RotationDamping = 17.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Spring")
	float ScaleStiffness = 240.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Spring")
	float ScaleDamping = 19.0f;

	// Fire recoil

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Fire")
	float RecoilKickback = -34.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Fire")
	float RecoilLift = 6.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Fire")
	float RecoilPitch = -140.0f;

	/** Alternates sign per shot so a burst does not walk one way. */
	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Fire")
	float RecoilRoll = 55.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Fire")
	float RecoilStretch = 0.8f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Fire")
	float RecoilSquash = 0.35f;

	// Reload

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Reload")
	FVector ReloadPoseLocation = FVector(-3.0f, 1.0f, -2.5f);

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Reload")
	FRotator ReloadPoseRotation = FRotator(22.0f, -8.0f, 12.0f);

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Reload")
	float ReloadPoseBlendSpeed = 9.0f;

	/** A punch on both the entry and exit edges is what makes the reload snap instead of glide. */
	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Reload")
	float ReloadPunchRotation = 90.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Reload")
	float ReloadPunchLocation = 16.0f;

	// Fire mode switch

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|FireMode")
	float FireModeRoll = 110.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|FireMode")
	float FireModeSquash = 0.5f;

	// Landing

	/** Softened, since the camera already dips hard on the same impact. */
	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Landing")
	float LandingKick = -18.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Landing")
	float LandingPitch = 40.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Landing")
	float LandingMaxImpactSpeed = 1600.0f;

	// Idle drift

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Idle")
	float IdleDriftFrequency = 1.1f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Idle")
	float IdleDriftLocation = 0.35f;

	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Idle")
	float IdleDriftRotation = 0.7f;

	/** Everything except the reload pose is scaled down by this much at full aim. */
	UPROPERTY(EditAnywhere, Category = "FPS|Feedback|Aim", meta = (ClampMin = "0", ClampMax = "1"))
	float AimDamping = 0.75f;

private:
	USceneComponent* ResolvePivot();
	void SubscribeToHolder();
	void UnsubscribeFromHolder();

	UFUNCTION()
	void HandleWeaponFired(const FFPS_WeaponFireInfo& FireInfo);

	UFUNCTION()
	void HandleFireModeChanged(EFPS_FireMode NewFireMode);

	UFUNCTION()
	void HandleLandedImpact(float ImpactSpeed);

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> Pivot;

	UPROPERTY(Transient)
	TObjectPtr<UFPS_WeaponHolderComponent> Holder;

	UPROPERTY(Transient)
	TObjectPtr<AFPS_Character> Character;

	FFPS_VectorSpring LocationSpring;
	FFPS_VectorSpring RotationSpring;
	FFPS_VectorSpring ScaleSpring;

	FVector BasePivotLocation = FVector::ZeroVector;
	FRotator BasePivotRotation = FRotator::ZeroRotator;
	FVector BasePivotScale = FVector::OneVector;

	float ReloadPoseAlpha = 0.0f;
	bool bWasReloading = false;
	float RecoilRollSign = 1.0f;
	float DriftTime = 0.0f;
};
