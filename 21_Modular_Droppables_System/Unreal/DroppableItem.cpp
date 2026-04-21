#include "DroppableItem.h"
#include "DropManagerSubsystem.h"
#include "Components/SphereComponent.h"

ADroppableItem::ADroppableItem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Physics Collider
    PhysicsCollider = CreateDefaultSubobject<USphereComponent>(TEXT("PhysicsCollider"));
    PhysicsCollider->SetSphereRadius(15.f);
    PhysicsCollider->SetSimulatePhysics(true);
    PhysicsCollider->SetEnableGravity(true);
    PhysicsCollider->BodyInstance.bLockXRotation = true;
    PhysicsCollider->BodyInstance.bLockYRotation = true;
    PhysicsCollider->SetLinearDamping(1.f);
    PhysicsCollider->SetAngularDamping(0.7f);
    PhysicsCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PhysicsCollider->SetCollisionObjectType(ECC_WorldDynamic);
    PhysicsCollider->SetCollisionResponseToAllChannels(ECR_Block);
    PhysicsCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    PhysicsCollider->SetNotifyRigidBodyCollision(true);
    RootComponent = PhysicsCollider;

    // Collection Trigger
    CollectionTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionTrigger"));
    CollectionTrigger->SetupAttachment(RootComponent);
    CollectionTrigger->SetSphereRadius(50.f);
    CollectionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollectionTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    CollectionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollectionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollectionTrigger->SetGenerateOverlapEvents(true);
}

void ADroppableItem::BeginPlay()
{
    Super::BeginPlay();

    InitialScale = GetActorScale3D();
    CurrentYaw = GetActorRotation().Yaw;

    CollectionTrigger->OnComponentBeginOverlap.AddDynamic(
        this, &ADroppableItem::OnTriggerBeginOverlap);
    PhysicsCollider->OnComponentHit.AddDynamic(
        this, &ADroppableItem::OnPhysicsHit);

    if (ItemData != nullptr)
    {
        ResetRuntimeState();
        StopPhysicsClean();

        UDroppableFloatSettings* Settings = GetFloatSettings();
        if (Settings->bEnabled)
        {
            SettlePosition = GetActorLocation();
            FloatStartZ = GetActorLocation().Z;
            FloatTargetZ = SettlePosition.Z + Settings->GroundOffset;
            FloatTransitionElapsed = 0.f;
            DropState = EDropState::TransitioningToFloat;
        }
        else
        {
            FloatAnchorPosition = GetActorLocation();
            DropState = EDropState::Floating;
        }

        UE_LOG(LogTemp, Log, TEXT("[DroppableItem] Pre-placed: %s"), *ItemData->ItemName);
    }
}

void ADroppableItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsPopping)
        TickPopEffect(DeltaTime);

    switch (DropState)
    {
        case EDropState::WaitingToSettle:
            TickWaitingToSettle(DeltaTime);
            break;
        case EDropState::TransitioningToFloat:
            TickTransitioningToFloat(DeltaTime);
            break;
        case EDropState::Floating:
            TickFloating(DeltaTime);
            break;
        case EDropState::BeingCollected:
            TickBeingCollected(DeltaTime);
            break;
        default:
            break;
    }
}

void ADroppableItem::ResetRuntimeState()
{
    bHasLanded = false;
    bCollectDelayFinished = false;
    SettleElapsed = 0.f;
    SettlePosition = FVector::ZeroVector;
    FloatTransitionElapsed = 0.f;
    FloatStartZ = 0.f;
    FloatTargetZ = 0.f;
    FloatAnchorPosition = FVector::ZeroVector;
    bIsPopping = false;
    PopElapsed = 0.f;
    bPopScalingUp = true;
    CollectorActor = nullptr;
    CurrentYaw = GetActorRotation().Yaw;
    DropState = EDropState::Scattering;

    SetActorScale3D(InitialScale);
    SetActorRotation(FRotator(0.f, CurrentYaw, 0.f));

    GetWorldTimerManager().ClearTimer(CollectDelayTimerHandle);
}

// Optional method, just in case you need to force-stop physics
void ADroppableItem::StopPhysicsClean()
{
    PhysicsCollider->SetSimulatePhysics(false);
    PhysicsCollider->SetEnableGravity(false);
    PhysicsCollider->SetPhysicsLinearVelocity(FVector::ZeroVector);
    PhysicsCollider->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
}

