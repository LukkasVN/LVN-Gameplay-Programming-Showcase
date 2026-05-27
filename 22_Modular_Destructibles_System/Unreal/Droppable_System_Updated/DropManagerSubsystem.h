#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DroppableItem.h"
#include "DropManagerSubsystem.generated.h"

/**
 * World subsystem that manages spawning and tracking of droppable items.
 * Spawns directly via SpawnActor.
 * Access via: GetWorld()->GetSubsystem<UDropManagerSubsystem>()
 */
UCLASS()
class MECHANICS_TEST_LVN_API UDropManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Drop Manager")
    ADroppableItem* DropItem(UDroppableItemData* ItemData, FVector Position);

    UFUNCTION(BlueprintCallable, Category = "Drop Manager")
    void OnDropCollected(ADroppableItem* Drop);

    UFUNCTION(BlueprintPure, Category = "Drop Manager")
    int32 GetActiveDropCount() const { return ActiveDrops.Num(); }

private:

    UPROPERTY()
    TArray<TObjectPtr<ADroppableItem>> ActiveDrops;
};