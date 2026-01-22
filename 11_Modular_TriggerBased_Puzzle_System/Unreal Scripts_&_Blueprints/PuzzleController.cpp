#include "PuzzleController.h"
#include "PuzzleTrigger.h"
#include "TimerManager.h"
#include "Engine/World.h"

APuzzleController::APuzzleController()
{
    PrimaryActorTick.bCanEverTick = false;
    bPuzzleComplete = false;
    DelayBeforeCheck = 0.0f;
}

void APuzzleController::BeginPlay()
{
    Super::BeginPlay();
}

void APuzzleController::AddPuzzleTrigger(APuzzleTrigger* Trigger)
{
    if (!Trigger) return;
    if (!PuzzleTriggers.Contains(Trigger))
    {
        PuzzleTriggers.Add(Trigger);
    }
}

void APuzzleController::CheckPuzzleState()
{
    if (!GetWorld()) return;

    bool bAllActivated = true;
    for (APuzzleTrigger* T : PuzzleTriggers)
    {
        if (!T || !T->IsActivated())
        {
            bAllActivated = false;
            break;
        }
    }

    if (bAllActivated && !bPuzzleComplete)
    {
        GetWorld()->GetTimerManager().ClearTimer(CheckTimerHandle);
        if (DelayBeforeCheck > 0.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(CheckTimerHandle, this, &APuzzleController::DelayedCheck, DelayBeforeCheck, false);
        }
        else
        {
            CheckPuzzleStateMethod();
        }
    }
    else if (!bAllActivated && bPuzzleComplete)
    {
        GetWorld()->GetTimerManager().ClearTimer(CheckTimerHandle);
        CheckPuzzleStateMethod();
    }
}

void APuzzleController::DelayedCheck()
{
    CheckPuzzleStateMethod();
}

void APuzzleController::CheckPuzzleStateMethod()
{
    bool bAllActivated = true;

    for (APuzzleTrigger* T : PuzzleTriggers)
    {
        if (!T || !T->IsActivated())
        {
            bAllActivated = false;
            break;
        }
    }

    if (bAllActivated && !bPuzzleComplete)
    {
        bPuzzleComplete = true;
        OnPuzzleComplete.Broadcast();
    }
    else if (!bAllActivated && bPuzzleComplete)
    {
        bPuzzleComplete = false;
        OnPuzzleCancelComplete.Broadcast();
    }
}
