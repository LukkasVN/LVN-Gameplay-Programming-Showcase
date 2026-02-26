#include "FP_Character.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FP_Interactable.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Components/SpotLightComponent.h"

AFP_Character::AFP_Character()
{
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 0.f;
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->bDoCollisionTest = false;

    PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
    PlayerCamera->SetupAttachment(CameraBoom);

    GetCharacterMovement()->bOrientRotationToMovement = false;
    bUseControllerRotationYaw = true;

    FlashlightComponent = CreateDefaultSubobject<UFlashlightComponent>(TEXT("FlashlightComponent"));

    FlashlightPivot = CreateDefaultSubobject<USceneComponent>(TEXT("FlashlightPivot"));
    FlashlightPivot->SetupAttachment(RootComponent);
    FlashlightPivot->SetUsingAbsoluteRotation(true);
    FlashlightPivot->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

    FlashlightLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightLight"));
    FlashlightLight->SetupAttachment(FlashlightPivot);
    FlashlightLight->SetMobility(EComponentMobility::Movable);
    FlashlightLight->SetVisibility(false);

    // Give component references
    FlashlightComponent->Pivot = FlashlightPivot;
    FlashlightComponent->Light = FlashlightLight;
}

void AFP_Character::BeginPlay()
{
    Super::BeginPlay();

    CameraBaseOffset = PlayerCamera->GetRelativeLocation();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (InputMapping)
                Subsystem->AddMappingContext(InputMapping, 0);
        }
    }

    GetCharacterMovement()->GravityScale = GravityScale;
    GetCharacterMovement()->AirControl = AirControl;

    PlayerCamera->SetFieldOfView(DefaultFOV);

    bWasGrounded = !GetCharacterMovement()->IsFalling();

    FlashlightComponent->InitializeFlashlight(FlashlightIntensity,FlashlightRadius,FlashlightInnerCone,FlashlightOuterCone);
    FlashlightComponent->InitializeBattery(FlashlightMaxLifetimeSeconds,FlashlightLowBatteryPercent);
}

void AFP_Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Interaction trace
    if (bCanInteract)
    {
        AActor* HitActor = TraceInteractable(300.f);

        if (HitActor != CurrentInteractable)
        {
            if (CurrentInteractable)
                IFP_Interactable::Execute_OnUnfocus(CurrentInteractable, this);

            if (HitActor)
                IFP_Interactable::Execute_OnFocus(HitActor, this);

            CurrentInteractable = HitActor;
        }
    }

    // Ground detection
    bool bGrounded = !GetCharacterMovement()->IsFalling();

    if (bGrounded && !bWasGrounded)
    {
        JumpCount = 0;
        bUsedGroundJump = false;
    }

    bWasGrounded = bGrounded;

    // Auto-stop sprint
    if (bIsRunning)
    {
        const FVector Vel = GetVelocity();
        const float ForwardDot = FVector::DotProduct(GetActorForwardVector(), Vel.GetSafeNormal());

        if (ForwardDot < 0.1f)
            StopSprint();
    }

    // FOV
    const float TargetFOV = bIsRunning ? SprintFOV : DefaultFOV;
    const float NewFOV = FMath::FInterpTo(PlayerCamera->FieldOfView, TargetFOV, DeltaTime, FOVInterpSpeed);
    PlayerCamera->SetFieldOfView(NewFOV);

    ApplyBobbing(DeltaTime);
}

void AFP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

    if (MoveAction)
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFP_Character::Move);

    if (LookAction)
        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFP_Character::Look);

    if (RunAction)
    {
        EIC->BindAction(RunAction, ETriggerEvent::Started, this, &AFP_Character::StartSprint);
        EIC->BindAction(RunAction, ETriggerEvent::Completed, this, &AFP_Character::StopSprint);
    }

    if (JumpAction)
    {
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AFP_Character::HandleJump);
        EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
    }

    if (InteractAction)
        EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AFP_Character::InteractPressed);

    if (FlashlightAction)
        EIC->BindAction(FlashlightAction, ETriggerEvent::Started, this, &AFP_Character::TriggerFlashlight);
}

void AFP_Character::Move(const FInputActionValue& Value)
{
    if (!bCanMove) return;

    FVector2D Input = Value.Get<FVector2D>();

    AddMovementInput(GetActorForwardVector(), Input.Y);
    AddMovementInput(GetActorRightVector(), Input.X);
}

