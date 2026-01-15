#include "Interactable.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"

AInteractable::AInteractable()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // Trigger
    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
    Trigger->SetupAttachment(RootComponent);
    Trigger->InitSphereRadius(150.f);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    Trigger->OnComponentBeginOverlap.AddDynamic(this, &AInteractable::OnTriggerEnter);
    Trigger->OnComponentEndOverlap.AddDynamic(this, &AInteractable::OnTriggerExit);

}

void AInteractable::BeginPlay()
{
    Super::BeginPlay();
    
    Mesh = FindComponentByClass<UStaticMeshComponent>();
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    EnableInput(PC);

    if (ULocalPlayer* LP = PC->GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Subsystem->AddMappingContext(InteractMappingContext, 1);
        }
    }

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EIC->BindAction(InteractAction, ETriggerEvent::Completed, this, &AInteractable::Interact);
    }

    if (bIsClickBased) bCanInteract = true;

    // Optional: make trigger physically inert when not used
    if (!bIsTriggerBased && Trigger) {
        Trigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AInteractable::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bHasHoverEvents || !bIsInteractable)
        return;

    if (bIsTriggerBased && bIsOnInteractCooldown)
    {
        HandleHover(false);
        return;
    }

    bool bHovering = false;

    // Click-based hover
    if (bIsClickBased)
    {
        if (bIsOnInteractCooldown)
        {
            HandleHover(false);
            return;
        }

        APlayerController* PC = GetWorld()->GetFirstPlayerController();

        if (!PC)
        {
            HandleHover(false);
            return;
        }

        FVector WorldPos, WorldDir;
        if (PC->bShowMouseCursor && PC->DeprojectMousePositionToWorld(WorldPos, WorldDir))
        {
            const float Distance = bHasClickDistance ? ClickDistance : 999999.f;
            FVector End = WorldPos + WorldDir * Distance;

            // Debug ray
            //DrawDebugLine(GetWorld(), WorldPos, End, FColor::Green, false, 0.05f, 0, 1.f); // Ray direction

            /*UE_LOG(LogTemp, Warning, TEXT("Ray Origin: %s | Dir: %s"),
                *WorldPos.ToString(),
                *WorldDir.ToString());
            */
            
            FHitResult Hit;
            FCollisionQueryParams Params;
            Params.bTraceComplex = false;

            if (GetWorld()->LineTraceSingleByChannel(Hit, WorldPos, End, ECC_Visibility, Params))
            {
                // Debug hit point
                // DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 12.f, 12, FColor::Blue, false, 0.05f);

                /*
                UE_LOG(LogTemp, Warning, TEXT("Ray Hit Actor: %s"), 
                    Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"));
                */
                if (Hit.GetActor() == this)
                    bHovering = true;
            }
            else
            {
                //UE_LOG(LogTemp, Warning, TEXT("Raycast: No Hit"));
            }
        }
        else
        {
            
            //UE_LOG(LogTemp, Warning, TEXT("DeprojectMousePositionToWorld FAILED"));
        }
    }


    // Trigger-based hover
    if (bIsTriggerBased)
    {
        bHovering = bIsInsideTrigger;
    }

    // Final hover handling
    HandleHover(bHovering);
}

void AInteractable::HandleHover(bool bHovering)
{
    if (!bHasHoverEvents || !bIsInteractable)
        return;

    // No hover while interaction cooldown
    if (bIsTriggerBased && bIsOnInteractCooldown)
        return;

    // Hover Enter
    if (bHovering && !bIsHovered)
    {
        if (!bIsOnHoverEnterCooldown)
        {
            bIsHovered = true;
            OnHoverEnter();

            if (bHasHoverEnterCooldown)
                StartHoverEnterCooldown();
        }
        return;
    }

    // Hover Stay
    if (bHovering && bIsHovered)
    {
        if (!bIsOnHoverStayCooldown)
        {
            OnHoverStay();

            if (bHasHoverStayCooldown)
                StartHoverStayCooldown();
        }
        return;
    }

    // Hover Exit
    if (!bHovering && bIsHovered)
    {
        if (!bIsOnHoverExitCooldown)
        {
            bIsHovered = false;
            OnHoverExit();

            if (bHasHoverExitCooldown)
                StartHoverExitCooldown();
        }
    }

}

