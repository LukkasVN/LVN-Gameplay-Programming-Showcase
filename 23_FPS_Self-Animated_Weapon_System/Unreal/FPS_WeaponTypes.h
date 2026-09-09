#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "FPS_WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EFPS_AmmoType : uint8
{
	Pistol	UMETA(DisplayName = "Pistol"),
	Rifle	UMETA(DisplayName = "Rifle"),
	Shotgun	UMETA(DisplayName = "Shotgun")
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFPS_FireMode : uint8
{
	None	= 0			UMETA(Hidden),
	Semi	= 1 << 0	UMETA(DisplayName = "Semi"),
	Burst	= 1 << 1	UMETA(DisplayName = "Burst"),
	Auto	= 1 << 2	UMETA(DisplayName = "Auto")
};
ENUM_CLASS_FLAGS(EFPS_FireMode);

/** Broad movement state. Read by the viewmodel feedback and by the HUD, written only by the character. */
UENUM(BlueprintType)
enum class EFPS_MovementState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Walking		UMETA(DisplayName = "Walking"),
	Running		UMETA(DisplayName = "Running"),
	Airborne	UMETA(DisplayName = "Airborne"),
	Dashing		UMETA(DisplayName = "Dashing")
};

/**
 * Payload broadcast once per pull of the trigger.
 * Per pellet impacts are broadcast separately, this describes the shot as a whole.
 */
USTRUCT(BlueprintType)
struct FFPS_WeaponFireInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "FPS|Weapon")
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "FPS|Weapon")
	FVector Direction = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category = "FPS|Weapon")
	bool bDidHit = false;

	UPROPERTY(BlueprintReadOnly, Category = "FPS|Weapon")
	FHitResult Hit;
};

namespace FPS_FireModeUtils
{
	/** Mask test kept here so the weapon, the data asset and the HUD all agree on what supported means. */
	FORCEINLINE bool IsSupported(int32 SupportedMask, EFPS_FireMode Mode)
	{
		return Mode != EFPS_FireMode::None && (SupportedMask & static_cast<int32>(Mode)) != 0;
	}

	/** Fixed cycle order. Callers skip unsupported entries rather than reordering. */
	FORCEINLINE EFPS_FireMode NextInCycle(EFPS_FireMode Mode)
	{
		switch (Mode)
		{
		case EFPS_FireMode::Semi:	return EFPS_FireMode::Burst;
		case EFPS_FireMode::Burst:	return EFPS_FireMode::Auto;
		default:					return EFPS_FireMode::Semi;
		}
	}
}
