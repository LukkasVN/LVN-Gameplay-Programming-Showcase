#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "FP_Character.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;

UCLASS(Blueprintable)
class MECHANICS_TEST_LVN_API AFP_Character : public ACharacter
{
    GENERATED_BODY()

public:
    AFP_Character();

    // Ability toggles (Unity-style)
    UFUNCTION(BlueprintCallable)
    void SetCanMove(bool bEnabled);

    UFUNCTION(BlueprintCallable)
    void SetCanLook(bool bEnabled);

    UFUNCTION(BlueprintCallable)
    void SetCanInteract(bool bEnabled);

    UFUNCTION(BlueprintCallable)
    void SetCanJump(bool bEnabled);

    UFUNCTION(BlueprintCallable)
    void SetCanSprint(bool bEnabled);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Input
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputMappingContext* InputMapping;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* RunAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* JumpAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* InteractAction;

    // Input handlers
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void HandleJump();
    void StartSprint();
    void StopSprint();
    void InteractPressed();

    // Camera
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    UCameraComponent* PlayerCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
    float DefaultFOV = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
    float SprintFOV = 110.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
    float FOVInterpSpeed = 8.f;

    // Look
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Look")
    float HorizontalSensitivity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Look")
    float VerticalSensitivity = 1.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Look")
    float PitchClamp = 85.f;

    float Pitch = 0.f;

    // Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float WalkSpeed = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float RunSpeed = 700.f;

    bool bIsRunning = false;

    // Jumping
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jumping")
    float GravityScale = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jumping")
    float AirControl = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jumping")
    bool bAllowDoubleJump = true;

    bool bWasGrounded = false;
    bool bUsedGroundJump = false;
    int32 JumpCount = 0;

    // Interaction
    AActor* CurrentInteractable = nullptr;
    AActor* TraceInteractable(float Distance);

    // Ability flags
    bool bCanMove = true;
    bool bCanLook = true;
    bool bCanInteract = true;
    bool bCanJump = true;
    bool bCanSprint = true;

    // Bobbing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bobbing")
    bool bHasBobbing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bobbing")
    float WalkBobFrequency = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bobbing")
    float WalkBobAmplitude = 6.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bobbing")
    float RunBobFrequency = 18.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bobbing")
    float RunBobAmplitude = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bobbing")
    float IdleBreathFrequency = 2.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bobbing")
    float IdleBreathAmplitude = 5.f;

    FVector CameraBaseOffset;

    // Landing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Landing")
    bool bHasLandingEffect = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Landing")
    float LandingDipAmount = -6.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Landing")
    float LandingDipSpeed = 6.f;

    float LandingLerp = 0.f;

    void ApplyBobbing(float DeltaTime);
};
