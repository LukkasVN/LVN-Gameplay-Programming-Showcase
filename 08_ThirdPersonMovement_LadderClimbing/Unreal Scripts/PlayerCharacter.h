#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class MECHANICS_TEST_LVN_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	// Input Actions
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* InputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RunAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ProneAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RollAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* GlideAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ClimbAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* DanceAction;

	// Camera public properties
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SensitivityMultiplier = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float VerticalSensitivityMultiplier = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraSpawnPitch = -20.f;

	float InitialCameraSpawnPitch;

	// Movement public properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkableSlopeAngle = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StepOffset = 30.f;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float GroundCheckDistance = 600.f;

	// Jumping public properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	float JumpForce = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	float FlipJumpForce = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	float CustomAirControl = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	float CustomGravityScale = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	float JumpBufferTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	bool bAllowDoubleJump = true;

	// Crouching public properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	float CrouchSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	float CrouchCapsuleHalfHeight = 44.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	float StandCapsuleHalfHeight = 88.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	float CustomCapsuleCrouchOffset = -40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	float CeilingCheckOffset = 5.f;

	// Prone public properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prone")
	float ProneSpeed = 125.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prone")
	float ProneCapsuleHalfHeight = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prone")
	float CustomCapsuleProneOffset = -20.f;

	// Sliding public properties
	UPROPERTY(EditAnywhere, Category = "Sliding")
	float MaxSlideSpeed = 3000.f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float SlideAirThreshold = 500.f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float SlideSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float MinSlideSpeed = 200.f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float SlideFriction = 2.25f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float SlideFallGraceTime = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float RampBoostSpeed = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float FlatSlideBoost = 300.f;

	// Rolling public properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rolling")
	float WalkRollSpeed = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rolling")
	float SprintRollSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rolling")
	float RollDuration = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rolling")
	float GravityScaleDivider = 1.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rolling")
	float RollRotationBlend = 12.f;

	// Gliding public properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gliding")
	float GlideGravityScale = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gliding")
	float GlideSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gliding")
	float GlideRotationSpeed = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gliding")
	float MinGlideActivationHeight = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gliding")
	float GlideFallVelocityThreshold = -200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gliding")
	float MaxGlideFallSpeed = -200.f;

	UPROPERTY(EditAnywhere, Category = "Gliding")
	float GlideYawOffset = 180.f;

	UPROPERTY(BlueprintReadWrite, Category = "Gliding")
	USkeletalMeshComponent* GliderMesh;

	// Ledge public properties
	UPROPERTY(EditAnywhere, Category = "Ledge")
	FName LedgeTag = "Ledge";

	UPROPERTY(EditAnywhere, Category = "Ledge")
	float LedgeSpeed = 100.f;

	UPROPERTY(EditAnywhere, Category = "Ledge")
	float VerticalOffset = 105.f;

	UPROPERTY(EditAnywhere, Category = "Ledge")
	float WallOffset = -5.f;

	UPROPERTY(EditAnywhere, Category = "Ledge")
	float LedgeExitCooldownTime = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	FName LadderTag = "Ladder";

	UPROPERTY(EditAnywhere, Category = "Ladder")
	FVector LadderPositionOffset = FVector(0.f,35.f,0.f);

	UPROPERTY(EditAnywhere, Category = "Ladder")
	float LadderClimbSpeed = 250.f;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	float LadderOffset = 35.f;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	float ClimbAnimationTime = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	float LadderRotationOffset = 180.f;

	bool bIsRunning = false;
	bool bIsDancing = false;
	bool bIsJumping = false;
	bool bIsFlipping = false;
	bool bIsCrouching = false;
	bool bIsProning = false;
	bool bIsSliding = false;
	bool bIsRolling = false;
	bool bIsGliding = false;
	bool bIsOnLedge = false;
	bool bIsLadderClimbing = false;
	bool bIsExitingLadder = false;
	bool bIsAtTopOfLadder = false;

