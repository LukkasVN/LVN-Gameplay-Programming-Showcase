#include "Elevator.h"

#include "FP_Character.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"

AElevator::AElevator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    ElevatorTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ElevatorTrigger"));
    ElevatorTrigger->SetupAttachment(RootComponent);
    
    ElevatorTrigger->OnComponentBeginOverlap.AddDynamic(this, &AElevator::OnOverlapBegin);
    ElevatorTrigger->OnComponentEndOverlap.AddDynamic(this, &AElevator::OnOverlapEnd);
}

void AElevator::BeginPlay()
{
    Super::BeginPlay();
    CurrentFloor = InitialFloor;
    bWasCalledExternally = false;

    if (bUseDoors && DoorLeft && DoorRight)
    {
        LeftClosedPos = DoorLeft->GetRelativeLocation();
        RightClosedPos = DoorRight->GetRelativeLocation();
        LeftOpenPos = LeftClosedPos + FVector(-DoorOpenDistance, 0, 0);
        RightOpenPos = RightClosedPos + FVector(DoorOpenDistance, 0, 0);
    }

    if(Floors.IsValidIndex(CurrentFloor)) 
    {
        SetActorLocation(Floors[CurrentFloor]->GetActorLocation());
        SetActorRotation(Floors[CurrentFloor]->GetActorRotation());
    }
}

void AElevator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (State == EElevatorState::Moving) MovementTick(DeltaTime);
    else if (State == EElevatorState::Rotating) RotationTick(DeltaTime);
}

void AElevator::CallElevator(int32 FloorIndex)
{
    if (State != EElevatorState::Idle) return;

    bWasCalledExternally = true; 

    if (FloorIndex == CurrentFloor) { OpenDoors(); return; }
    MoveToFloor(FloorIndex);
}

void AElevator::MoveToFloor(int32 FloorIndex)
{
    if (!Floors.IsValidIndex(FloorIndex) || FloorIndex == CurrentFloor) return;

    if (State == EElevatorState::OpeningDoors)
    {
        FloorQueue.Empty();
        int32 Step = (FloorIndex > CurrentFloor) ? 1 : -1;
        for (int32 F = CurrentFloor + Step; F != FloorIndex + Step; F += Step) FloorQueue.Add(F);
        return;
    }

    if (State != EElevatorState::Idle) return;
    
    FloorQueue.Empty();
    int32 Step = (FloorIndex > CurrentFloor) ? 1 : -1;
    for (int32 F = CurrentFloor + Step; F != FloorIndex + Step; F += Step) FloorQueue.Add(F);
    
    if (bUseDoors) 
    {
        CloseDoors();
    }
    else StartNextSegment();
}

void AElevator::OpenDoors() 
{ 
    if (bUseDoors) 
    { 
        State = EElevatorState::OpeningDoors; 
        AnimateDoors(true); 
    } 
    else State = EElevatorState::Idle; 
}

void AElevator::CloseDoors() 
{ 
    if (bUseDoors) 
    {
        State = EElevatorState::ClosingDoors;
        AnimateDoors(false); 
    }
    else StartNextSegment(); 
}

void AElevator::AnimateDoors(bool bOpening)
{
    if (!DoorLeft || !DoorRight) 
    { 
        if (bOpening) State = EElevatorState::Idle; 
        else StartNextSegment(); 
        return; 
    }

    DoorLTarget = bOpening ? LeftOpenPos : LeftClosedPos;
    DoorRTarget = bOpening ? RightOpenPos : RightClosedPos;
    bAnimatingDoorsForward = bOpening;
    GetWorldTimerManager().SetTimer(DoorAnimationTimer, this, &AElevator::DoorAnimationTick, 0.016f, true);
}

void AElevator::DoorAnimationTick()
{
    DoorLeft->SetRelativeLocation(FMath::VInterpTo(DoorLeft->GetRelativeLocation(), DoorLTarget, 0.016f, DoorSpeed));
    DoorRight->SetRelativeLocation(FMath::VInterpTo(DoorRight->GetRelativeLocation(), DoorRTarget, 0.016f, DoorSpeed));

    if (FVector::DistSquared(DoorLeft->GetRelativeLocation(), DoorLTarget) < 1.f)
    {
        GetWorldTimerManager().ClearTimer(DoorAnimationTimer);
        
        if (bAnimatingDoorsForward)
        {
            State = EElevatorState::Idle;

            if (FloorQueue.Num() > 0)
            {
                CloseDoors();
            }
            else if (MainOccupantCount == 0 && !bWasCalledExternally)
            {
                GetWorldTimerManager().SetTimer(AutoMoveTimerHandle, this, &AElevator::CloseDoors, CloseDoorCooldown, false);
            }
        }
        else
        {
            if (FloorQueue.Num() > 0) 
            {
                StartNextSegment();
            }
            else 
            {
                State = EElevatorState::Idle;
            }
        }
    }
}

