// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueWidget.h"
#include "GameFramework/HUD.h"
#include "ReferencesHUD.generated.h"

/**
 * 
 */
UCLASS()
class MECHANICS_TEST_LVN_API AReferencesHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UDialogueWidget* DialogueWidget;

	// References the DialogueWidget instance
	UFUNCTION(BlueprintCallable)
	void ReferenceWidget(UDialogueWidget* Widget);

	// Getter for DialogueWidget
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UDialogueWidget* GetDialogueWidget() const { return DialogueWidget; }

};
