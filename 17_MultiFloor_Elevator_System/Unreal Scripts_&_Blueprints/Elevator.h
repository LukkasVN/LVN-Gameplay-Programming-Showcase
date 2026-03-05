#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Curves/CurveFloat.h"
#include "Elevator.generated.h"

UENUM(BlueprintType)
enum class EElevatorState : uint8
{
    Idle,
    ClosingDoors,
    Moving,
    Rotating,
    OpeningDoors
};

UCLASS()
class MECHANICS_TEST_LVN_API AElevator : public AActor
{
    GENERATED_BODY()

public:
    AElevator();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floors")
    TArray<AActor*> Floors;

    // Assign your Door Mesh Components to these in the Blueprint's EventGraph
    UPROPERTY(BlueprintReadWrite, Category="Doors")
    class USceneComponent* DoorLeft;

    UPROPERTY(BlueprintReadWrite, Category="Doors")
    class USceneComponent* DoorRight;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    class UBoxComponent* ElevatorTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
    EElevatorState State = EElevatorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float TravelTime = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    UCurveFloat* MovementCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float ArrivalRotationTime = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    UCurveFloat* RotationCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Doors")
    bool bUseDoors = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Doors")
    float DoorOpenDistance = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Doors")
    float DoorSpeed = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Doors")
    float CloseDoorCooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Occupants")
    TArray<FName> MainOccupantTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floors")
    int32 InitialFloor = 0;

private:
    int32 CurrentFloor = 0;
    int32 TargetFloor = 0;
    TArray<int32> FloorQueue;

    FVector StartPos; FVector EndPos;
    FQuat StartRot; FQuat EndRot;

    FVector LeftClosedPos; FVector RightClosedPos;
    FVector LeftOpenPos; FVector RightOpenPos;

    int32 MainOccupantCount = 0;
    float MoveTimer = 0.f;
    float RotationTimer = 0.f;

    FTimerHandle AutoMoveTimerHandle;
    FTimerHandle DoorAnimationTimer;

    bool bAnimatingDoorsForward = false;
    FVector DoorLTarget; FVector DoorRTarget;

    bool bWasCalledExternally;

    void MovementTick(float DeltaTime);
    void RotationTick(float DeltaTime);
    void StartNextSegment();
    void FinishMovement();
    
    void AnimateDoors(bool bOpening);
    void DoorAnimationTick();
    void OpenDoors();
    void CloseDoors();

public:
    UFUNCTION(BlueprintCallable)
    void CallElevator(int32 FloorIndex);

    UFUNCTION(BlueprintCallable)
    void MoveToFloor(int32 FloorIndex);

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};