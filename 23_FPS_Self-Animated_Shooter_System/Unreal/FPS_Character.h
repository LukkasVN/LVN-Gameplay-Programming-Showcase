#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPS_WeaponTypes.h"
#include "FPS_Spring.h"
#include "FPS_Character.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFPS_OnLandedImpact, float, ImpactSpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFPS_OnDashStarted);

/**
 * Arcade first person character. Movement and camera feel only, zero weapon awareness.
 *
 * The single contact point with the weapon system is the camera zoom override,
 * which carries a raw FOV value rather than a weapon reference. That is what
 * keeps this class from ever learning that weapons exist.
 */
UCLASS()
class MECHANICS_TEST_LVN_API AFPS_Character : public ACharacter
{
	GENERATED_BODY()

public:
	AFPS_Character();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;

	// Integration points

	UPROPERTY(BlueprintAssignable, Category = "FPS|Character|Events")
	FFPS_OnLandedImpact OnLandedImpact;

	UPROPERTY(BlueprintAssignable, Category = "FPS|Character|Events")
	FFPS_OnDashStarted OnDashStarted;

	/**
	 * An external system writes this while it wants FOV control and clears it on
	 * release. Impulses still layer on top, only the walk and run blend is suppressed.
	 */
	UFUNCTION(BlueprintCallable, Category = "FPS|Character|Camera")
	void SetCameraZoomOverride(float InFOV);

	UFUNCTION(BlueprintCallable, Category = "FPS|Character|Camera")
	void ClearCameraZoomOverride();

	UFUNCTION(BlueprintPure, Category = "FPS|Character|Camera")
	float GetBaseFOV() const { return BaseFOV; }

	UFUNCTION(BlueprintPure, Category = "FPS|Character|Camera")
	UCameraComponent* GetFirstPersonCamera() const { return CameraComponent; }

	UFUNCTION(BlueprintPure, Category = "FPS|Character|Movement")
	EFPS_MovementState GetMovementState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "FPS|Character|Movement")
	FVector2D GetMoveInput() const { return MoveInput; }

	UFUNCTION(BlueprintPure, Category = "FPS|Character|Movement")
	bool IsDashing() const { return DashTimeRemaining > 0.0f; }

	UFUNCTION(BlueprintPure, Category = "FPS|Character|Movement")
	float GetDashCooldownNormalized() const;

	UFUNCTION(BlueprintPure, Category = "FPS|Character|Movement")
	float GetDashCooldownRemaining() const { return FMath::Max(0.0f, DashCooldownRemaining); }

	UFUNCTION(BlueprintPure, Category = "FPS|Character|Movement")
	bool IsDashReady() const { return DashCooldownRemaining <= 0.0f && DashTimeRemaining <= 0.0f; }

protected:
	virtual void BeginPlay() override;

	// Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS|Character")
	TObjectPtr<UCameraComponent> CameraComponent;

	// Input

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	int32 MappingContextPriority = 0;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> RunAction;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	float LookSensitivity = 1.0f;

	// Movement

	UPROPERTY(EditAnywhere, Category = "FPS|Movement")
	float WalkSpeed = 480.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Movement")
	float RunSpeed = 750.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Movement")
	float JumpVelocity = 620.0f;

	// Gravity
	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Gravity")
	float RiseGravityScale = 1.6f;

	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Gravity")
	float FallGravityMultiplier = 2.1f;

	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Gravity")
	float TerminalVelocity = 2400.0f;

	// Dash

	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Dash")
	float DashSpeed = 2200.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Dash")
	float DashDuration = 0.16f;

	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Dash")
	float DashCooldown = 1.4f;

	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Dash")
	float DashFOVKick = 12.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Dash")
	float DashFOVInSpeed = 22.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Dash")
	float DashFOVOutSpeed = 5.0f;

	/** Dash */
	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Dash")
	float DashBankAngle = 7.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Movement|Dash")
	float DashBankInterpSpeed = 12.0f;

	// Camera tilt

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Tilt")
	float StrafeTiltAngle = 2.2f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Tilt")
	float LookTiltAngle = 1.4f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Tilt")
	float LookTiltMaxTurnRate = 3.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Tilt")
	float TiltInterpSpeed = 9.0f;

	// Headbob

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Headbob")
	float BobFrequency = 9.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Headbob")
	float BobVerticalAmount = 3.4f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Headbob")
	float BobLateralAmount = 2.6f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Headbob")
	float BobRollAmount = 0.8f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Headbob")
	float BobBlendSpeed = 8.0f;

	// Landing

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Landing")
	float LandingMinImpactSpeed = 400.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Landing")
	float LandingMaxImpactSpeed = 1600.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Landing")
	float LandingDipImpulse = -260.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Landing")
	float LandingDipStiffness = 190.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Landing")
	float LandingDipDamping = 16.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Landing")
	float LandingFOVImpulse = 55.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Landing")
	float LandingFOVStiffness = 170.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|Landing")
	float LandingFOVDamping = 15.0f;

	// FOV

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|FOV")
	float BaseFOV = 95.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|FOV")
	float RunFOVOffset = 6.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|FOV")
	float FOVBlendSpeed = 7.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Camera|FOV")
	float CameraHeight = 68.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FPS|Character|Movement")
	EFPS_MovementState CurrentState = EFPS_MovementState::Idle;

private:
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleJumpPressed();
	void HandleRunStarted();
	void HandleRunCompleted();
	void HandleDashPressed();

	void UpdateGravity(float DeltaSeconds);
	void UpdateDash(float DeltaSeconds);
	void UpdateMovementState();
	void UpdateHeadbob(float DeltaSeconds);
	void UpdateTilt(float DeltaSeconds);
	void UpdateCameraTransform(float DeltaSeconds);
	void UpdateFOV(float DeltaSeconds);

	FVector2D MoveInput = FVector2D::ZeroVector;
	float LookTurnRate = 0.0f;
	bool bRunInputHeld = false;

	FVector CameraBaseLocation = FVector::ZeroVector;

	float BobTime = 0.0f;
	float BobBlend = 0.0f;
	FVector BobOffset = FVector::ZeroVector;
	float BobRoll = 0.0f;

	float CurrentTiltRoll = 0.0f;

	FFPS_Spring LandingDipSpring;
	FFPS_Spring LandingFOVSpring;
	float LastFallSpeed = 0.0f;

	FVector DashDirection = FVector::ZeroVector;
	float DashTimeRemaining = 0.0f;
	float DashCooldownRemaining = 0.0f;
	float DashFOVCurrent = 0.0f;
	float DashBankCurrent = 0.0f;
	float DashBankTarget = 0.0f;

	float SmoothedBaseFOV = 0.0f;

	TOptional<float> CameraZoomOverride;
};