void AElevator::StartNextSegment()
{
    if (FloorQueue.Num() == 0) { FinishMovement(); return; }
    TargetFloor = FloorQueue[0];
    FloorQueue.RemoveAt(0);

    StartPos = GetActorLocation();
    EndPos = Floors[TargetFloor]->GetActorLocation();
    StartRot = GetActorQuat();
    EndRot = Floors[TargetFloor]->GetActorQuat();

    MoveTimer = 0.f;
    State = EElevatorState::Moving;
}

void AElevator::MovementTick(float DeltaTime)
{
    MoveTimer += DeltaTime;
    float t = FMath::Clamp(MoveTimer / TravelTime, 0.f, 1.f);
    float Curved = MovementCurve ? MovementCurve->GetFloatValue(t) : t;

    SetActorLocation(FMath::Lerp(StartPos, EndPos, Curved), false, nullptr, ETeleportType::TeleportPhysics);

    if (t >= 1.f) {
        CurrentFloor = TargetFloor;
        if (FloorQueue.Num() > 0) StartNextSegment();
        else FinishMovement();
    }
}

void AElevator::RotationTick(float DeltaTime)
{
    RotationTimer += DeltaTime;
    float t = FMath::Clamp(RotationTimer / ArrivalRotationTime, 0.f, 1.f);
    float Curved = RotationCurve ? RotationCurve->GetFloatValue(t) : t;

    SetActorRotation(FQuat::Slerp(StartRot, EndRot, Curved), ETeleportType::TeleportPhysics);
    if (t >= 1.f) FinishMovement();
}

void AElevator::FinishMovement()
{
    float Angle = FQuat::ErrorAutoNormalize(GetActorQuat(), EndRot);
    if (Angle > 0.01f && ArrivalRotationTime > 0.f)
    {
        State = EElevatorState::Rotating;
        RotationTimer = 0.f;
        StartRot = GetActorQuat();
    }
    else { OpenDoors(); }
}

void AElevator::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    bool bIsMain = false;
    for (const FName& Tag : MainOccupantTags) { if (OtherActor->ActorHasTag(Tag)) { bIsMain = true; break; } }

    if (bIsMain)
    {
        MainOccupantCount++;
        GetWorldTimerManager().ClearTimer(AutoMoveTimerHandle);
        bWasCalledExternally = false;

        // Dirty fix for jump reset and physics interaction while on elevator - assumes main occupants are characters
        if (ACharacter* Character = Cast<ACharacter>(OtherActor))
        {
            if (AFP_Character* FP_Char = Cast<AFP_Character>(OtherActor))
            {
                FP_Char->SetCanJump(false);
            }
            if (UCharacterMovementComponent* CMC = Character->GetCharacterMovement())
            {
                CMC->bEnablePhysicsInteraction = false;
            }
        }
        
        // If this is a main occupant and there are only 2 floors, auto-move to the other floor after cooldown
        if (Floors.Num() == 2)
        {
            int32 NextF = (CurrentFloor == 0) ? 1 : 0;
            GetWorldTimerManager().SetTimer(AutoMoveTimerHandle, [this, NextF]() { MoveToFloor(NextF); }, CloseDoorCooldown, false);
        }
    }
}

void AElevator::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    bool bIsMain = false;
    for (const FName& Tag : MainOccupantTags) { if (OtherActor->ActorHasTag(Tag)) { bIsMain = true; break; } }

    if (bIsMain)
    {
        MainOccupantCount = FMath::Max(0, MainOccupantCount - 1);
        
        // Dirty fix for jump reset and physics interaction while on elevator - assumes main occupants are characters
        if (ACharacter* Character = Cast<ACharacter>(OtherActor))
        {
            if (AFP_Character* FP_Char = Cast<AFP_Character>(OtherActor))
            {
                FP_Char->SetCanJump(true);
            }
            if (UCharacterMovementComponent* CMC = Character->GetCharacterMovement())
            {
                CMC->bEnablePhysicsInteraction = true;
            }
        }

        // If this is a main occupant, auto close doors after cooldown
        bWasCalledExternally = false;
        if (State == EElevatorState::Idle || State == EElevatorState::OpeningDoors)
        {
            GetWorldTimerManager().SetTimer(AutoMoveTimerHandle, this, &AElevator::CloseDoors, CloseDoorCooldown, false);
        }
    }
}