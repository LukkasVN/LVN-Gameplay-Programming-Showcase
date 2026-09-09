#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPS_WeaponTypes.h"
#include "FPS_Weapon.generated.h"

class UFPS_WeaponData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFPS_OnWeaponAmmoChanged, int32, MagazineAmmo, int32, ReserveAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFPS_OnWeaponFired, const FFPS_WeaponFireInfo&, FireInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFPS_OnWeaponFireModeChanged, EFPS_FireMode, NewFireMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFPS_OnWeaponImpact, FVector, ImpactPoint, FVector, ImpactNormal);

UINTERFACE(MinimalAPI, BlueprintType)
class UFPS_Weapon : public UInterface
{
	GENERATED_BODY()
};

/**
 * Everything the holder, the HUD, the pickups and the viewmodel feedback are
 * allowed to know about a weapon. None of them ever see a concrete weapon type.
 */
class MECHANICS_TEST_LVN_API IFPS_Weapon
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	UFPS_WeaponData* GetWeaponData();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	int32 GetMagazineAmmo();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	int32 GetReserveAmmo();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	EFPS_FireMode GetCurrentFireMode();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	bool IsReloading();

	/**
	 * Negative magazine or reserve means no snapshot was supplied and the weapon
	 * should fall back to its data asset defaults. 
	 * That is the world pickup case, as opposed to picking up a weapon that was tossed with ammo already spent.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	void InitializeWeapon(UFPS_WeaponData* InData, int32 InMagazineAmmo, int32 InReserveAmmo);

	/**
	 * Parameterless, and called every frame the fire input is held.
	 * The press versus hold edge is derived inside the weapon. Deriving it in the
	 * holder would force the holder to know which weapons are semi automatic,
	 * which drags fire mode logic out of the data asset and into the player.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	void Fire();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	void Reload();

	/** Drops any in flight burst or reload and returns to idle. Called before a toss. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	void CancelActions();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	void AimIn();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	void AimOut();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	void CycleFireMode();

	/** Returns how much was actually taken, so a pickup can tell full from partial from rejected. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Weapon")
	int32 AddReserveAmmo(int32 Amount);

	virtual FFPS_OnWeaponAmmoChanged& GetOnAmmoChanged() = 0;
	virtual FFPS_OnWeaponFired& GetOnFired() = 0;
	virtual FFPS_OnWeaponFireModeChanged& GetOnFireModeChanged() = 0;
};
