#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DestructibleDropSpawnerComponent.generated.h"

class UDroppableItemData;
class UDestructibleData;

USTRUCT(BlueprintType)
struct FDestructibleDropEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UDroppableItemData> ItemData;

    // Gates whether this entry spawns at all. 0 = never, 1 = always.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
    float DropChance = 1.f;

    // Guaranteed minimum if DropChance passes.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 MinDropCount = 1;

    // Maximum possible pickups. Each count above Min requires a BonusRollChance pass.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 MaxDropCount = 3;

    // Probability of each additional pickup above MinDropCount. 0 = always Min, 1 = always Max.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
    float BonusRollChance = 0.5f;

    // Random radius around the destructible to scatter spawn positions.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
    float SpawnRadius = 30.f;
};

/**
 - Companion component for ADestructibleActor.
 - Subscribes to OnHit / OnDestroyed and spawns drops via UDropManagerSubsystem (Feel free to change it if you want in RollAndSpawn).).
 - Does nothing if its owner isn't an ADestructibleActor.
 */
UCLASS(ClassGroup = (Destructible), meta = (BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API UDestructibleDropSpawnerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDestructibleDropSpawnerComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drops")
    TArray<FDestructibleDropEntry> DropsOnHit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drops")
    TArray<FDestructibleDropEntry> DropsOnDestroy;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UFUNCTION()
    void HandleHit(UDestructibleData* Data, FVector HitLocation, FVector HitDirection);

    UFUNCTION()
    void HandleDestroy(UDestructibleData* Data, FVector Location);

    void RollAndSpawn(const TArray<FDestructibleDropEntry>& Entries, FVector Location);
};