// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidget.generated.h"

/**
 * 
 */
UCLASS()
class MECHANICS_TEST_LVN_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

	public:
	
	// Called when C++ wants to update the UI with a new dialogue node
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateDialogue(const UDialogueNode* Node);

	// Called when dialogue starts
	UFUNCTION(BlueprintImplementableEvent)
	void ShowDialogue();

	// Called when dialogue ends
	UFUNCTION(BlueprintImplementableEvent)
	void HideDialogue();
	
};
