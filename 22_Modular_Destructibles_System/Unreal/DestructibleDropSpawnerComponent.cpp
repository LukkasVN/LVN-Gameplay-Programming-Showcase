#include "Destructible_System/DestructibleDropSpawnerComponent.h"
#include "Destructible_System/DestructibleActor.h"
#include "Destructible_System/DestructibleData.h"
#include "Engine/World.h"
#include "Mechanics_Test_LVN/Droppable_System/DropManagerSubsystem.h"

UDestructibleDropSpawnerComponent::UDestructibleDropSpawnerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDestructibleDropSpawnerComponent::BeginPlay()
{
    Super::BeginPlay();

    if (ADestructibleActor* Owner = Cast<ADestructibleActor>(GetOwner()))
    {
        Owner->OnHit.AddDynamic(this, &UDestructibleDropSpawnerComponent::HandleHit);
        Owner->OnDestroy.AddDynamic(this, &UDestructibleDropSpawnerComponent::HandleDestroy);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[DestructibleDropSpawner] %s attached to non-Destructible owner '%s'. Component idle."),
            *GetName(),
            GetOwner() ? *GetOwner()->GetName() : TEXT("null"));
    }
}

void UDestructibleDropSpawnerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ADestructibleActor* Owner = Cast<ADestructibleActor>(GetOwner()))
    {
        Owner->OnHit.RemoveDynamic(this, &UDestructibleDropSpawnerComponent::HandleHit);
        Owner->OnDestroy.RemoveDynamic(this, &UDestructibleDropSpawnerComponent::HandleDestroy);
    }
    Super::EndPlay(EndPlayReason);
}

// Data and HitDirection available if per-type or directional drop logic is needed in the future.
void UDestructibleDropSpawnerComponent::HandleHit(UDestructibleData* /*Data*/, FVector HitLocation, FVector /*HitDirection*/)
{
    RollAndSpawn(DropsOnHit, HitLocation);
}

// Data available if per-type drop filtering is needed in the future.
void UDestructibleDropSpawnerComponent::HandleDestroy(UDestructibleData* /*Data*/, FVector Location)
{
    RollAndSpawn(DropsOnDestroy, Location);
}

void UDestructibleDropSpawnerComponent::RollAndSpawn(const TArray<FDestructibleDropEntry>& Entries, FVector Location)
{
    UDropManagerSubsystem* Manager = GetWorld() ? GetWorld()->GetSubsystem<UDropManagerSubsystem>() : nullptr;
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DestructibleDropSpawner] DropManagerSubsystem unavailable."));
        return;
    }

    for (const FDestructibleDropEntry& Entry : Entries)
    {
        if (!Entry.ItemData) continue;
        if (FMath::FRand() > Entry.DropChance) continue;

        const int32 Min = FMath::Max(1, Entry.MinDropCount);
        const int32 Max = FMath::Max(Min, Entry.MaxDropCount);

        int32 Count = Min;
        for (int32 i = Min; i < Max; ++i)
        {
            if (FMath::FRand() <= Entry.BonusRollChance)
                Count++;
        }

        for (int32 i = 0; i < Count; ++i)
        {
            Manager->DropItem(Entry.ItemData, Location);
        }
    }
}