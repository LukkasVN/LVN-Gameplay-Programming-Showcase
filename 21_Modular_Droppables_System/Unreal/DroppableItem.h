#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IDroppable.h"
#include "DroppableItemData.h"
#include "DroppableScatterSettings.h"
#include "DroppablePopSettings.h"
#include "DroppableFloatSettings.h"
#include "DroppableItem.generated.h"

class USphereComponent;

UENUM(BlueprintType)
enum class EDropState : uint8
{
    Scattering,
    WaitingToSettle,
    TransitioningToFloat,
    Floating,
    BeingCollected
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCollectedEvent);

/**
 * Actor representing a single dropped item in the world.
 * Can be placed directly in a level with ItemData assigned in the Details panel,
 * OR spawned and managed by UDropManagerSubsystem.
 */
UCLASS(BlueprintType, Blueprintable)
class MECHANICS_TEST_LVN_API ADroppableItem : public AActor, public IDroppable
{
    GENERATED_BODY()

public:
    ADroppableItem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Droppable")
    void MakeCollectible();

    UFUNCTION(BlueprintCallable, Category = "Droppable")
    void StartCollecting(AActor* Collector);

    UFUNCTION(BlueprintCallable, Category = "Droppable")
    void OnCollected();

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> CollectionTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> PhysicsCollider;

    // Item Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TObjectPtr<UDroppableItemData> ItemData;

    // Settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter Settings")
    TObjectPtr<UDroppableScatterSettings> ScatterSettings;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pop Settings")
    TObjectPtr<UDroppablePopSettings> PopSettings;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Float Settings")
    TObjectPtr<UDroppableFloatSettings> FloatSettings;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settle Settings")
    float SettledVelocityThreshold = 10.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settle Settings")
    float SettleCheckTimeout = 5.f;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCollectedEvent OnCollectedEvent;

    bool bManagedBySubsystem = true;

    void SetManagedBySubsystem(bool bManaged) { bManagedBySubsystem = bManaged; }
    bool IsManagedBySubsystem() const { return bManagedBySubsystem; }

    UFUNCTION(BlueprintCallable, Category = "Droppable")
    void OnSpawn();

    void SetQuantity(int32 InQuantity) { Quantity = InQuantity; }
    int32 GetQuantity() const { return Quantity; }

    EDropState GetDropState() const { return DropState; }

    // IDroppable
    virtual UDroppableItemData* GetItemData_Implementation() const override { return ItemData; }
    virtual bool GetIsCollectible_Implementation() const override;
    virtual bool GetIsBeingCollected_Implementation() const override;
    virtual void SetItemData_Implementation(UDroppableItemData* Data) override;

private:
    EDropState DropState = EDropState::Scattering;

    bool bCollectDelayFinished = false;

    int32 Quantity = 1;

    bool bHasLanded = false;
    float SettleElapsed = 0.f;
    FVector SettlePosition = FVector::ZeroVector;

    float FloatTransitionElapsed = 0.f;
    float FloatStartZ = 0.f;
    float FloatTargetZ = 0.f;

    FVector FloatAnchorPosition = FVector::ZeroVector;

    // Driven explicitly each frame, prevents pitch/roll drift
    float CurrentYaw = 0.f;

    bool bIsPopping = false;
    float PopElapsed = 0.f;
    bool bPopScalingUp = true;
    FVector InitialScale = FVector::OneVector;

    UPROPERTY()
    TObjectPtr<AActor> CollectorActor;

    FTimerHandle CollectDelayTimerHandle;

    // Private Methods

    // Single consolidated reset, used by both OnSpawn and BootstrapAsFloating
    void ResetRuntimeState();

    // Kills physics cleanly in all cases if needed
    void StopPhysicsClean();

    void ApplyScatterEffect();
    void OnCollectDelayFinished();

    void TickWaitingToSettle(float DeltaTime);
    void TickTransitioningToFloat(float DeltaTime);
    void TickFloating(float DeltaTime);
    void TickBeingCollected(float DeltaTime);
    void TickPopEffect(float DeltaTime);

    UDroppableScatterSettings* GetScatterSettings() const;
    UDroppablePopSettings* GetPopSettings() const;
    UDroppableFloatSettings* GetFloatSettings() const;
    
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPhysicsHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, FVector NormalImpulse,const FHitResult& Hit);
};