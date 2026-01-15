// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueNode.h"
#include "DialogueWidget.h"
#include "Components/ActorComponent.h"
#include "DialogueComponent.generated.h"

UENUM(BlueprintType)
enum class EDialogueListType : uint8
{
	Locked,
	Base,
	Used
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueComponent();
	void ResolveDialogueWidget();

protected:
	virtual void BeginPlay() override;

public:
	
	// Dialogue lists
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasLockedBaseDialogueState = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UDialogueNode*> LockedDialogueNodes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UDialogueNode*> BaseDialogueNodes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasUsedDialogueNodesState = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UDialogueNode*> UsedDialogueNodes;

	// Callable dialogue state methods
	UFUNCTION(BlueprintCallable)
	void SetLockedDialogueState(bool state);

	UFUNCTION(BlueprintCallable)
	void SetUsedDialogueState(bool state);
	
	// Dialogue event booleans
	UPROPERTY(BlueprintReadOnly)
	bool bLockedDialogueStartBool = false;

	UPROPERTY(BlueprintReadOnly)
	bool bLockedDialogueEndBool = false;

	UPROPERTY(BlueprintReadOnly)
	bool bBaseDialogueStartBool = false;

	UPROPERTY(BlueprintReadOnly)
	bool bBaseDialogueEndBool = false;

	UPROPERTY(BlueprintReadOnly)
	bool bUsedDialogueStartBool = false;

	UPROPERTY(BlueprintReadOnly)
	bool bUsedDialogueEndBool = false;

	UPROPERTY(BlueprintReadOnly)
	EDialogueListType CurrentDialogueListType = EDialogueListType::Base;

	UFUNCTION(BlueprintCallable)
	void ResetEventBools();

	// Blueprint-Callable Dialogue interaction methods
	UFUNCTION(BlueprintCallable)
	void HandleDialogueNode();

	UFUNCTION(BlueprintCallable)
	void ForceEndDialogue();

private:
	UDialogueWidget* DialogueWidget = nullptr;

	bool bIsDialogueActive = false;
	bool bHasDialogueBeenUsed = false;
	int32 CurrentIndex = 0;

	TArray<UDialogueNode*> CurrentDialogueList;

	// Dialogue control methods
	void StartDialogue();
	void EndDialogue();
};

