#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightComponent.generated.h"

class USpotLightComponent;
class USceneComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API UFlashlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlashlightComponent();

	UPROPERTY()
	USceneComponent* Pivot;

	UPROPERTY()
	USpotLightComponent* Light;

	float LightIntensity;

	float MaxLifetime; // Total battery life in seconds
	float CurrentLifetime;

	float LowBatteryThresholdSeconds;

	// Initialization
	void InitializeFlashlight(float Intensity, float Radius, float Inner, float Outer);
	void InitializeBattery(float InMaxLifetime, float InLowBatteryThreshold);

	// Core actions
	UFUNCTION(BlueprintCallable, Category="Flashlight")
	void ToggleFlashlight();

	UFUNCTION(BlueprintCallable, Category="Flashlight")
	void RechargeInstant(float AmountPercent);

	UFUNCTION(BlueprintCallable, Category="Flashlight")
	void StartRechargeGradual(float AmountPercent);

	void RechargeGradualTick(float DeltaTime);
	
	UPROPERTY()
	float FullRechargeTimeSeconds = 1.f; // Time it takes to fully recharge from empty (for gradual recharge)

	// Flicker settings
	float FlickerTimer = 0.f;
	float FlickerInterval = 0.05f; // how often flicker updates
	float FlickerMinIntensity = 0.2f; // 20% of normal
	float FlickerMaxIntensity = 1.0f; // 100% of normal

	void FlickerTick(float DeltaTime);

	// Camera Lag System
	float LagSpeed = 10.f;
	FRotator TargetRotation;

	void LagTick(float DeltaTime);
	
	// Accessor for UI
	UFUNCTION(BlueprintPure, Category="Flashlight")
	float GetBatteryPercent() const { return CurrentLifetime / MaxLifetime; }


private:
	bool bIsOn = false;
	bool bIsRecharging = false;
	bool bRechargeUsedThisFrame = false;
	float RechargeTargetAmount = 0.f;
	float RechargeRatePerSecond = 0.f;
	float RechargeAccumulated = 0.f;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

