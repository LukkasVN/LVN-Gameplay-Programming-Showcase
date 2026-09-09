#include "FPS_Character.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

AFPS_Character::AFPS_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCapsuleComponent()->InitCapsuleSize(38.0f, 88.0f);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());

	// Pawn control rotation is applied manually in UpdateCameraTransform, because the
	// camera also needs a roll channel for tilt and bank, which pawn control rotation
	// would overwrite every frame.
	CameraComponent->bUsePawnControlRotation = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = false;
		Movement->MaxWalkSpeed = WalkSpeed;
		Movement->AirControl = 0.9f;
		Movement->BrakingDecelerationWalking = 2400.0f;
		Movement->GroundFriction = 9.0f;
	}
}

void AFPS_Character::BeginPlay()
{
	Super::BeginPlay();

	CameraBaseLocation = FVector(0.0f, 0.0f, CameraHeight);
	CameraComponent->SetRelativeLocation(CameraBaseLocation);
	CameraComponent->SetFieldOfView(BaseFOV);

	SmoothedBaseFOV = BaseFOV;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->JumpZVelocity = JumpVelocity;
		Movement->GravityScale = RiseGravityScale;
	}

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, MappingContextPriority);
			}
		}
	}
}

void AFPS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	if (MoveAction)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPS_Character::HandleMove);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AFPS_Character::HandleMove);
	}
	if (LookAction)
	{
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPS_Character::HandleLook);
	}
	if (JumpAction)
	{
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &AFPS_Character::HandleJumpPressed);
	}
	if (RunAction)
	{
		EnhancedInput->BindAction(RunAction, ETriggerEvent::Started, this, &AFPS_Character::HandleRunStarted);
		EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this, &AFPS_Character::HandleRunCompleted);
	}
	if (DashAction)
	{
		EnhancedInput->BindAction(DashAction, ETriggerEvent::Started, this, &AFPS_Character::HandleDashPressed);
	}
}

void AFPS_Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateGravity(DeltaSeconds);
	UpdateDash(DeltaSeconds);
	UpdateMovementState();
	UpdateHeadbob(DeltaSeconds);
	UpdateTilt(DeltaSeconds);

	LandingDipSpring.Tick(LandingDipStiffness, LandingDipDamping, DeltaSeconds);
	LandingFOVSpring.Tick(LandingFOVStiffness, LandingFOVDamping, DeltaSeconds);

	UpdateCameraTransform(DeltaSeconds);
	UpdateFOV(DeltaSeconds);

	LookTurnRate = FMath::FInterpTo(LookTurnRate, 0.0f, DeltaSeconds, 12.0f);
}

// Input

void AFPS_Character::HandleMove(const FInputActionValue& Value)
{
	MoveInput = Value.Get<FVector2D>();

	if (DashTimeRemaining > 0.0f || MoveInput.IsNearlyZero())
	{
		return;
	}

	const FRotator YawOnly(0.0f, GetControlRotation().Yaw, 0.0f);
	AddMovementInput(YawOnly.RotateVector(FVector::RightVector), MoveInput.X);
	AddMovementInput(YawOnly.RotateVector(FVector::ForwardVector), MoveInput.Y);
}

void AFPS_Character::HandleLook(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>() * LookSensitivity;

	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(-LookAxis.Y);

	LookTurnRate = LookAxis.X;
}

void AFPS_Character::HandleJumpPressed()
{
	Jump();
}

void AFPS_Character::HandleRunStarted()
{
	bRunInputHeld = true;
}

void AFPS_Character::HandleRunCompleted()
{
	bRunInputHeld = false;
}

void AFPS_Character::HandleDashPressed()
{
	if (!IsDashReady())
	{
		return;
	}

	FVector Direction = FVector::ZeroVector;
	const FRotator YawOnly(0.0f, GetControlRotation().Yaw, 0.0f);

	if (!MoveInput.IsNearlyZero())
	{
		Direction = YawOnly.RotateVector(FVector(MoveInput.Y, MoveInput.X, 0.0f));
	}
	else
	{
		Direction = YawOnly.Vector();
	}

	DashDirection = Direction.GetSafeNormal();
	DashTimeRemaining = DashDuration;
	DashCooldownRemaining = DashCooldown;

	const FVector LocalDash = YawOnly.UnrotateVector(DashDirection);
	DashBankTarget = -LocalDash.Y * DashBankAngle;

	OnDashStarted.Broadcast();
}

// Movement

void AFPS_Character::UpdateGravity(float DeltaSeconds)
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	const bool bRising = Movement->Velocity.Z > 0.0f;
	Movement->GravityScale = bRising ? RiseGravityScale : RiseGravityScale * FallGravityMultiplier;

	if (Movement->Velocity.Z < -TerminalVelocity)
	{
		Movement->Velocity.Z = -TerminalVelocity;
	}

	if (Movement->IsFalling())
	{
		LastFallSpeed = FMath::Max(LastFallSpeed, -Movement->Velocity.Z);
	}

	Movement->MaxWalkSpeed = bRunInputHeld ? RunSpeed : WalkSpeed;
}

