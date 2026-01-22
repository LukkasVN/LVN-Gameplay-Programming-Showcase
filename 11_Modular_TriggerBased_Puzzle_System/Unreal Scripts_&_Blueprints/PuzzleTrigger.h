#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleTrigger.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class APuzzleController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPuzzleTriggerEvent);

UCLASS(Blueprintable)
class MECHANICS_TEST_LVN_API APuzzleTrigger : public AActor
{
    GENERATED_BODY()

public:
    APuzzleTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    int32 ObjectsInside = 0;
    bool bIsActivated = false;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle", meta = (ExposeOnSpawn = "true"))
    FName RequiredTag = FName("PuzzleObject");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    bool bTriggerOnlyWhenEmpty = false;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Puzzle")
    APuzzleController* PuzzleController = nullptr;

    UPROPERTY(BlueprintAssignable, Category = "Puzzle")
    FPuzzleTriggerEvent OnAnyEnter;

    UPROPERTY(BlueprintAssignable, Category = "Puzzle")
    FPuzzleTriggerEvent OnAnyExit;

    UPROPERTY(BlueprintAssignable, Category = "Puzzle")
    FPuzzleTriggerEvent OnTagEnter;

    UPROPERTY(BlueprintAssignable, Category = "Puzzle")
    FPuzzleTriggerEvent OnTagExit;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle")
    UBoxComponent* BoxComponent = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    bool IsActivated() const { return bIsActivated; }
};