void AFP_Character::Look(const FInputActionValue& Value)
{
    if (!bCanLook) return;

    FVector2D LookInput = Value.Get<FVector2D>();

    AddControllerYawInput(LookInput.X * HorizontalSensitivity);

    Pitch = FMath::Clamp(
        Pitch + (LookInput.Y * VerticalSensitivity),
        -PitchClamp,
        PitchClamp
    );

    PlayerCamera->SetRelativeRotation(FRotator(Pitch, 0.f, 0.f));
}

void AFP_Character::HandleJump()
{
    if (!bCanJump) return;

    const bool bGrounded = !GetCharacterMovement()->IsFalling();

    if (bGrounded)
    {
        JumpCount = 1;
        bUsedGroundJump = true;
        Jump();
        return;
    }

    if (JumpCount == 0)
    {
        JumpCount = 1;
        bUsedGroundJump = false;
        LaunchCharacter(FVector(0,0,GetCharacterMovement()->JumpZVelocity), false, true);
        return;
    }

    if (bAllowDoubleJump && bUsedGroundJump && JumpCount == 1)
    {
        JumpCount = 2;
        LaunchCharacter(FVector(0,0,GetCharacterMovement()->JumpZVelocity), false, true);
    }
}

void AFP_Character::StartSprint()
{
    if (!bCanSprint) return;

    const FVector Velocity = GetVelocity();
    const float ForwardDot = FVector::DotProduct(GetActorForwardVector(), Velocity.GetSafeNormal());

    if (ForwardDot > 0.5f)
    {
        bIsRunning = true;
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    }
}

void AFP_Character::StopSprint()
{
    bIsRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AFP_Character::InteractPressed()
{
    if (!bCanInteract || !CurrentInteractable) return;

    IFP_Interactable::Execute_Interact(CurrentInteractable, this);
}

AActor* AFP_Character::TraceInteractable(float Distance)
{
    FVector Start = PlayerCamera->GetComponentLocation();
    FVector End = Start + PlayerCamera->GetForwardVector() * Distance;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        if (Hit.GetActor() && Hit.GetActor()->GetClass()->ImplementsInterface(UFP_Interactable::StaticClass()))
            return Hit.GetActor();
    }

    return nullptr;
}

void AFP_Character::TriggerFlashlight()
{
    if (FlashlightComponent && bCanUseFlashlight)
    {
        FlashlightComponent->ToggleFlashlight();
    }
}

void AFP_Character::ApplyBobbing(float DeltaTime)
{
    if (!bHasBobbing) return;

    FVector Target = CameraBaseOffset;
    float Time = GetWorld()->GetTimeSeconds();

    bool bGrounded = !GetCharacterMovement()->IsFalling();
    bool bMoving = GetVelocity().Size2D() > 10.f;

    if (bGrounded && bMoving)
    {
        float Freq = bIsRunning ? RunBobFrequency : WalkBobFrequency;
        float Amp = bIsRunning ? RunBobAmplitude : WalkBobAmplitude;
        Target.Z += FMath::Sin(Time * Freq) * Amp;
    }
    else if (bGrounded)
    {
        Target.Z += FMath::Sin(Time * IdleBreathFrequency) * IdleBreathAmplitude;
    }

    if (!bWasGrounded && bGrounded)
        LandingLerp = 1.f;

    if (LandingLerp > 0.f)
    {
        Target.Z += LandingDipAmount * LandingLerp;
        LandingLerp -= DeltaTime * LandingDipSpeed;
    }

    bWasGrounded = bGrounded;

    FVector Current = PlayerCamera->GetRelativeLocation();
    FVector Smoothed = FMath::VInterpTo(Current, Target, DeltaTime, 12.f);
    PlayerCamera->SetRelativeLocation(Smoothed);
}

void AFP_Character::SetCanMove(bool bEnabled) { bCanMove = bEnabled; }
void AFP_Character::SetCanLook(bool bEnabled) { bCanLook = bEnabled; }
void AFP_Character::SetCanInteract(bool bEnabled) { bCanInteract = bEnabled; }
void AFP_Character::SetCanJump(bool bEnabled) { bCanJump = bEnabled; }
void AFP_Character::SetCanSprint(bool bEnabled) { bCanSprint = bEnabled; }
void AFP_Character::SetCanUseFlashlight(bool bEnabled) { bCanUseFlashlight = bEnabled; }
