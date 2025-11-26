
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
	
	// Player functions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void RunPressed();
	void Dance();
	void RunReleased();

	// Jump system functions
	void QueueJumpInput();

	// Boolean states
	bool bIsRunning = false;
	bool bIsDancing = false;
	bool bIsJumping = false;
	bool bIsFlipping = false;
	bool bIsCrouching = false;
	bool bIsProning = false;
	bool bIsSliding = false;
	bool bIsRolling = false;
	
	// Input properties
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* InputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RunAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* DanceAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ProneAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RollAction;

	// Camera components and properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float SensitivityMultiplier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float VerticalSensitivityMultiplier;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraSpawnPitch;
	float InitialCameraSpawnPitch;

	// Movement properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float RotationSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float WalkSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float SprintSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float WalkableSlopeAngle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float StepOffset;

	// Jump General properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	float JumpForce = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	float FlipJumpForce = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	float CustomAirControl = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	float CustomGravityScale = 2.f;

	// Jump Extra properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	float JumpBufferTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	bool bAllowDoubleJump = true;

	UPROPERTY(BlueprintReadOnly, Category = "Jumping")
	bool bJumpInputQueued = false;

	UPROPERTY(BlueprintReadOnly, Category = "Jumping")
	bool bJumpPending = false;

	UPROPERTY(BlueprintReadOnly, Category = "Jumping")
	int32 JumpCount = 0;

	float JumpBufferTimer = 0.1f;

	// Crouch properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	float CrouchSpeed = 200.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	float CrouchCapsuleHalfHeight = 44.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	float StandCapsuleHalfHeight = 88.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	float CustomCapsuleCrouchOffset = -40.f;

	// Slide properties
	FVector SlideVelocity = FVector::ZeroVector;
	float SlideFallTimer = 0.f;
	float SlideStartTimer = 0.f;
	

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float GroundCheckDistance = 600.f;

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
	float RampBoostSpeed  = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float FlatSlideBoost = 300.f;

	//Prone properties

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prone")
	float ProneCapsuleHalfHeight = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prone")
	float CeilingCheckOffset = 5.f;

	UPROPERTY(BlueprintReadOnly, Category = "Prone")
	bool bIsInProneTransition = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prone")
	float ProneSpeed = 125.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prone")
	float CustomCapsuleProneOffset = -20.f;

	// Roll properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rolling")
	float WalkRollSpeed = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rolling")
	float SprintRollSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rolling")
	float RollDuration = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rolling")
	float GravityScaleDivider = 1.65f;
	
	float OriginalGravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rolling")
	float RollRotationBlend = 12.f;

	float RollTimer = 0.f;
	FVector RollDirection = FVector::ZeroVector;
	float CurrentRollSpeed = 0.f;

	// Ledge properties
	bool bIsOnLedge = false;
	bool bLedgeExitCooldown = false;
	FTimerHandle LedgeExitCooldownTimer;

	// Current ledge references
	AActor* CurrentLedge = nullptr;
	FVector LedgeForward;   // Axis along the ledge floor
	FVector LedgeInward;    // Axis into the wall
	FVector LedgeStartPos;  // Start of ledge floor
	FVector LedgeEndPos;    // End of ledge floor
	float   LedgeT = 0.f;   // Distance along ledge

	UPROPERTY(EditAnywhere, Category="Ledge")
	FName LedgeTag = "Ledge"; // ~half walk speed
	
	UPROPERTY(EditAnywhere, Category="Ledge")
	float LedgeSpeed = 100.f; 

	UPROPERTY(EditAnywhere, Category="Ledge")
	float WallHugOffset = 20.f; 

	UPROPERTY(EditAnywhere, Category="Ledge")
	float VerticalOffset = 105.f; // Mesh vertical offset from ledge floor

	UPROPERTY(EditAnywhere, Category="Ledge")
	float ForwardOffset = 0.f;

	UPROPERTY(EditAnywhere, Category="Ledge")
	float WallOffset = -5.f;  // Player distance offset from wall


	UPROPERTY(EditAnywhere, Category="Ledge")
	float LedgeExitCooldownTime = 0.5f; // Grace period

	bool bIsLedgeIdleLeft = false;
	bool bIsLedgeIdleRight = false;
	bool bIsLedgeWalkingLeft = false;
	bool bIsLedgeWalkingRight = false;
	bool bLastIdleLeft = false;


	// Overlap handlers
	UFUNCTION()
	void OnCapsuleOverlap(UPrimitiveComponent* OverlappedComp,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);

	UFUNCTION()
	void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComp,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex);
	