private:
	FVector2D MovementInput;
	
	// Jump
	bool bJumpInputQueued = false;
	bool bJumpPending = false;
	int32 JumpCount = 0;
	float JumpBufferTimer = 0.1f;

	// Slide
	FVector SlideVelocity = FVector::ZeroVector;
	float SlideFallTimer = 0.f;
	float SlideStartTimer = 0.f;

	// Roll
	float RollTimer = 0.f;
	FVector RollDirection = FVector::ZeroVector;
	float CurrentRollSpeed = 0.f;
	float OriginalGravityScale = 1.0f;

	// Glide
	bool bGlideInputHeld = false;
	float OriginalGravityScaleBeforeGlide = 2.f;
	FRotator OriginalMeshRotation;

	// Ledge
	bool bLedgeExitCooldown = false;
	FTimerHandle LedgeExitCooldownTimer;
	AActor* CurrentLedge = nullptr;
	FVector LedgeForward;
	FVector LedgeInward;
	FVector LedgeStartPos;
	FVector LedgeEndPos;
	float LedgeT = 0.f;
	bool bLastIdleLeft = false;
	bool bIsLedgeIdleLeft = false;
	bool bIsLedgeIdleRight = false;
	bool bIsLedgeWalkingLeft = false;
	bool bIsLedgeWalkingRight = false;

	// Ladder
	AActor* CurrentLadder = nullptr;
	AActor* LadderCandidate = nullptr;
	FVector LadderFaceDir;
	FTimerHandle LadderExitTimer;
	float startingLadderZ = 0.f;
	FVector LadderAttachBase = FVector::ZeroVector;

	// Exit lerp data
	FVector ExitStartLocation = FVector::ZeroVector;
	FVector ExitTargetLocation = FVector::ZeroVector;
	float ExitStartTime = 0.f;
	FVector MeshInitialPosition;
	FRotator MeshInitialRotation;

	// Prone transition
	bool bIsInProneTransition = false;

	// Input Handlers
	void Move(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void RunPressed();
	void RunReleased();
	void Dance();
	void QueueJumpInput();
	void HandleCrouchOrSlidePressed();
	void HandleCrouchReleased();
	void ToggleProne();
	void GlidePressed();
	void GlideReleased();
	void ClimbPressed();

	// Movement Systems
	void HandleMovement(float DeltaTime);
	void HandleJumping(float DeltaTime);
	void HandleSliding(float DeltaTime);
	void HandleRolling(float DeltaTime);
	void HandleGliding(float DeltaTime);
	void HandleLedgeMovement(float DeltaTime);
	void HandleLadderClimbing(float DeltaTime);
	void UpdateExitLerp(float DeltaTime);

	// Sliding methods
	void TryStartSlide();
	void ExitSlide();
	float GetGroundDistance() const;

	// Rolling methods
	void TryStartRoll();
	void UpdateRoll(float DeltaTime);
	void FinishRoll();

	// Ledge methods
	void TryEnterLedge(AActor* LedgeActor);
	void EnterLedge(AActor* LedgeActor);
	void ExitLedge();
	void ResetLedgeExitCooldown();

	// Ladder methods
	void TryStartLadderClimb(AActor* LadderActor);
	void ExitLadderAtBottom();
	void StartLadderTopExit();
	void OnLadderExitAnimationComplete();
	void FinishLadderExitCleanup();
	void ReattachMeshAfterLadderClimb();

	// Utility methods
	bool CanStandUp() const;
	bool CanCrouchUpFromProne() const;
	bool IsGrounded() const;

	// Overlap handlers
	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Animation Callbacks
public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void ApplyJumpForce();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void TriggerFlip();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void EndFlip();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void StartProneTransition() { bIsInProneTransition = true; }

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void EndProneTransition() { bIsInProneTransition = false; }

	// Getters
	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsRunning() const { return bIsRunning; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsDancing() const { return bIsDancing; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsJumping() const { return bIsJumping; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsFlipping() const { return bIsFlipping; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsPlayerCrouching() const { return bIsCrouching; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsPlayerProning() const { return bIsProning; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsSliding() const { return bIsSliding; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsPlayerRolling() const { return bIsRolling; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsGliding() const { return bIsGliding; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsOnLedge() const { return bIsOnLedge; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsLadderClimbing() const { return bIsLadderClimbing; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsExitingLadder() const { return bIsExitingLadder; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsLedgeIdleLeft() const { return bIsLedgeIdleLeft; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsLedgeIdleRight() const { return bIsLedgeIdleRight; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsLedgeWalkingLeft() const { return bIsLedgeWalkingLeft; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsLedgeWalkingRight() const { return bIsLedgeWalkingRight; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsInProneTransition() const { return bIsInProneTransition; }

	UFUNCTION(BlueprintCallable, Category = "State")
	float GetForwardInput() const { return MovementInput.Y; }
};