#include "FlashlightComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"

UFlashlightComponent::UFlashlightComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UFlashlightComponent::ToggleFlashlight()
{
    if (!Light)
        return;

    if (!bIsOn && CurrentLifetime <= 0.f)
        return;

    bIsOn = !bIsOn;

    Light->SetVisibility(bIsOn);
    Light->SetIntensity(bIsOn ? LightIntensity : 0.f);
}

void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    bRechargeUsedThisFrame = false;

    // Battery drain only when ON
    if (bIsOn)
    {
        CurrentLifetime -= DeltaTime;
        CurrentLifetime = FMath::Clamp(CurrentLifetime, 0.f, MaxLifetime);

        if (CurrentLifetime <= 0.f)
        {
            bIsOn = false;
            Light->SetVisibility(false);
            Light->SetIntensity(0.f);
        }
    }

    // Gradual recharge
    if (bIsRecharging)
    {
        RechargeGradualTick(DeltaTime);
    }

    // Flicker effect
    FlickerTick(DeltaTime);

    // Lag effect (applied to Pivot)
    LagTick(DeltaTime);

    // Debug UI
    if (GEngine)
    {
        float Percent = GetBatteryPercent() * 100.f;
        FString BatteryText = FString::Printf(TEXT("FLASHLIGHT BATTERY: %.0f%%"), Percent);

        FColor DebugColor;

        if (CurrentLifetime <= LowBatteryThresholdSeconds)
            DebugColor = FColor::Red;
        else if (bIsOn)
            DebugColor = FColor::Yellow;
        else
            DebugColor = FColor::White;

        GEngine->AddOnScreenDebugMessage(
            1,
            0.f,
            DebugColor,
            BatteryText
        );
    }
}

void UFlashlightComponent::InitializeFlashlight(float Intensity, float Radius, float Inner, float Outer)
{
    Light->SetIntensity(Intensity);
    LightIntensity = Intensity;
    Light->SetAttenuationRadius(Radius);
    Light->SetInnerConeAngle(Inner);
    Light->SetOuterConeAngle(Outer);
}

void UFlashlightComponent::InitializeBattery(float InMaxLifetime, float InLowBatteryPercent)
{
    MaxLifetime = InMaxLifetime;
    CurrentLifetime = MaxLifetime;

    LowBatteryThresholdSeconds = MaxLifetime * (InLowBatteryPercent / 100.f);
}

void UFlashlightComponent::RechargeInstant(float AmountPercent)
{
    if (bRechargeUsedThisFrame || AmountPercent <= 0.f)
        return;

    // Convert percent to seconds
    float AmountSeconds = MaxLifetime * (AmountPercent / 100.f);

    CurrentLifetime += AmountSeconds;
    CurrentLifetime = FMath::Clamp(CurrentLifetime, 0.f, MaxLifetime);

    bRechargeUsedThisFrame = true;
}

void UFlashlightComponent::StartRechargeGradual(float AmountPercent)
{
    if (AmountPercent <= 0.f || FullRechargeTimeSeconds <= 0.f)
        return;

    float AmountSeconds = MaxLifetime * (AmountPercent / 100.f);

    RechargeRatePerSecond = AmountSeconds / FullRechargeTimeSeconds;

    RechargeTargetAmount = AmountSeconds;
    RechargeAccumulated = 0.f;

    bIsRecharging = true;
}

void UFlashlightComponent::RechargeGradualTick(float DeltaTime)
{
    if (!bIsRecharging)
        return;

    float DeltaRecharge = RechargeRatePerSecond * DeltaTime;

    CurrentLifetime += DeltaRecharge;
    CurrentLifetime = FMath::Clamp(CurrentLifetime, 0.f, MaxLifetime);

    RechargeAccumulated += DeltaRecharge;

    if (RechargeAccumulated >= RechargeTargetAmount || CurrentLifetime >= MaxLifetime)
    {
        bIsRecharging = false;
    }
}

void UFlashlightComponent::FlickerTick(float DeltaTime)
{
    if (!bIsOn)
        return;

    if (CurrentLifetime > LowBatteryThresholdSeconds)
    {
        Light->SetIntensity(LightIntensity);
        return;
    }

    FlickerTimer -= DeltaTime;

    if (FlickerTimer <= 0.f)
    {
        FlickerTimer = FlickerInterval;

        float RandomFactor = FMath::FRandRange(FlickerMinIntensity, FlickerMaxIntensity);

        Light->SetIntensity(LightIntensity * RandomFactor);
    }
}

void UFlashlightComponent::LagTick(float DeltaTime)
{
    if (!Pivot)
        return;

    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>();
    if (!Camera)
        return;

    TargetRotation = Camera->GetComponentRotation();
    TargetRotation += FRotator(10.f, 0.f, 0.f); // Slight upward offset for better visibility

    FRotator NewRotation = FMath::RInterpTo(
        Pivot->GetComponentRotation(),
        TargetRotation,
        DeltaTime,
        LagSpeed
    );

    Pivot->SetWorldRotation(NewRotation);
}

