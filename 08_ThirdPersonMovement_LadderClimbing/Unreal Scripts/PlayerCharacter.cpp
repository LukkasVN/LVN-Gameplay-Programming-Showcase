#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

	APlayerCharacter::APlayerCharacter()
	{
		PrimaryActorTick.bCanEverTick = true;

		// Camera setup
		CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
		CameraBoom->SetupAttachment(GetRootComponent());
		CameraBoom->TargetArmLength = 375.f;
		CameraBoom->bUsePawnControlRotation = true;
		CameraBoom->bEnableCameraLag = true;
		CameraBoom->CameraLagSpeed = 10.f;
		CameraBoom->bEnableCameraRotationLag = true;
		CameraBoom->CameraRotationLagSpeed = 10.f;

		FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
		FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
		FollowCamera->FieldOfView = 110.f;
		FollowCamera->bUsePawnControlRotation = false;

		// Character rotation
		bUseControllerRotationYaw = false;
		bUseControllerRotationPitch = false;
		bUseControllerRotationRoll = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;

		// Movement defaults
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		GetCharacterMovement()->AirControl = CustomAirControl;
		GetCharacterMovement()->GravityScale = CustomGravityScale;

		
	}

	void APlayerCharacter::BeginPlay()
	{
		Super::BeginPlay();

		// Overlap events
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnCapsuleBeginOverlap);
		GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnCapsuleEndOverlap);

		MeshInitialPosition = GetMesh()->GetRelativeLocation();
		MeshInitialRotation = GetMesh()->GetRelativeRotation();
		
		InitialCameraSpawnPitch = CameraSpawnPitch;

		// Movement component settings
		GetCharacterMovement()->SetWalkableFloorAngle(WalkableSlopeAngle);
		GetCharacterMovement()->MaxStepHeight = StepOffset;
		GetCharacterMovement()->PerchRadiusThreshold = 10.f;
		GetCharacterMovement()->BrakingDecelerationWalking = 1800.f;
		GetCharacterMovement()->GroundFriction = 8.f;

		CameraBoom->SetRelativeRotation(FRotator(CameraSpawnPitch, 0.f, 0.f));

		// Setup Enhanced Input
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(InputMapping, 0);
			}
		}

		// Find glider mesh
		TArray<USkeletalMeshComponent*> SkeletalMeshes;
		GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
		for (USkeletalMeshComponent* CurrentMesh : SkeletalMeshes)
		{
			if (CurrentMesh->GetName().Contains("Glider"))
			{
				GliderMesh = CurrentMesh;
				GliderMesh->SetVisibility(false);
				GliderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				break;
			}
		}
	}

	void APlayerCharacter::Tick(float DeltaTime)
	{
		Super::Tick(DeltaTime);

		// Priority system
		if (bIsOnLedge)
		{
			HandleLedgeMovement(DeltaTime);
			return;
		}

		if (bIsExitingLadder)
		{
			UpdateExitLerp(DeltaTime);
			return; 
		}

		if (bIsLadderClimbing)
		{
			HandleLadderClimbing(DeltaTime);
			return;
		}

		if (bIsRolling)
		{
			HandleRolling(DeltaTime);
			return;
		}

		if (bIsGliding)
		{
			HandleGliding(DeltaTime);
			return;
		}

		// Standard movement
		HandleJumping(DeltaTime);
		HandleSliding(DeltaTime);
	}

	// ========== INPUT HANDLERS ==========

	void APlayerCharacter::Move(const FInputActionValue& Value)
	{
		FVector2D Input = Value.Get<FVector2D>();
		MovementInput = Input;

		if (!Controller || bIsInProneTransition || bIsRolling || bIsOnLedge || bIsLadderClimbing || bIsExitingLadder)
			return;

		if (bIsDancing)
		{
			if (!Input.IsNearlyZero())
				bIsDancing = false;
			else
				return;
		}

		FRotator CameraRot = FollowCamera->GetComponentRotation();
		CameraRot.Pitch = 0.f;
		CameraRot.Roll = 0.f;

		const FVector Forward = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Y);

		FVector MoveInput = Forward * Input.Y + Right * Input.X;

		if (MoveInput.IsNearlyZero())
			return;

		AddMovementInput(MoveInput.GetSafeNormal(), 1.f);

		// Rotate character
		FRotator DesiredRot = MoveInput.Rotation();
		if (Input.Y < 0.f)
			DesiredRot.Yaw += 180.f;

		FRotator Current = GetActorRotation();
		FRotator TargetYawOnly(0.f, DesiredRot.Yaw, 0.f);
		FRotator NewRot = FMath::RInterpTo(Current, TargetYawOnly, GetWorld()->GetDeltaSeconds(), RotationSpeed);
		SetActorRotation(NewRot);

		// Set speed based on state
		if (bIsSliding)
			return;

		if (bIsProning)
			GetCharacterMovement()->MaxWalkSpeed = ProneSpeed;
		else if (bIsCrouching)
			GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
		else if (bIsRunning && Input.Y >= 0.f)
			GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		else
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	void APlayerCharacter::StopMove(const FInputActionValue& Value)
	{
		MovementInput = FVector2D::ZeroVector;
	}

	void APlayerCharacter::Look(const FInputActionValue& Value)
	{
		const FVector2D LookInput = Value.Get<FVector2D>();
		if (!LookInput.IsNearlyZero())
		{
			AddControllerYawInput(LookInput.X * SensitivityMultiplier);
			AddControllerPitchInput(LookInput.Y * VerticalSensitivityMultiplier);

			FRotator ControlRot = GetControlRotation();
			ControlRot.Pitch = FMath::ClampAngle(ControlRot.Pitch, InitialCameraSpawnPitch - 15.f, InitialCameraSpawnPitch + 45.f);
			GetController()->SetControlRotation(ControlRot);
		}
	}

	void APlayerCharacter::RunPressed()
	{
		bIsRunning = true;
		if (MovementInput.Y >= 0.f && !bIsSliding && !bIsCrouching && !bIsProning)
			GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}

	void APlayerCharacter::RunReleased()
	{
		bIsRunning = false;
		if (!bIsSliding && !bIsCrouching && !bIsProning)
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	void APlayerCharacter::Dance()
	{
		if (IsGrounded() && MovementInput.IsNearlyZero())
			bIsDancing = true;
	}

	void APlayerCharacter::QueueJumpInput()
	{
		if (bIsDancing || bIsCrouching || bIsProning || bIsOnLedge || bIsLadderClimbing)
			return;

		bJumpInputQueued = true;
		JumpBufferTimer = JumpBufferTime;
	}

	void APlayerCharacter::HandleCrouchOrSlidePressed()
	{
		if (bIsProning || bIsDancing || bIsRolling || bIsJumping || bIsFlipping || bIsInProneTransition || bIsOnLedge || bIsLadderClimbing)
			return;

		float GroundDistance = GetGroundDistance();
		const bool bNearGround = GroundDistance <= SlideAirThreshold;
		const bool bCanSlide = !bIsSliding && bIsRunning && MovementInput.Size() > 0.1f;

		if (bCanSlide && (IsGrounded() || bNearGround))
		{
			TryStartSlide();
		}
		else if (IsGrounded())
		{
			// Toggle crouch
			if (bIsCrouching)
			{
				if (CanStandUp())
				{
					GetCapsuleComponent()->SetCapsuleHalfHeight(StandCapsuleHalfHeight, true);
					GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -StandCapsuleHalfHeight));
					bIsCrouching = false;
					GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
				}
			}
			else
			{
				GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchCapsuleHalfHeight, true);
				GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -CrouchCapsuleHalfHeight));
				bIsCrouching = true;
				GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
			}
		}
	}

	void APlayerCharacter::HandleCrouchReleased()
	{
		if (bIsSliding)
			ExitSlide();
	}

	void APlayerCharacter::ToggleProne()
	{
		if (bIsInProneTransition || bIsRolling || bIsDancing || bIsRunning || bIsJumping || bIsFlipping || !IsGrounded())
			return;

		UCapsuleComponent* Capsule = GetCapsuleComponent();
		UMeshComponent* PlayerMesh = GetMesh();

		if (bIsProning)
		{
			if (CanCrouchUpFromProne())
			{
				float OldHeight = Capsule->GetUnscaledCapsuleHalfHeight();
				float NewHeight = CrouchCapsuleHalfHeight;
				float Delta = NewHeight - OldHeight;

				FVector UpOffset(0.f, 0.f, 2.0f);
				FHitResult Hit;
				AddActorWorldOffset(UpOffset, true, &Hit);

				FVector DownOffset(0.f, 0.f, -Delta * 0.95f + CustomCapsuleCrouchOffset);
				AddActorWorldOffset(DownOffset, true, &Hit);

				Capsule->SetCapsuleHalfHeight(NewHeight, true);
				PlayerMesh->SetRelativeLocation(FVector(0.f, 0.f, -NewHeight));

				bIsProning = false;
				bIsCrouching = true;
				bIsInProneTransition = true;
				GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
			}
		}
		else if (bIsCrouching)
		{
			float OldHeight = Capsule->GetUnscaledCapsuleHalfHeight();
			float NewHeight = ProneCapsuleHalfHeight;
			float Delta = OldHeight - NewHeight;

			FVector DownOffset(0.f, 0.f, Delta * 0.95f + CustomCapsuleProneOffset);
			FHitResult Hit;
			AddActorWorldOffset(DownOffset, true, &Hit);

			Capsule->SetCapsuleHalfHeight(NewHeight, true);
			PlayerMesh->SetRelativeLocation(FVector(0.f, 0.f, -NewHeight));

			bIsProning = true;
			bIsInProneTransition = true;
			GetCharacterMovement()->MaxWalkSpeed = ProneSpeed;
		}
	}

	void APlayerCharacter::ClimbPressed()
	{
		if (LadderCandidate != nullptr && !bIsLadderClimbing)
		{
			TryStartLadderClimb(LadderCandidate);
		}
	}

	// ========== JUMPING ==========

	void APlayerCharacter::HandleJumping(float DeltaTime)
	{
		const bool bGrounded = IsGrounded();

		// Update jump buffer
		if (bJumpInputQueued)
		{
			JumpBufferTimer -= DeltaTime;
			if (JumpBufferTimer <= 0.f)
				bJumpInputQueued = false;
		}

		if (bGrounded)
		{
			JumpCount = 0;
			bJumpPending = false;
			bIsFlipping = false;

			// Buffered jump
			if (bJumpInputQueued && !bJumpPending && !bIsCrouching && !bIsProning)
			{
				bJumpPending = true;
				bJumpInputQueued = false;
				JumpCount++;
				bIsJumping = true;
			}
		}
		else
		{
			// Double jump
			if (bAllowDoubleJump && JumpCount < 2 && bJumpInputQueued && !bJumpPending && !bIsCrouching && !bIsProning)
			{
				bIsFlipping = true;
				bIsJumping = true;
				bJumpPending = true;
				bJumpInputQueued = false;
				JumpCount++;
			}

			// Reset jumping flag when falling
			if (bIsJumping && GetCharacterMovement()->IsFalling())
				bIsJumping = false;
		}
	}

	void APlayerCharacter::ApplyJumpForce()
	{
		if (bIsCrouching)
			return;

		LaunchCharacter(FVector(0.f, 0.f, JumpForce), false, true);
		bJumpPending = false;
	}

	void APlayerCharacter::TriggerFlip()
	{
		if (bIsCrouching)
			return;

		bIsFlipping = true;
		bIsJumping = true;
		bJumpPending = true;
		JumpCount++;
		LaunchCharacter(FVector(0.f, 0.f, FlipJumpForce), false, true);
	}

	void APlayerCharacter::EndFlip()
	{
		bIsFlipping = false;
		bIsJumping = false;
	}

	// ========== SLIDING ==========

	void APlayerCharacter::HandleSliding(float DeltaTime)
	{
		if (!bIsSliding)
			return;

		FVector GroundNormal = FVector::UpVector;
		FHitResult Hit;
		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0.f, 0.f, 150.f);

		bool bIsDownhillAligned = false;
		float SlideExitSpeedThreshold = MinSlideSpeed;

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			GroundNormal = Hit.Normal;
			float Incline = FVector::DotProduct(GroundNormal, FVector::UpVector);
			float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(Incline));

			FVector DownhillDir = FVector::CrossProduct(GroundNormal, FVector::CrossProduct(FVector::UpVector, GroundNormal)).GetSafeNormal();
			float Alignment = FVector::DotProduct(SlideVelocity.GetSafeNormal(), DownhillDir);

			// Downhill boost
			if (SlopeAngle > 10.f && Alignment < 0.5f)
			{
				float BoostScale = FMath::Clamp((1.f - Alignment) * (SlopeAngle / 30.f), 0.5f, 2.5f);
				SlideVelocity += DownhillDir * RampBoostSpeed * DeltaTime * BoostScale;
				bIsDownhillAligned = true;
			}

			// Uphill penalty
			if (Alignment > 0.f)
			{
				float UphillPenalty = FMath::Clamp(Alignment, 0.2f, 1.f);
				SlideVelocity *= 1.f - UphillPenalty * 0.5f;
			}

			SlideExitSpeedThreshold = bIsDownhillAligned ? MinSlideSpeed * 0.5f : MinSlideSpeed;

			// Flat boost
			float FlatBoostScale = FMath::Clamp(1.f - Alignment, 0.f, 1.f);
			FVector TargetFlatBoost = GetActorForwardVector() * FlatSlideBoost * FlatBoostScale;
			SlideVelocity = FMath::VInterpTo(SlideVelocity, TargetFlatBoost, DeltaTime, 3.0f);
		}

		SlideVelocity = SlideVelocity.GetClampedToMaxSize(MaxSlideSpeed);
		GetCharacterMovement()->MaxWalkSpeed = MaxSlideSpeed;
		AddMovementInput(SlideVelocity.GetSafeNormal(), SlideVelocity.Size() * DeltaTime);

		// Track airborne time
		if (!IsGrounded())
			SlideFallTimer += DeltaTime;
		else
			SlideFallTimer = 0.f;

		// Grace period timer
		if (SlideStartTimer > 0.f)
			SlideStartTimer -= DeltaTime;

		// Exit conditions
		bool bShouldExitSlide = SlideVelocity.Size() < SlideExitSpeedThreshold && !bIsDownhillAligned;
		if (SlideStartTimer <= 0.f && (bShouldExitSlide || SlideFallTimer > SlideFallGraceTime || !bIsRunning))
		{
			ExitSlide();
			return;
		}

		SlideVelocity = FMath::VInterpTo(SlideVelocity, FVector::ZeroVector, DeltaTime, SlideFriction);
	}

	void APlayerCharacter::TryStartSlide()
	{
		if (bIsSliding || bIsRolling || bIsCrouching || bIsProning || !bIsRunning || MovementInput.Size() < 0.1f)
			return;

		float GroundDistance = GetGroundDistance();
		if (!IsGrounded() && GroundDistance > SlideAirThreshold)
			return;

		bIsSliding = true;
		SlideStartTimer = 0.2f;
		SlideVelocity = GetActorForwardVector() * SlideSpeed;

		GetCapsuleComponent()->SetCapsuleHalfHeight(ProneCapsuleHalfHeight, true);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -ProneCapsuleHalfHeight));
	}

	void APlayerCharacter::ExitSlide()
	{
		bIsSliding = false;

		const bool bCanStand = CanStandUp();
		const bool bCanCrouch = CanCrouchUpFromProne();

		if (bCanStand && bCanCrouch)
		{
			GetCapsuleComponent()->SetCapsuleHalfHeight(StandCapsuleHalfHeight, true);
			GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -StandCapsuleHalfHeight));
			bIsCrouching = false;
			bIsProning = false;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}
		else if (bCanCrouch)
		{
			GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchCapsuleHalfHeight, true);
			GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -CrouchCapsuleHalfHeight));
			bIsCrouching = true;
			bIsProning = false;
			GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
		}
		else
		{
			GetCapsuleComponent()->SetCapsuleHalfHeight(ProneCapsuleHalfHeight, true);
			GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -ProneCapsuleHalfHeight));
			bIsCrouching = true;
			bIsProning = true;
			GetCharacterMovement()->MaxWalkSpeed = ProneSpeed;
		}
	}

	// ========== ROLLING ==========

	void APlayerCharacter::TryStartRoll()
	{
		if (!IsGrounded() || bIsSliding || bIsCrouching || bIsProning || bIsOnLedge || bIsLadderClimbing)
			return;

		FRotator CameraRot = FollowCamera->GetComponentRotation();
		CameraRot.Pitch = 0.f;
		CameraRot.Roll = 0.f;

		const FVector Forward = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Y);
		FVector MoveInput = Forward * MovementInput.Y + Right * MovementInput.X;

		if (MoveInput.IsNearlyZero())
			MoveInput = Forward;

		RollDirection = MoveInput.GetSafeNormal();
		CurrentRollSpeed = bIsRunning ? SprintRollSpeed : WalkRollSpeed;
		GetCharacterMovement()->MaxWalkSpeed = CurrentRollSpeed;

		bIsRolling = true;
		RollTimer = 0.f;

		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;

		FRotator DesiredRot = RollDirection.Rotation();
		SetActorRotation(FRotator(0.f, DesiredRot.Yaw, 0.f));

		GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchCapsuleHalfHeight, true);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -CrouchCapsuleHalfHeight));

		OriginalGravityScale = GetCharacterMovement()->GravityScale;
		GetCharacterMovement()->GravityScale /= GravityScaleDivider;
	}

	void APlayerCharacter::HandleRolling(float DeltaTime)
	{
		UpdateRoll(DeltaTime);
	}

	void APlayerCharacter::UpdateRoll(float DeltaTime)
	{
		if (bIsJumping)
		{
			FinishRoll();
			return;
		}

		RollTimer += DeltaTime;

		float RollAlpha = RollTimer / RollDuration;
		float SpeedFactor = FMath::Sin(RollAlpha * PI);

		FVector FlatDir = FVector(RollDirection.X, RollDirection.Y, 0.f).GetSafeNormal();
		AddMovementInput(FlatDir, CurrentRollSpeed * SpeedFactor * DeltaTime);

		FRotator DesiredRot = RollDirection.Rotation();
		FRotator Current = GetActorRotation();
		FRotator TargetYawOnly(0.f, DesiredRot.Yaw, 0.f);
		FRotator NewRot = FMath::RInterpTo(Current, TargetYawOnly, DeltaTime, RollRotationBlend);
		SetActorRotation(NewRot);

		if (RollTimer >= RollDuration)
			FinishRoll();
	}

	void APlayerCharacter::FinishRoll()
	{
		bIsRolling = false;

		if (CanStandUp())
		{
			GetCapsuleComponent()->SetCapsuleHalfHeight(StandCapsuleHalfHeight, true);
			GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -StandCapsuleHalfHeight));
			bIsCrouching = false;
		}
		else
		{
			GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchCapsuleHalfHeight, true);
			GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -CrouchCapsuleHalfHeight));
			bIsCrouching = true;
		}

		GetCharacterMovement()->GravityScale = OriginalGravityScale;
	}

	// ========== GLIDING ==========

	void APlayerCharacter::GlidePressed()
	{
		if (bIsOnLedge || bIsRolling || bIsSliding || bIsCrouching || bIsProning || bIsDancing || bIsLadderClimbing)
			return;

		if (!GetCharacterMovement()->IsFalling())
			return;

		float CurrentFallSpeed = GetCharacterMovement()->Velocity.Z;
		if (CurrentFallSpeed > GlideFallVelocityThreshold)
			return;

		float GroundDistance = GetGroundDistance();
		if (GroundDistance < MinGlideActivationHeight)
			return;

		bGlideInputHeld = true;

		if (!bIsGliding)
		{
			bIsGliding = true;
			OriginalMeshRotation = GetMesh()->GetRelativeRotation();
			OriginalGravityScaleBeforeGlide = GetCharacterMovement()->GravityScale;
			GetCharacterMovement()->GravityScale = GlideGravityScale;
			GetCharacterMovement()->MaxWalkSpeed = GlideSpeed;
			GetCharacterMovement()->bOrientRotationToMovement = false;
			bUseControllerRotationYaw = false;

			FRotator MeshRot = OriginalMeshRotation;
			MeshRot.Yaw += GlideYawOffset;
			GetMesh()->SetRelativeRotation(MeshRot);

			if (GliderMesh)
				GliderMesh->SetVisibility(true);
		}
	}

	void APlayerCharacter::HandleGliding(float DeltaTime)
	{
		if (!bIsGliding)
			return;

		// Auto-exit when grounded
		if (IsGrounded())
		{
			GlideReleased();
			return;
		}

		// Exit if released or too close to ground
		float GroundDistance = GetGroundDistance();
		if (!bGlideInputHeld || GroundDistance < MinGlideActivationHeight * 0.3f)
		{
			GlideReleased();
			return;
		}

		// Camera-relative movement
		FRotator CameraRot = FollowCamera->GetComponentRotation();
		CameraRot.Pitch = 0.f;
		CameraRot.Roll = 0.f;

		const FVector Forward = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Y);
		FVector MoveInput = Forward * MovementInput.Y + Right * MovementInput.X;

		if (!MoveInput.IsNearlyZero())
		{
			FVector GlideDirection = MoveInput.GetSafeNormal();
			AddMovementInput(GlideDirection, GlideSpeed * DeltaTime);

			FRotator DesiredRot = GlideDirection.Rotation();
			FRotator Current = GetActorRotation();
			FRotator TargetYawOnly(0.f, DesiredRot.Yaw, 0.f);
			FRotator NewRot = FMath::RInterpTo(Current, TargetYawOnly, DeltaTime, GlideRotationSpeed);
			SetActorRotation(NewRot);
		}

		// Always maintain mesh rotation offset
		FRotator MeshRot = OriginalMeshRotation;
		MeshRot.Yaw += GlideYawOffset;
		GetMesh()->SetRelativeRotation(MeshRot);

		// Clamp fall speed
		FVector Velocity = GetCharacterMovement()->Velocity;
		if (Velocity.Z < MaxGlideFallSpeed)
		{
			Velocity.Z = MaxGlideFallSpeed;
			GetCharacterMovement()->Velocity = Velocity;
		}
	}

	void APlayerCharacter::GlideReleased()
	{
		bGlideInputHeld = false;

		if (bIsGliding)
		{
			bIsGliding = false;
			GetMesh()->SetRelativeRotation(OriginalMeshRotation);
			GetCharacterMovement()->GravityScale = OriginalGravityScaleBeforeGlide;
			GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? SprintSpeed : WalkSpeed;
			GetCharacterMovement()->bOrientRotationToMovement = false;
			bUseControllerRotationYaw = false;

			if (GliderMesh)
				GliderMesh->SetVisibility(false);
		}
	}

	// ========== LEDGE MOVEMENT ==========

	void APlayerCharacter::TryEnterLedge(AActor* LedgeActor)
	{
		if (bIsOnLedge || bLedgeExitCooldown)
			return;
		if (bIsRolling || bIsSliding || bIsCrouching || bIsProning || bIsFlipping || bIsDancing)
			return;
		if (!IsGrounded())
			return;

		EnterLedge(LedgeActor);
	}

	void APlayerCharacter::EnterLedge(AActor* LedgeActor)
	{
		bIsOnLedge = true;
		CurrentLedge = LedgeActor;

		GetCharacterMovement()->DisableMovement();

		UStaticMeshComponent* FloorComp = LedgeActor->FindComponentByClass<UStaticMeshComponent>();
		USceneComponent* StartComp = nullptr;
		USceneComponent* EndComp = nullptr;

		for (UActorComponent* Comp : LedgeActor->GetComponents())
		{
			if (Comp->GetName().Contains("LedgeStart"))
				StartComp = Cast<USceneComponent>(Comp);
			else if (Comp->GetName().Contains("LedgeEnd"))
				EndComp = Cast<USceneComponent>(Comp);
		}

		if (StartComp && EndComp)
		{
			LedgeStartPos = StartComp->GetComponentLocation();
			LedgeEndPos = EndComp->GetComponentLocation();
		}
		else if (FloorComp)
		{
			FBoxSphereBounds Bounds = FloorComp->CalcBounds(FloorComp->GetComponentTransform());
			FVector Extent = Bounds.BoxExtent;
			LedgeForward = FloorComp->GetForwardVector().GetSafeNormal();
			LedgeStartPos = Bounds.Origin - LedgeForward * Extent.Size();
			LedgeEndPos = Bounds.Origin + LedgeForward * Extent.Size();
		}

		LedgeForward = (LedgeEndPos - LedgeStartPos).GetSafeNormal();
		LedgeInward = -FloorComp->GetRightVector();

		FVector ToChar = GetActorLocation() - LedgeStartPos;
		LedgeT = FVector::DotProduct(ToChar, LedgeForward);

		FRotator FaceWallRot(0.f, LedgeInward.Rotation().Yaw, 0.f);
		SetActorRotation(FaceWallRot);

		bLastIdleLeft = FVector::DotProduct(GetActorRightVector(), LedgeForward) < 0.f;
		bIsLedgeIdleLeft = bLastIdleLeft;
		bIsLedgeIdleRight = !bLastIdleLeft;
	}

	void APlayerCharacter::HandleLedgeMovement(float DeltaTime)
	{
		if (!bIsOnLedge || !CurrentLedge)
			return;

		const float X = -MovementInput.X;
		float LedgeLength = (LedgeEndPos - LedgeStartPos).Size();

		float NewT = LedgeT + X * LedgeSpeed * DeltaTime;

		// Exit if trying to go beyond limits
		if ((X < 0.f && NewT < 0.f) || (X > 0.f && NewT > LedgeLength))
		{
			ExitLedge();
			return;
		}

		LedgeT = FMath::Clamp(NewT, 0.f, LedgeLength);

		FVector DesiredPos = LedgeStartPos + LedgeForward * LedgeT + LedgeInward * WallOffset + FVector(0.f, 0.f, VerticalOffset);
		SetActorLocation(DesiredPos, false);

		// Animation flags
		bIsLedgeIdleLeft = bIsLedgeIdleRight = bIsLedgeWalkingLeft = bIsLedgeWalkingRight = false;
		if (FMath::Abs(X) < 0.1f)
		{
			bIsLedgeIdleRight = bLastIdleLeft;
			bIsLedgeIdleLeft = !bLastIdleLeft;
		}
		else if (X < -0.1f)
		{
			bLastIdleLeft = true;
			bIsLedgeWalkingRight = true;
		}
		else if (X > 0.1f)
		{
			bLastIdleLeft = false;
			bIsLedgeWalkingLeft = true;
		}
	}

	void APlayerCharacter::ExitLedge()
	{
		bIsOnLedge = false;
		CurrentLedge = nullptr;

		GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		bIsLedgeIdleLeft = bIsLedgeIdleRight = bIsLedgeWalkingLeft = bIsLedgeWalkingRight = false;

		bLedgeExitCooldown = true;
		GetWorldTimerManager().SetTimer(LedgeExitCooldownTimer, this, &APlayerCharacter::ResetLedgeExitCooldown, LedgeExitCooldownTime, false);
	}

	void APlayerCharacter::ResetLedgeExitCooldown()
	{
		bLedgeExitCooldown = false;
	}

	// ========== LADDER CLIMBING ==========

	void APlayerCharacter::TryStartLadderClimb(AActor* LadderActor)
	{
		if (!LadderActor || bIsLadderClimbing || bIsExitingLadder || bIsJumping || bIsCrouching || bIsProning || bIsDancing || bIsGliding || bIsRolling || bIsOnLedge || bIsSliding) return;

		bIsLadderClimbing = true;
		bIsAtTopOfLadder = false;
		CurrentLadder = LadderActor;

		// Ladder center
		UPrimitiveComponent* LadderCollider = LadderActor->FindComponentByClass<UPrimitiveComponent>();
		const FVector LadderCenter = LadderCollider ? LadderCollider->Bounds.Origin : LadderActor->GetActorLocation();

		// Which side to face
		const FVector ToPlayer = (GetActorLocation() - LadderCenter).GetSafeNormal();
		const FVector LadderForward = LadderActor->GetActorForwardVector();
		const float Dot = FVector::DotProduct(ToPlayer, LadderForward);
		LadderFaceDir = (Dot >= 0.f) ? LadderForward : -LadderForward;

		// Rotate LadderPositionOffset into world space
		const FVector WorldOffset = LadderActor->GetActorRotation().RotateVector(LadderPositionOffset);

		LadderAttachBase = LadderCenter + LadderFaceDir * LadderOffset + WorldOffset;
		LadderAttachBase.Z = GetActorLocation().Z;

		// Place player on attach base XY immediately
		FVector StartPos = GetActorLocation();
		StartPos.X = LadderAttachBase.X;
		StartPos.Y = LadderAttachBase.Y;
		SetActorLocation(StartPos);
		FRotator FaceLadderRot = LadderFaceDir.Rotation();
		SetActorRotation(FRotator(0.f, FaceLadderRot.Yaw + LadderRotationOffset, 0.f));

		// Movement setup for climb
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	}

	// Per-frame climb updating Z while keeping XY centered
	void APlayerCharacter::HandleLadderClimbing(float DeltaTime)
	{
		if (!bIsLadderClimbing) return;

		// Bottom exit detection based on Input
		const bool bGroundedOnLadder = GetGroundDistance() < 5.f;
		if (bGroundedOnLadder && MovementInput.Y < -0.1f)
		{
			ExitLadderAtBottom();
			return;
		}

		// Vertical input drives Z velocity [I have the locomotion configured to also match the animation speed and direction]
		const float VerticalInput = MovementInput.Y;
		if (FMath::Abs(VerticalInput) > 0.1f)
		{
			GetCharacterMovement()->Velocity = FVector(0.f, 0.f, VerticalInput * LadderClimbSpeed);
		}
		else
		{
			GetCharacterMovement()->Velocity = FVector::ZeroVector;
		}
		
		FVector Current = GetActorLocation();
		Current.X = FMath::FInterpTo(Current.X, LadderAttachBase.X, DeltaTime, 10.f);
		Current.Y = FMath::FInterpTo(Current.Y, LadderAttachBase.Y, DeltaTime, 10.f);
		SetActorLocation(Current);

	}


	// Bottom exit: snap to walk and restore mesh/collision
	void APlayerCharacter::ExitLadderAtBottom()
	{
		bIsLadderClimbing = false;
		bIsAtTopOfLadder = false;
		CurrentLadder = nullptr;

		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCharacterMovement()->StopMovementImmediately();

		// Ensure collision on
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// Start top exit
	void APlayerCharacter::StartLadderTopExit()
	{
		if (!bIsLadderClimbing || !CurrentLadder) return;

		bIsExitingLadder = true;
		bIsLadderClimbing = false;

		// Find exit point once [Must set empty Actor inside the ladder Blueprint]
		USceneComponent* ExitPoint = nullptr;
		for (UActorComponent* Comp : CurrentLadder->GetComponents())
		{
			if (Comp->GetName().Contains("ExitPoint"))
			{
				ExitPoint = Cast<USceneComponent>(Comp);
				break;
			}
		}

		// Get ladder forward vector
		const FVector LadderForward = CurrentLadder->GetActorForwardVector();
		
		const FVector OffsetDir = -LadderForward;

		// Apply small offset if needed
		const FVector Offset = OffsetDir * (LadderOffset * 0.75f);

		SetActorLocation(GetActorLocation() + Offset);
		
		ExitStartLocation = GetActorLocation();
		ExitTargetLocation = ExitPoint ? ExitPoint->GetComponentLocation() : ExitStartLocation;

		// Freeze movement and collision during transition 
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform); // Due to my climb animation being not rooted properly
		
		// Timer for animation completion (positions are lerped during Tick)
		GetWorldTimerManager().SetTimer(LadderExitTimer,this,&APlayerCharacter::OnLadderExitAnimationComplete,ClimbAnimationTime,false);
	}

	void APlayerCharacter::UpdateExitLerp(float DeltaTime)
	{
		if (!bIsExitingLadder) return;
		
		static float Accumulated = 0.f;
		Accumulated = FMath::Clamp(Accumulated + DeltaTime, 0.f, ClimbAnimationTime);
		const float Alpha = ClimbAnimationTime > 0.f ? (Accumulated / ClimbAnimationTime) : 1.f;

		const FVector NewLoc = FMath::Lerp(ExitStartLocation, ExitTargetLocation, Alpha);
		SetActorLocation(NewLoc);

		if (Alpha >= 1.f)
		{
			SetActorLocation(ExitTargetLocation);
		}
	}

	void APlayerCharacter::OnLadderExitAnimationComplete()
	{
		// Ensure final location is set and cleanup
		SetActorLocation(ExitTargetLocation);
		FinishLadderExitCleanup();
	}

	void APlayerCharacter::FinishLadderExitCleanup()
	{
		bIsExitingLadder = false;
		bIsAtTopOfLadder = false;
		CurrentLadder = nullptr;
		
		// Restore movement and collision
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		//Timer based on animation transition [Yeah more hardcoded thing but it is for animation only, totally optional delay here]
		//ReattachMeshAfterLadderClimb()
		GetWorldTimerManager().SetTimer(LadderExitTimer,this,&APlayerCharacter::ReattachMeshAfterLadderClimb,0.105f,false); 
	}

	void APlayerCharacter::ReattachMeshAfterLadderClimb()
	{
		// Reattach mesh properly
		GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		GetMesh()->SetRelativeLocation(MeshInitialPosition);
		GetMesh()->SetRelativeRotation(MeshInitialRotation);
	}

	// ========== UTILITY METHODS ==========

	bool APlayerCharacter::IsGrounded() const
	{
		return GetCharacterMovement()->IsMovingOnGround();
	}

	float APlayerCharacter::GetGroundDistance() const
	{
		UCapsuleComponent* Capsule = GetCapsuleComponent();
		float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 88.f;
		
		// Start at capsule bottom
		FVector Start = GetActorLocation() - FVector(0.f, 0.f, HalfHeight);
		FVector End   = Start - FVector(0.f, 0.f, GroundCheckDistance);
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.f, 0, 2.f);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
		{
			return (Start - Hit.Location).Size();
		}

		return MAX_FLT;
	}


	bool APlayerCharacter::CanStandUp() const
	{
		FVector Start = GetActorLocation() + FVector(0.f, 0.f, CrouchCapsuleHalfHeight);
		float CheckDistance = (StandCapsuleHalfHeight - CrouchCapsuleHalfHeight) - CeilingCheckOffset;
		FVector End = Start + FVector(0.f, 0.f, CheckDistance);
		float Radius = GetCapsuleComponent()->GetScaledCapsuleRadius();

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		return !GetWorld()->SweepTestByChannel(Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius), Params);
	}

	bool APlayerCharacter::CanCrouchUpFromProne() const
	{
		FVector Start = GetActorLocation() + FVector(0.f, 0.f, ProneCapsuleHalfHeight);
		float CheckDistance = (CrouchCapsuleHalfHeight - ProneCapsuleHalfHeight) - CeilingCheckOffset;
		FVector End = Start + FVector(0.f, 0.f, CheckDistance);
		float Radius = GetCapsuleComponent()->GetScaledCapsuleRadius();

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		return !GetWorld()->SweepTestByChannel(Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius), Params);
	}

	// ========== OVERLAP HANDLERS ==========

	void APlayerCharacter::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
	{
		if (!OtherActor)
			return;

		if (OtherActor->ActorHasTag(LadderTag))
		{
			if (!bIsLadderClimbing && !bIsExitingLadder)
				LadderCandidate = OtherActor;
				startingLadderZ = GetActorLocation().Z;
				UE_LOG(LogTemp, Warning, TEXT("ENTERING ladder at Z=%f"), GetActorLocation().Z);
		}

		if (OtherActor->ActorHasTag(LedgeTag))
		{
			if (!bIsOnLedge && !bLedgeExitCooldown)
				TryEnterLedge(OtherActor);
		}
	}

	void APlayerCharacter::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
	{
		if (!OtherActor)
			return;

		if (OtherActor->ActorHasTag(LadderTag))
		{
			if (LadderCandidate != nullptr && OtherActor == LadderCandidate)
				LadderCandidate = nullptr;

			// Handle exit from ladder while climbing
			if (bIsLadderClimbing && CurrentLadder != nullptr && OtherActor == CurrentLadder)
			{
				// In this case I check only as if the ladder is always a grounded ladder (no flying ladders)
				bool bExitingFromTop = startingLadderZ < GetActorLocation().Z;

				if (bExitingFromTop)
				{
					UE_LOG(LogTemp, Warning, TEXT("Exiting ladder from TOP at Z=%f"), GetActorLocation().Z);
					StartLadderTopExit();
				}
			}
		}

		if (CurrentLedge != nullptr && OtherActor == CurrentLedge)
		{
			ExitLedge();
		}
	}

	// ========== INPUT SETUP ==========

	void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
	{
		Super::SetupPlayerInputComponent(PlayerInputComponent);

		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
			EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopMove);
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
			EIC->BindAction(RunAction, ETriggerEvent::Started, this, &APlayerCharacter::RunPressed);
			EIC->BindAction(RunAction, ETriggerEvent::Completed, this, &APlayerCharacter::RunReleased);
			EIC->BindAction(DanceAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Dance);
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::QueueJumpInput);
			EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayerCharacter::HandleCrouchOrSlidePressed);
			EIC->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APlayerCharacter::HandleCrouchReleased);
			EIC->BindAction(ProneAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleProne);
			EIC->BindAction(RollAction, ETriggerEvent::Started, this, &APlayerCharacter::TryStartRoll);
			EIC->BindAction(GlideAction, ETriggerEvent::Started, this, &APlayerCharacter::GlidePressed);
			EIC->BindAction(GlideAction, ETriggerEvent::Completed, this, &APlayerCharacter::GlideReleased);
			EIC->BindAction(ClimbAction, ETriggerEvent::Started, this, &APlayerCharacter::ClimbPressed);
		}
	}