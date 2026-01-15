// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueNode.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MECHANICS_TEST_LVN_API UDialogueNode : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Portrait;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Headline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true))
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TextDelay = 0.02f;
};

