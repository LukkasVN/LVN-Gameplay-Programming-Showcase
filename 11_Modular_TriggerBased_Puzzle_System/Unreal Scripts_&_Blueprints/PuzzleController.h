#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleController.generated.h"

class APuzzleTrigger;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPuzzleControllerEvent);

UCLASS()
class MECHANICS_TEST_LVN_API APuzzleController : public AActor
{
	GENERATED_BODY()

public:
	APuzzleController();

protected:
	virtual void BeginPlay() override;

	FTimerHandle CheckTimerHandle;
	void DelayedCheck();

public:
	// Add a trigger to the controller (called by triggers)
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void AddPuzzleTrigger(APuzzleTrigger* Trigger);

	// Check puzzle state (can be delayed)
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void CheckPuzzleState();

	// Immediate check method
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void CheckPuzzleStateMethod();

	// Blueprint Events
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FPuzzleControllerEvent OnPuzzleComplete;

	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FPuzzleControllerEvent OnPuzzleCancelComplete;

	// Optional delay before checking puzzle state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	float DelayBeforeCheck = 0.0f;

private:
	TArray<APuzzleTrigger*> PuzzleTriggers;
	bool bPuzzleComplete;
};
