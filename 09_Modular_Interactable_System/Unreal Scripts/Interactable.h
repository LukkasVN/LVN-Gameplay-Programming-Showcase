#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"

UCLASS()
class MECHANICS_TEST_LVN_API AInteractable : public AActor
{
    GENERATED_BODY()

public:
    AInteractable();

    UPROPERTY(EditAnywhere, Category="Interactable")
    bool bIsInteractable = true;
    // Click‑based interaction (line trace)
    UPROPERTY(EditAnywhere, Category="Interactable")
    bool bIsClickBased = false;

    // Has max distance for click‑based interaction
    UPROPERTY(EditAnywhere, Category="Interactable")
    bool bHasClickDistance  = false;
    
    // Max distance for click‑based interaction
    UPROPERTY(EditAnywhere, Category="Interactable")
    float ClickDistance = 500.f;

    // Trigger‑based interaction (overlap)
    UPROPERTY(EditAnywhere, Category="Interactable")
    bool bIsTriggerBased = true;

    UPROPERTY(EditAnywhere, Category="Interactable")
    FName TriggerTag = "Player";

    // Interaction cooldown
    UPROPERTY(EditAnywhere, Category="Interactable")
    bool bHasInteractCooldown = false;

    UPROPERTY(EditAnywhere, Category="Interactable")
    float InteractCooldownDuration = 1.f;

    // Hover events (enter / stay / exit)
    UPROPERTY(EditAnywhere, Category="Interactable")
    bool bHasHoverEvents = true;
    
    // Hover Cooldowns
    UPROPERTY(EditAnywhere, Category="Interactable")
    bool bHasHoverEnterCooldown = false;

    UPROPERTY(EditAnywhere, Category="Interactable")
    float HoverEnterCooldownDuration = 1.f;

    UPROPERTY(EditAnywhere, Category="Interactable")
    bool bHasHoverStayCooldown = false;

    UPROPERTY(EditAnywhere, Category="Interactable")
    float HoverStayCooldownDuration = 1.f;

    UPROPERTY(EditAnywhere, Category="Interactable")
    bool bHasHoverExitCooldown = false;

    UPROPERTY(EditAnywhere, Category="Interactable")
    float HoverExitCooldownDuration = 1.f;

    // Runtime flags
    bool bIsOnHoverEnterCooldown = false;
    bool bIsOnHoverStayCooldown = false;
    bool bIsOnHoverExitCooldown = false;

    // Timer handles
    FTimerHandle HoverEnterCooldownTimer;
    FTimerHandle HoverStayCooldownTimer;
    FTimerHandle HoverExitCooldownTimer;

    void Interact();
    void SetInteractable(bool state);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Trigger volume for trigger‑based interaction
    UPROPERTY(VisibleAnywhere)
    class USphereComponent* Trigger;
    // Optional mesh for visual representation

    class UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, Category="Input")
    class UInputMappingContext* InteractMappingContext;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input");
    UInputAction *InteractAction;

    bool bCanInteract = false;
    bool bIsHovered = false;
    bool bIsInsideTrigger = false;
    bool bIsOnInteractCooldown = false;

    FTimerHandle InteractCooldownTimer;

    // Called when interaction occurs
    virtual void OnInteract();

    // Hover events
    virtual void OnHoverEnter();
    virtual void OnHoverStay();
    virtual void OnHoverExit();

    void HandleHover(bool bHovering);

    // Cooldown management
    void StartInteractCooldown();
    void EndInteractCooldown();

    UFUNCTION()
    void OnTriggerEnter(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnTriggerExit(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );
    
    void EnablePhysicsMode(UStaticMeshComponent* Mesh);
    void StartHoverEnterCooldown();
    void StartHoverStayCooldown();
    void StartHoverExitCooldown();
};
