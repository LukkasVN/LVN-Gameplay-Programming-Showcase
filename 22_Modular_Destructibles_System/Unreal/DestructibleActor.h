#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IDestructible.h"
#include "DestructibleData.h"
#include "DestructibleActor.generated.h"

class UStaticMeshComponent;
class UHitWeaponData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnDestructibleHit,
    UDestructibleData*, Data,
    FVector, HitLocation,
    FVector, HitDirection);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnDestructibleDestroyed,
    UDestructibleData*, Data,
    FVector, Location);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestructibleRestored);

/**
  - Destructible actor.
  - Type config on UDestructibleData, placement overrides here.
  - Hit: TakeHit -> HP-- -> OnHit. If HP <= 0: HandleDestruction -> OnDestroyed.
  - Respawn: Hides and disables collision instead of destroying, RestoreDestructible() reactivates..
 */
UCLASS(BlueprintType, Blueprintable)
class MECHANICS_TEST_LVN_API ADestructibleActor : public AActor, public IDestructible
{
    GENERATED_BODY()

public:
    ADestructibleActor();

    // Type data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible")
    TObjectPtr<UDestructibleData> Data;

    // Placement-level overrides
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Placement")
    bool bIsRespawnable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Placement")
    bool bSpawnDestroyedActor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Placement", meta = (ClampMin = "0"))
    float DestroyDelay = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Placement")
    FVector DestroyedActorOffset = FVector::ZeroVector;

    // 0 = no auto-respawn (external systems must call RestoreDestructible).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Placement", meta = (ClampMin = "0"))
    float RespawnDelay = 0.f;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Mesh;

    // Events (C++ and Blueprint both bind here)
    UPROPERTY(BlueprintAssignable, Category = "Destructible|Events")
    FOnDestructibleHit OnHit;

    UPROPERTY(BlueprintAssignable, Category = "Destructible|Events")
    FOnDestructibleDestroyed OnDestroy;

    UPROPERTY(BlueprintAssignable, Category = "Destructible|Events")
    FOnDestructibleRestored OnRestored;

    // Destructible API
    UFUNCTION(BlueprintCallable, Category = "Destructible")
    void RestoreDestructible();

    UFUNCTION(BlueprintCallable, Category = "Destructible")
    void RestoreHP();

    UFUNCTION(BlueprintPure, Category = "Destructible")
    int32 GetCurrentHP() const { return CurrentHP; }

    UFUNCTION(BlueprintPure, Category = "Destructible")
    bool IsDestroyed() const { return CurrentHP <= 0; }

    // IDestructible Implementations
    virtual bool TakeHit_Implementation(UHitWeaponData* Weapon, FVector HitLocation, FVector HitDirection) override;
    virtual void ForceDestroy_Implementation() override;
    virtual UDestructibleData* GetDestructibleData_Implementation() const override { return Data; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    int32 CurrentHP = 0;

    // Pop effect state
    bool bIsPopping = false;
    float PopElapsed = 0.f;
    bool bPopScalingUp = true;
    FVector InitialScale = FVector::OneVector;

    // Respawn tracking
    UPROPERTY()
    TObjectPtr<AActor> SpawnedDestroyedActor;

    FTimerHandle RespawnTimerHandle;

    void HandleDestruction(FVector HitLocation, FVector HitDirection);
    void StartHitPop();
    void TickPopEffect(float DeltaTime);
};