void ADroppableItem::OnSpawn()
{
    if (!ItemData)
    {
        UE_LOG(LogTemp, Error, TEXT("[DroppableItem] OnSpawn called with null ItemData!"));
        return;
    }

    ResetRuntimeState();

    PhysicsCollider->SetEnableGravity(true);
    PhysicsCollider->SetSimulatePhysics(true);

    UE_LOG(LogTemp, Log, TEXT("[DroppableItem] Spawned: %s x%d at %s"),*ItemData->ItemName, Quantity, *GetActorLocation().ToString());

    ApplyScatterEffect();

    // Start settle tracking immediately, but delay the actual check until the collect delay has passed.
    DropState = EDropState::WaitingToSettle;

    // Start delay in parallel
    GetWorldTimerManager().SetTimer(CollectDelayTimerHandle,this,&ADroppableItem::OnCollectDelayFinished,ItemData->CollectDelay,false);
}

// Scatter the item with an initial random impulse and spin when it spawns.
void ADroppableItem::ApplyScatterEffect()
{
    UDroppableScatterSettings* Settings = GetScatterSettings();
    if (!Settings->bEnabled) return;

    FVector RandomDir = FMath::VRand();
    float RandomForce = FMath::RandRange(Settings->ForceMin, Settings->ForceMax);

    FVector ScatterForce = RandomDir * RandomForce;
    ScatterForce.Z = FMath::Abs(ScatterForce.Z) + Settings->UpForce;

    PhysicsCollider->SetPhysicsLinearVelocity(ScatterForce);

    FVector RandomSpin = FMath::VRand() * (ScatterForce.Size() * Settings->SpinForceMultiplier);
    PhysicsCollider->SetPhysicsAngularVelocityInDegrees(RandomSpin);
}

// Collision / Overlap

void ADroppableItem::OnPhysicsHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (bHasLanded || DropState != EDropState::WaitingToSettle) return;

    if (Hit.ImpactNormal.Z > 0.2f)
    {
        bHasLanded = true;
        SettlePosition = GetActorLocation();
    }
}

void ADroppableItem::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp,AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,bool bFromSweep, const FHitResult& SweepResult)
{
    if (DropState != EDropState::Floating && DropState != EDropState::TransitioningToFloat)
        return;

    if (!OtherActor || !ItemData) return;

    if (OtherActor->Tags.Contains(FName("Player")))
    {
        UE_LOG(LogTemp, Log, TEXT("[DroppableItem] Player overlapped: %s"), *ItemData->ItemName);
        StartCollecting(OtherActor);
    }
}

// Delay & Settle for Scatter

void ADroppableItem::OnCollectDelayFinished()
{
    bCollectDelayFinished = true;
}

void ADroppableItem::TickWaitingToSettle(float DeltaTime)
{
    SettleElapsed += DeltaTime;

    float Velocity = PhysicsCollider->GetPhysicsLinearVelocity().Size();

    bool bVelocityLow = Velocity <= SettledVelocityThreshold;

    bool bReadyAfterDelay = bCollectDelayFinished && (bHasLanded || bVelocityLow);

    bool bTimedOut = SettleElapsed >= SettleCheckTimeout;

    if (bReadyAfterDelay || bTimedOut)
    {
        if (bTimedOut)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[DroppableItem] Settle timeout — forcing collectible"));
        }

        SettlePosition = GetActorLocation();
        MakeCollectible();
    }
}


bool ADroppableItem::GetIsCollectible_Implementation() const
{
    return DropState == EDropState::Floating
        || DropState == EDropState::TransitioningToFloat;
}

bool ADroppableItem::GetIsBeingCollected_Implementation() const
{
    return DropState == EDropState::BeingCollected;
}

void ADroppableItem::MakeCollectible()
{
    StopPhysicsClean();

    CurrentYaw = GetActorRotation().Yaw;
    SetActorRotation(FRotator(0.f, CurrentYaw, 0.f));

    UDroppableFloatSettings* Settings = GetFloatSettings();

    if (Settings->bEnabled)
    {
        FloatStartZ = GetActorLocation().Z;
        FloatTargetZ = SettlePosition.Z + Settings->GroundOffset;
        FloatTransitionElapsed = 0.f;
        DropState = EDropState::TransitioningToFloat;
    }
    else
    {
        FloatAnchorPosition = GetActorLocation();
        DropState = EDropState::Floating;
    }

    UE_LOG(LogTemp, Log, TEXT("[DroppableItem] %s is now collectible!"), *ItemData->ItemName);
}

void ADroppableItem::StartCollecting(AActor* Collector)
{
    if (DropState == EDropState::BeingCollected) return;

    CollectorActor = Collector;
    DropState = EDropState::BeingCollected;

    StopPhysicsClean();

    bIsPopping = true;
    bPopScalingUp = true;
    PopElapsed = 0.f;

    UE_LOG(LogTemp, Log, TEXT("[DroppableItem] Starting collection: %s"), *ItemData->ItemName);
}