private:
	FVector2D MovementInput;

public:

	// Notify methods
	void StopMove(const FInputActionValue&);
	void ApplyJumpForce(); // Called when Jumping Animation leaves the ground
	void TriggerFlip(); // Called at the start of Flip Animation
	void EndFlip(); // Called at the end of Flip Animation
	void HandleCrouchReleased();
	void HandleCrouchOrSlidePressed();
	void ToggleProne();
	void TryStartSlide();
	void ExitSlide();
	void TryStartRoll();
	void UpdateRoll(float DeltaTime);
	void FinishRoll();
	float GetGroundDistance() const;
	void EnterLedge(AActor *LedgeActor);
	void TryEnterLedge(AActor *LedgeActor);
	void ExitLedge();
	void HandleLedgeMovement(float DeltaTime);
	void ResetLedgeExitCooldown();

	// Getters for movement input and states
	UFUNCTION(BlueprintCallable, Category="Movement")
	float GetForwardInput() const { return MovementInput.Y; }

	UFUNCTION(BlueprintCallable, Category="Movement")
	bool IsRunning() const { return bIsRunning; }

	UFUNCTION(BlueprintCallable, Category="Movement")
	bool IsDancing() const { return bIsDancing; }

	UFUNCTION(BlueprintCallable, Category="Jumping")
	bool IsJumping() const { return bIsJumping; }

	UFUNCTION(BlueprintCallable, Category="Jumping")
	bool IsFlipping() const { return bIsFlipping; }

	UFUNCTION(BlueprintCallable, Category="Sliding")
	bool IsSliding() const { return bIsSliding; }
	
	UFUNCTION(BlueprintCallable, Category="Crouching")
	bool IsPlayerCrouching() const { return bIsCrouching; }

	UFUNCTION(BlueprintCallable, Category="Proning")
	bool IsPlayerProning() const { return bIsProning; }

	UFUNCTION(BlueprintCallable, Category="Rolling")
	bool IsPlayerRolling() const { return bIsRolling; }

	UFUNCTION(BlueprintCallable, Category="Ledge")
	bool IsOnLedge() const { return bIsOnLedge; }

	UFUNCTION(BlueprintCallable, Category="Ledge")
	bool IsLedgeIdleLeft() const { return bIsLedgeIdleLeft; }
	
	UFUNCTION(BlueprintCallable, Category="Ledge")
	bool IsLedgeIdleRight() const { return bIsLedgeIdleRight; }
	
	UFUNCTION(BlueprintCallable, Category="Ledge")
	bool IsLedgeWalkingLeft() const { return bIsLedgeWalkingLeft; }
	
	UFUNCTION(BlueprintCallable, Category="Ledge")
	bool IsLedgeWalkingRight() const { return bIsLedgeWalkingRight; }

	UFUNCTION(BlueprintCallable, Category="Ledge")
	bool IsLedgeExitOnCooldown() const { return bLedgeExitCooldown; }

	UFUNCTION(BlueprintCallable, Category = "Crouch")
	bool CanStandUp() const;

	UFUNCTION(BlueprintCallable, Category = "Prone")
	bool CanCrouchUpFromProne() const;

	UFUNCTION(BlueprintCallable, Category = "Prone")
	void StartProneTransition();

	UFUNCTION(BlueprintCallable, Category = "Prone")
	void EndProneTransition();

	
};
