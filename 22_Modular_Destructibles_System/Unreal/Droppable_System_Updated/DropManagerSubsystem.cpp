#include "DropManagerSubsystem.h"
#include "Engine/World.h"

void UDropManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[DropManagerSubsystem] Initialized"));
}

void UDropManagerSubsystem::Deinitialize()
{
    ActiveDrops.Empty();
    Super::Deinitialize();
}

ADroppableItem* UDropManagerSubsystem::DropItem(UDroppableItemData* ItemData, FVector Position)
{
    if (!ItemData)
    {
        UE_LOG(LogTemp, Error, TEXT("[DropManagerSubsystem] Cannot drop: null ItemData!"));
        return nullptr;
    }

    TSubclassOf<ADroppableItem> ItemClass{ ItemData->ItemClass };
    if (!ItemClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[DropManagerSubsystem] ItemData %s has no class assigned!"),*ItemData->ItemName);
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ADroppableItem* Item = GetWorld()->SpawnActor<ADroppableItem>(ItemClass, Position, FRotator::ZeroRotator, SpawnParams);

    if (!Item)
    {
        UE_LOG(LogTemp, Error, TEXT("[DropManagerSubsystem] SpawnActor failed for %s"), *ItemClass->GetName());
        return nullptr;
    }

    int32 FinalQuantity = FMath::RandRange(ItemData->MinDropQuantity, ItemData->MaxDropQuantity);

    Item->SetManagedBySubsystem(true);
    Item->SetQuantity(FinalQuantity);
    Item->Execute_SetItemData(Item, ItemData);

    ActiveDrops.Add(Item);
    Item->OnSpawn();

    UE_LOG(LogTemp, Log, TEXT("[DropManagerSubsystem] Dropped %s x%d | Active: %d"), *ItemData->ItemName, FinalQuantity, ActiveDrops.Num());

    return Item;
}

void UDropManagerSubsystem::OnDropCollected(ADroppableItem* Drop)
{
    if (!Drop) return;

    ActiveDrops.Remove(Drop);
    Drop->Destroy();

    UE_LOG(LogTemp, Log, TEXT("[DropManagerSubsystem] Item destroyed | Active: %d"),
        ActiveDrops.Num());
}