void AFPS_Character::UpdateDash(float DeltaSeconds)
{
	if (DashCooldownRemaining > 0.0f)
	{
		DashCooldownRemaining = FMath::Max(0.0f, DashCooldownRemaining - DeltaSeconds);
	}

	if (DashTimeRemaining > 0.0f)
	{
		DashTimeRemaining -= DeltaSeconds;

		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			// Vertical velocity is left alone so a dash off a ledge still arcs.
			const float PreservedZ = Movement->Velocity.Z;
			Movement->Velocity = DashDirection * DashSpeed;
			Movement->Velocity.Z = PreservedZ;
		}
	}
	else
	{
		DashBankTarget = 0.0f;
	}

	const bool bDashing = DashTimeRemaining > 0.0f;
	const float TargetFOVKick = bDashing ? DashFOVKick : 0.0f;
	const float Speed = bDashing ? DashFOVInSpeed : DashFOVOutSpeed;
	DashFOVCurrent = FMath::FInterpTo(DashFOVCurrent, TargetFOVKick, DeltaSeconds, Speed);

	DashBankCurrent = FMath::FInterpTo(DashBankCurrent, DashBankTarget, DeltaSeconds, DashBankInterpSpeed);
}

void AFPS_Character::UpdateMovementState()
{
	if (DashTimeRemaining > 0.0f)
	{
		CurrentState = EFPS_MovementState::Dashing;
		return;
	}

	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Movement && Movement->IsFalling())
	{
		CurrentState = EFPS_MovementState::Airborne;
		return;
	}

	const float PlanarSpeed = GetVelocity().Size2D();
	if (PlanarSpeed < 10.0f || MoveInput.IsNearlyZero())
	{
		CurrentState = EFPS_MovementState::Idle;
		return;
	}

	CurrentState = bRunInputHeld ? EFPS_MovementState::Running : EFPS_MovementState::Walking;
}

void AFPS_Character::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	const float Range = FMath::Max(LandingMaxImpactSpeed - LandingMinImpactSpeed, 1.0f);
	const float Alpha = FMath::Clamp((LastFallSpeed - LandingMinImpactSpeed) / Range, 0.0f, 1.0f);

	if (Alpha > 0.0f)
	{
		LandingDipSpring.AddImpulse(LandingDipImpulse * Alpha);
		LandingFOVSpring.AddImpulse(LandingFOVImpulse * Alpha);
		OnLandedImpact.Broadcast(LastFallSpeed);
	}

	LastFallSpeed = 0.0f;
}

// Camera

void AFPS_Character::UpdateHeadbob(float DeltaSeconds)
{
	const bool bGroundedAndMoving =
		CurrentState == EFPS_MovementState::Walking || CurrentState == EFPS_MovementState::Running;

	const float SpeedRatio = RunSpeed > 0.0f ? FMath::Clamp(GetVelocity().Size2D() / RunSpeed, 0.0f, 1.0f) : 0.0f;
	const float TargetBlend = bGroundedAndMoving ? SpeedRatio : 0.0f;
	BobBlend = FMath::FInterpTo(BobBlend, TargetBlend, DeltaSeconds, BobBlendSpeed);

	if (BobBlend < KINDA_SMALL_NUMBER)
	{
		BobTime = 0.0f;
		BobOffset = FVector::ZeroVector;
		BobRoll = 0.0f;
		return;
	}

	BobTime += DeltaSeconds * BobFrequency * FMath::Max(SpeedRatio, 0.2f);

	const float LateralFactor = FMath::Abs(MoveInput.X);

	BobOffset.Z = FMath::Sin(BobTime * 2.0f) * BobVerticalAmount * BobBlend;
	BobOffset.Y = FMath::Sin(BobTime) * BobLateralAmount * BobBlend * LateralFactor;
	BobRoll = FMath::Sin(BobTime) * BobRollAmount * BobBlend * LateralFactor;
}

void AFPS_Character::UpdateTilt(float DeltaSeconds)
{
	const float StrafeRoll = -MoveInput.X * StrafeTiltAngle;

	const float TurnAlpha = LookTiltMaxTurnRate > 0.0f
		? FMath::Clamp(LookTurnRate / LookTiltMaxTurnRate, -1.0f, 1.0f)
		: 0.0f;
	const float LookRoll = TurnAlpha * LookTiltAngle;

	CurrentTiltRoll = FMath::FInterpTo(CurrentTiltRoll, StrafeRoll + LookRoll, DeltaSeconds, TiltInterpSpeed);
}

void AFPS_Character::UpdateCameraTransform(float DeltaSeconds)
{
	if (!CameraComponent)
	{
		return;
	}

	const FVector Location = CameraBaseLocation
		+ FVector(0.0f, BobOffset.Y, BobOffset.Z + LandingDipSpring.Value);

	const float Pitch = FRotator::NormalizeAxis(GetControlRotation().Pitch);
	const float Roll = CurrentTiltRoll + BobRoll + DashBankCurrent;

	CameraComponent->SetRelativeLocationAndRotation(Location, FRotator(Pitch, 0.0f, Roll));
}

void AFPS_Character::UpdateFOV(float DeltaSeconds)
{
	if (!CameraComponent)
	{
		return;
	}

	const float TargetBase = BaseFOV + (CurrentState == EFPS_MovementState::Running ? RunFOVOffset : 0.0f);
	SmoothedBaseFOV = FMath::FInterpTo(SmoothedBaseFOV, TargetBase, DeltaSeconds, FOVBlendSpeed);

	float FinalFOV = CameraZoomOverride.IsSet() ? CameraZoomOverride.GetValue() : SmoothedBaseFOV;
	FinalFOV += LandingFOVSpring.Value + DashFOVCurrent;

	CameraComponent->SetFieldOfView(FinalFOV);
}

void AFPS_Character::SetCameraZoomOverride(float InFOV)
{
	CameraZoomOverride = InFOV;
}

void AFPS_Character::ClearCameraZoomOverride()
{
	CameraZoomOverride.Reset();
}

float AFPS_Character::GetDashCooldownNormalized() const
{
	if (DashCooldown <= 0.0f)
	{
		return 1.0f;
	}

	return FMath::Clamp(1.0f - (DashCooldownRemaining / DashCooldown), 0.0f, 1.0f);
}
