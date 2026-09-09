#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FPS_WeaponTypes.h"
#include "FPS_WeaponData.generated.h"

class UCurveFloat;
class UTexture2D;

/**
 * A new weapon is a new asset of this type. Never a new class.
 *
 * If a value ever feels like it belongs on a Blueprint child of the weapon actor,
 * it belongs here instead. The weapon Blueprint exists for mesh, sockets and
 * effect wiring only.
 */
UCLASS(BlueprintType)
class MECHANICS_TEST_LVN_API UFPS_WeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Identity

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	TObjectPtr<UTexture2D> Icon;

	/** The viewmodel actor spawned and attached when this weapon is equipped. Must implement IFPS_Weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	TSubclassOf<AActor> WeaponActorClass;

	/** Spawned when this weapon is tossed, and the class a world pickup of it uses. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	TSubclassOf<AActor> PickupActorClass;

	// Ammo

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	EFPS_AmmoType AmmoType = EFPS_AmmoType::Pistol;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "1"))
	int32 MagazineSize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0"))
	int32 MaxReserveAmmo = 180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0"))
	int32 StartingReserveAmmo = 90;

	/** Rounds spent per pull. A shotgun spends one shell and sprays several pellets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "1"))
	int32 AmmoPerShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	float ReloadDuration = 1.9f;

	// Firing

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = "1"))
	float RoundsPerMinute = 620.0f;

	/** Traces per pull. This is what covers shotguns without a second weapon class. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = "1"))
	int32 PelletsPerShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing")
	float Damage = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing")
	float Range = 12000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = "0"))
	float HipSpreadDegrees = 2.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = "0"))
	float AimSpreadDegrees = 0.3f;

	// Fire modes

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Modes",
		meta = (Bitmask, BitmaskEnum = "/Script/Mechanics_Test_LVN.EFPS_FireMode"))
	int32 SupportedFireModes = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Modes")
	EFPS_FireMode DefaultFireMode = EFPS_FireMode::Semi;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Modes", meta = (ClampMin = "1"))
	int32 BurstCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Modes", meta = (ClampMin = "0.01"))
	float BurstInterval = 0.06f;

	// Poses
	//
	// Absolute transforms relative to the attach point, not offsets. An offset
	// scheme means the socket silently contributes to every weapon, so fixing one
	// breaks the rest, and an aim position of zero could never actually center a weapon.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poses")
	FTransform HipPose = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poses")
	FTransform AimPose = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poses")
	float AimFOV = 65.0f;

	UFUNCTION(BlueprintPure, Category = "FPS|Weapon Data")
	float GetFireInterval() const { return 60.0f / FMath::Max(RoundsPerMinute, 1.0f); }

	UFUNCTION(BlueprintPure, Category = "FPS|Weapon Data")
	bool SupportsFireMode(EFPS_FireMode Mode) const { return FPS_FireModeUtils::IsSupported(SupportedFireModes, Mode); }

	EFPS_FireMode ResolveInitialFireMode() const;
};