void ADroppableItem::OnCollected()
{
    if (!ItemData) return;

    UE_LOG(LogTemp, Log, TEXT("[DroppableItem] Collected: %s x%d"),*ItemData->ItemName, Quantity);

    OnCollectedEvent.Broadcast(); // You can add logic in a Blueprints binding this "OnCollectedEvent" to it.

    // EXPANSION POINT: hook into your custom system here (Examples: Inventory, Quest tracking, Achievements, etc)

    if (bManagedBySubsystem)
    {
        if (UDropManagerSubsystem* Manager = GetWorld()->GetSubsystem<UDropManagerSubsystem>())Manager->OnDropCollected(this);
        // Manager calls Destroy(), do not call it here
    }
    else
    {
        Destroy();
    }
}

void ADroppableItem::SetItemData_Implementation(UDroppableItemData* Data)
{
    ItemData = Data;
}

// Tick States
void ADroppableItem::TickTransitioningToFloat(float DeltaTime)
{
    UDroppableFloatSettings* Settings = GetFloatSettings();

    FloatTransitionElapsed += DeltaTime;
    float Progress = FMath::Clamp(FloatTransitionElapsed / Settings->TransitionDuration, 0.f, 1.f);
    float Eased = FMath::SmoothStep(0.f, 1.f, Progress);
    float NewZ = FMath::Lerp(FloatStartZ, FloatTargetZ, Eased);

    FVector Loc = GetActorLocation();
    SetActorLocation(FVector(Loc.X, Loc.Y, NewZ));

    if (Progress >= 1.f)
    {
        FloatAnchorPosition = GetActorLocation();
        DropState = EDropState::Floating;
    }
}

void ADroppableItem::TickFloating(float DeltaTime)
{
    UDroppableFloatSettings* Settings = GetFloatSettings();
    if (!Settings->bEnabled) return;

    float Time = GetWorld()->GetTimeSeconds();
    float BobOffset = FMath::Sin(Time * Settings->BobSpeed) * Settings->BobHeight * 0.5f;

    FVector Loc = GetActorLocation();
    SetActorLocation(FVector(Loc.X, Loc.Y, FloatAnchorPosition.Z + BobOffset));

    CurrentYaw += Settings->RotationSpeed * DeltaTime;
    if (CurrentYaw > 360.f) CurrentYaw -= 360.f;
    SetActorRotation(FRotator(0.f, CurrentYaw, 0.f));
}

void ADroppableItem::TickBeingCollected(float DeltaTime)
{
    if (!CollectorActor || !ItemData) return;

    FVector Target = CollectorActor->GetActorLocation() + FVector(0.f, 0.f, 80.f);
    FVector Direction = (Target - GetActorLocation()).GetSafeNormal();
    SetActorLocation(GetActorLocation() + Direction * ItemData->CollectSpeed * DeltaTime);

    if (FVector::Dist(GetActorLocation(), Target) < 15.f)
        OnCollected();
}

void ADroppableItem::TickPopEffect(float DeltaTime)
{
    UDroppablePopSettings* Settings = GetPopSettings();
    if (!Settings->bEnabled) { bIsPopping = false; return; }

    PopElapsed += DeltaTime;
    float Progress = FMath::Clamp(PopElapsed / Settings->Duration, 0.f, 1.f);
    FVector TargetScale = InitialScale * Settings->Scale;

    if (bPopScalingUp)
    {
        SetActorScale3D(FMath::Lerp(InitialScale, TargetScale, Progress));
        if (Progress >= 1.f) { bPopScalingUp = false; PopElapsed = 0.f; }
    }
    else
    {
        SetActorScale3D(FMath::Lerp(TargetScale, InitialScale, Progress));
        if (Progress >= 1.f) { SetActorScale3D(InitialScale); bIsPopping = false; }
    }
}

// Default Settings

UDroppableScatterSettings* ADroppableItem::GetScatterSettings() const
{
    if (ScatterSettings) return ScatterSettings;
    return NewObject<UDroppableScatterSettings>();
}

UDroppablePopSettings* ADroppableItem::GetPopSettings() const
{
    if (PopSettings) return PopSettings;
    return NewObject<UDroppablePopSettings>();
}

UDroppableFloatSettings* ADroppableItem::GetFloatSettings() const
{
    if (FloatSettings) return FloatSettings;
    return NewObject<UDroppableFloatSettings>();
}