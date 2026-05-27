#include "Destructible_System/DestructibleActor.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Destructible_System/HitWeaponData.h"
#include "Engine/World.h"

ADestructibleActor::ADestructibleActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionObjectType(ECC_WorldDynamic);
    Mesh->SetCollisionResponseToAllChannels(ECR_Block);
    RootComponent = Mesh;
}

void ADestructibleActor::BeginPlay()
{
    Super::BeginPlay();

    InitialScale = GetActorScale3D();

    if (Data)
    {
        CurrentHP = Data->HitPoints;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[DestructibleActor] %s has no Data assigned!"), *GetName());
    }
}

void ADestructibleActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsPopping)
        TickPopEffect(DeltaTime);
}

bool ADestructibleActor::TakeHit_Implementation(UHitWeaponData* Weapon, FVector HitLocation, FVector HitDirection)
{
    if (!Data || !Weapon || CurrentHP <= 0)
        return false;

    if (Weapon->Tier < Data->TierRequired)
    {
        UE_LOG(LogTemp, Verbose, TEXT("[DestructibleActor] %s rejected hit: weapon tier %d < required %d"),
            *Data->DestructibleName, Weapon->Tier, Data->TierRequired);
        return false;
    }

    CurrentHP = FMath::Max(0, CurrentHP - Weapon->Damage);

    if (Data->bUseHitPopEffect)
        StartHitPop();

    // EXPANSION POINT: play hit SFX/particles here using fields you've added to UDestructibleData. -------
    // Spawn hit particle facing BACK toward the player (opposite of hit direction)
    if (Data->HitParticle)
    {
        // Base direction
        FVector BaseDirection = -HitDirection;
    
        // Randomize within a 45° cone around the base direction
        FVector RandomDirection = FMath::VRandCone(BaseDirection, FMath::DegreesToRadians(45.f));
        FRotator ParticleRotation = RandomDirection.Rotation();
    
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            Data->HitParticle,
            HitLocation,
            ParticleRotation,
            FVector::OneVector,
            true,
            true,
            ENCPoolMethod::AutoRelease);
    }
    // ------
    
    OnHit.Broadcast(Data, HitLocation, HitDirection);

    if (CurrentHP <= 0)
        HandleDestruction(HitLocation, HitDirection);

    return true;
}

void ADestructibleActor::ForceDestroy_Implementation()
{
    if (CurrentHP <= 0) return;
    CurrentHP = 0;
    HandleDestruction(GetActorLocation(), FVector::ZeroVector);
}

void ADestructibleActor::HandleDestruction(FVector /*HitLocation*/, FVector /*HitDirection*/) // You can remove HitLocation and HitDirection if using the code as it is
{
    OnDestroy.Broadcast(Data, GetActorLocation()); // OnDestroy event call

    // EXPANSION POINT (Same as TakeHit): play hit SFX/particles here using fields you've added to UDestructibleData. --------
    //Example for destruction particles
    if (Data && Data->DestroyParticle)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            Data->DestroyParticle,
            GetActorLocation(), // Destructible's center location
            GetActorRotation(), // Destructible's rotation
            FVector::OneVector,
            true,
            true,
            ENCPoolMethod::AutoRelease);
    }
    //-------

    if (bSpawnDestroyedActor && Data && Data->DestroyedActorClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // You may modify this section with more offsets or directly making the destroyed actor have the destructible's values
        SpawnedDestroyedActor = GetWorld()->SpawnActor<AActor>(
            Data->DestroyedActorClass,
            GetActorLocation() + DestroyedActorOffset, // For example, I added an offset while tracking the destructible's position
            Data->DestroyedActorClass->GetDefaultObject<AActor>()->GetActorRotation(),  // And the destroyed actor's original rotation
            SpawnParams);

        if (SpawnedDestroyedActor)
            SpawnedDestroyedActor->SetActorScale3D(GetActorScale3D()); // Same goes for scale, you can also add your own offset
    }

    //Respawn Logic
    if (bIsRespawnable) 
    {
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        bIsPopping = false;
        SetActorScale3D(InitialScale);

        if (RespawnDelay > 0.f)
        {
            GetWorldTimerManager().SetTimer(
                RespawnTimerHandle,
                this,
                &ADestructibleActor::RestoreDestructible,
                RespawnDelay,
                false);
        }
    }
    else
    {
        SetLifeSpan(FMath::Max(0.001f, DestroyDelay));
    }
}

void ADestructibleActor::RestoreDestructible()
{
    if (!bIsRespawnable || !Data) return;

    // Clear the spawned destroyed actor if it's still around
    if (IsValid(SpawnedDestroyedActor))
    {
        SpawnedDestroyedActor->Destroy();
        SpawnedDestroyedActor = nullptr;
    }

    GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

    CurrentHP = Data->HitPoints;
    bIsPopping = false;
    SetActorScale3D(InitialScale);
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    OnRestored.Broadcast();
}

void ADestructibleActor::RestoreHP()
{
    if (Data) CurrentHP = Data->HitPoints;
}

void ADestructibleActor::StartHitPop()
{
    bIsPopping = true;
    bPopScalingUp = true;
    PopElapsed = 0.f;
}

void ADestructibleActor::TickPopEffect(float DeltaTime)
{
    if (!Data) { bIsPopping = false; return; }

    PopElapsed += DeltaTime;
    const float HalfDuration = FMath::Max(0.01f, Data->PopEffectDuration * 0.5f);
    const float Progress = FMath::Clamp(PopElapsed / HalfDuration, 0.f, 1.f);

    const FVector TargetScale = InitialScale * (1.f + Data->PopEffectIntensity);

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

