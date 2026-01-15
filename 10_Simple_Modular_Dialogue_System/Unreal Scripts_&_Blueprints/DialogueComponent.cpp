#include "DialogueComponent.h"
#include "DialogueWidget.h"
#include "DialogueNode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "ReferencesHUD.h"

UDialogueComponent::UDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// Resolves the DialogueWidget from the HUD if not already set
void UDialogueComponent::ResolveDialogueWidget()
{
    if (DialogueWidget)
        return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
        return;

    AReferencesHUD* HUD = Cast<AReferencesHUD>(PC->GetHUD());
    if (!HUD)
        return;

    DialogueWidget = HUD->GetDialogueWidget();
}


void UDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
}

// Handles advancing the dialogue or starting it (Perfect for interaction systems)
void UDialogueComponent::HandleDialogueNode()
{
    ResolveDialogueWidget();
    if (!DialogueWidget){
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: DialogueWidget is null"));
        return;
    }

    ResetEventBools();

    // Start dialogue if not active
    if (!bIsDialogueActive)
    {
        StartDialogue();
        return;
    }

    // Advance to next node
    CurrentIndex++;

    // End if finished
    if (CurrentIndex >= CurrentDialogueList.Num())
    {
        EndDialogue();
        return;
    }
    
    // Update UI
    DialogueWidget->UpdateDialogue(CurrentDialogueList[CurrentIndex]);
}

void UDialogueComponent::StartDialogue()
{
    bIsDialogueActive = true;

    // Pick dialogue list to select proper node list
    if (bHasLockedBaseDialogueState)
    {
        CurrentDialogueList = LockedDialogueNodes;
        CurrentDialogueListType = EDialogueListType::Locked;
    }
    else
    {
        if (bHasDialogueBeenUsed && bHasUsedDialogueNodesState)
        {
            CurrentDialogueList = UsedDialogueNodes;
            CurrentDialogueListType = EDialogueListType::Used;
        }
        else
        {
            CurrentDialogueList = BaseDialogueNodes;
            bHasDialogueBeenUsed = true;
            CurrentDialogueListType = EDialogueListType::Base;
        }
    }

    if (CurrentDialogueList.Num() == 0)
    {
        EndDialogue();
        return;
    }

    CurrentIndex = 0;

    // Show UI and update first node
    DialogueWidget->ShowDialogue();

    // Trigger start event bools
    switch (CurrentDialogueListType)
    {
    case EDialogueListType::Locked:
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Starting Locked Dialogue"));
        bLockedDialogueStartBool = true;
        break;
    case EDialogueListType::Base:
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Starting Base Dialogue"));
        bBaseDialogueStartBool = true;
        break;
    case EDialogueListType::Used:
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Starting Used Dialogue"));
        bUsedDialogueStartBool = true;
        break;
    default: break;
    }

    // Update UI with first node
    DialogueWidget->UpdateDialogue(CurrentDialogueList[0]);
}

void UDialogueComponent::EndDialogue()
{
    // Trigger end event bools
    switch (CurrentDialogueListType)
    {
        case EDialogueListType::Locked:
            UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Ended Locked Dialogue"));
            bLockedDialogueEndBool = true;
        break;
        case EDialogueListType::Base:
            UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Ended Base Dialogue"));
            bBaseDialogueEndBool = true;
        break;
        case EDialogueListType::Used:
            UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Ended Used Dialogue"));
            bUsedDialogueEndBool = true;
        break;
        default: break;
    }
    
    bIsDialogueActive = false;
    CurrentDialogueList.Empty();
    CurrentIndex = 0;
    
    if (DialogueWidget)
    {
        DialogueWidget->HideDialogue();
    }
}

// Immediately ends the dialogue, useful for interruptions
void UDialogueComponent::ForceEndDialogue()
{
    if (!bIsDialogueActive)
        return;

    bIsDialogueActive = false;
    CurrentDialogueList.Empty();
    CurrentIndex = 0;

    if (DialogueWidget)
    {
        DialogueWidget->HideDialogue();
    }
}

// Sets the locked dialogue state (If true locks the player in the locked dialogue nodes)
void UDialogueComponent::SetLockedDialogueState(bool state)
{
    bHasLockedBaseDialogueState = state;
}

// Sets the used dialogue state (If true allows the used dialogue nodes to be used after base dialogue [if any] has been completed)
void UDialogueComponent::SetUsedDialogueState(bool state)
{
    bHasUsedDialogueNodesState = state;
}

// Resets all event booleans to false
void UDialogueComponent::ResetEventBools()
{
    bBaseDialogueStartBool = false;
    bBaseDialogueEndBool = false;
    bLockedDialogueStartBool = false;
    bLockedDialogueEndBool = false;
    bUsedDialogueStartBool = false;
    bUsedDialogueEndBool = false;
}

