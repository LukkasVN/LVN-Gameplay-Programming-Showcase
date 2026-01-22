#include "PuzzleTrigger.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "PuzzleController.h" // forward declared in header
#include "Components/SceneComponent.h"

APuzzleTrigger::APuzzleTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    BoxComponent->SetupAttachment(RootComponent);
    BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BoxComponent->SetGenerateOverlapEvents(true);

    ObjectsInside = 0;
    bIsActivated = false;
}


void APuzzleTrigger::BeginPlay()
{
    Super::BeginPlay();

    BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &APuzzleTrigger::OnOverlapBegin);
    BoxComponent->OnComponentEndOverlap.AddDynamic(this, &APuzzleTrigger::OnOverlapEnd);

    if (PuzzleController != nullptr)
    {
        PuzzleController->AddPuzzleTrigger(this);
    }
}

void APuzzleTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    if (bTriggerOnlyWhenEmpty)
    {
        ObjectsInside++;
        if (ObjectsInside > 1)
        {
            return; // do not trigger if more than one object is inside
        }
    }

    OnAnyEnter.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Object entered PuzzleTrigger: %s"), *OtherActor->GetName());

    if (!bIsActivated && OtherActor->ActorHasTag(RequiredTag))
    {
        bIsActivated = true;
        OnTagEnter.Broadcast();

        if (PuzzleController)
        {
            PuzzleController->CheckPuzzleState();
        }

        UE_LOG(LogTemp, Log, TEXT("PuzzleTrigger activated by %s"), *OtherActor->GetName());
    }
}

void APuzzleTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this) return;

    if (bTriggerOnlyWhenEmpty)
    {
        ObjectsInside--;
        if (ObjectsInside <= 0)
        {
            ObjectsInside = 0;
        }
        else
        {
            return; // still objects inside, do not trigger exit logic
        }
    }

    OnAnyExit.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Object exited PuzzleTrigger: %s"), *OtherActor->GetName());

    if (bIsActivated && OtherActor->ActorHasTag(RequiredTag))
    {
        bIsActivated = false;
        OnTagExit.Broadcast();

        if (PuzzleController)
        {
            PuzzleController->CheckPuzzleState();
        }

        UE_LOG(LogTemp, Log, TEXT("PuzzleTrigger deactivated by %s"), *OtherActor->GetName());
    }
}