void AInteractable::Interact()
{
    if (!bCanInteract || bIsOnInteractCooldown)
        return;
    if (bIsClickBased && !bIsHovered)
        return;

    UE_LOG(LogTemp, Warning, TEXT("Interacted with %s"), *GetName());
    OnInteract();

    if (bHasInteractCooldown)
        StartInteractCooldown();
}

void AInteractable::StartInteractCooldown()
{
    bIsOnInteractCooldown = true;

    GetWorld()->GetTimerManager().SetTimer(
        InteractCooldownTimer,
        this,
        &AInteractable::EndInteractCooldown,
        InteractCooldownDuration,
        false
    );
}

void AInteractable::EndInteractCooldown()
{
    bIsOnInteractCooldown = false;

    if (bIsTriggerBased && bIsInsideTrigger)
        bCanInteract = true;
}

void AInteractable::SetInteractable(bool bState)
{
    bIsInteractable = bState;

    if (!bIsInteractable)
    {
        bCanInteract = false;
        bIsHovered = false;
        bIsInsideTrigger = false;

        bIsOnInteractCooldown = false;
        bIsOnHoverEnterCooldown = false;
        bIsOnHoverStayCooldown = false;
        bIsOnHoverExitCooldown = false;

        GetWorld()->GetTimerManager().ClearTimer(InteractCooldownTimer);
        GetWorld()->GetTimerManager().ClearTimer(HoverEnterCooldownTimer);
        GetWorld()->GetTimerManager().ClearTimer(HoverStayCooldownTimer);
        GetWorld()->GetTimerManager().ClearTimer(HoverExitCooldownTimer);
    }
}


void AInteractable::OnTriggerEnter(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (!bIsTriggerBased || !OtherActor->ActorHasTag(TriggerTag))
        return;

    if (OtherActor && OtherActor != this)
    {
        bIsInsideTrigger = true;
        bCanInteract = !bIsOnInteractCooldown;
    }
}

void AInteractable::OnTriggerExit(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    if (!bIsTriggerBased || !OtherActor->ActorHasTag(TriggerTag))
        return;

    if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(TriggerTag))
    {
        bIsInsideTrigger = false;
        bCanInteract = false;
    }
}

void AInteractable::EnablePhysicsMode(UStaticMeshComponent* InteractableMesh)
{
    if (!InteractableMesh || !Trigger) return;

    InteractableMesh->SetSimulatePhysics(true);
    InteractableMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    Trigger->AttachToComponent(InteractableMesh, FAttachmentTransformRules::KeepWorldTransform);
    Trigger->SetSimulatePhysics(false);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AInteractable::StartHoverEnterCooldown()
{
    bIsOnHoverEnterCooldown = true;

    GetWorld()->GetTimerManager().SetTimer(
        HoverEnterCooldownTimer,
        [this]()
        {
            bIsOnHoverEnterCooldown = false;
        },
        HoverEnterCooldownDuration,
        false
    );
}

void AInteractable::StartHoverStayCooldown()
{
    bIsOnHoverStayCooldown = true;

    GetWorld()->GetTimerManager().SetTimer(
        HoverStayCooldownTimer,
        [this]()
        {
            bIsOnHoverStayCooldown = false;
        },
        HoverStayCooldownDuration,
        false
    );
}

void AInteractable::StartHoverExitCooldown()
{
    bIsOnHoverExitCooldown = true;

    GetWorld()->GetTimerManager().SetTimer(
        HoverExitCooldownTimer,
        [this]()
        {
            bIsOnHoverExitCooldown = false;
        },
        HoverExitCooldownDuration,
        false
    );
}



void AInteractable::OnInteract() {}
void AInteractable::OnHoverEnter() {}
void AInteractable::OnHoverStay() {}
void AInteractable::OnHoverExit() {}
