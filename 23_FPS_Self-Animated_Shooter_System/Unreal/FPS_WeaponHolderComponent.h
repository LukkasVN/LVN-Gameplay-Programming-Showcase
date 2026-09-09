#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FPS_Weapon.h"
#include "FPS_WeaponTypes.h"
#include "FPS_WeaponHolderComponent.generated.h"

class AFPS_Character;
class UCurveFloat;
class UFPS_WeaponData;
class UInputAction;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFPS_OnActiveWeaponChanged, UFPS_WeaponData*, WeaponData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFPS_OnAmmoRejected, EFPS_AmmoType, RejectedType);

UCLASS(ClassGroup = (FPS), meta = (BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API UFPS_WeaponHolderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFPS_WeaponHolderComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Integration points

	UPROPERTY(BlueprintAssignable, Category = "FPS|Holder|Events")
	FFPS_OnWeaponAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "FPS|Holder|Events")
	FFPS_OnActiveWeaponChanged OnActiveWeaponChanged;

	UPROPERTY(BlueprintAssignable, Category = "FPS|Holder|Events")
	FFPS_OnWeaponFired OnWeaponFired;

	UPROPERTY(BlueprintAssignable, Category = "FPS|Holder|Events")
	FFPS_OnWeaponFireModeChanged OnFireModeChanged;

	/** Fires when a pickup could not be taken, so the HUD can flash instead of silently doing nothing. */
	UPROPERTY(BlueprintAssignable, Category = "FPS|Holder|Events")
	FFPS_OnAmmoRejected OnAmmoRejected;

	UFUNCTION(BlueprintCallable, Category = "FPS|Holder")
	void EquipWeapon(UFPS_WeaponData* Data, int32 MagazineAmmo = -1, int32 ReserveAmmo = -1);

	/** Tosses whatever is held to make room, so a pickup is never refused. */
	UFUNCTION(BlueprintCallable, Category = "FPS|Holder")
	void TryPickUpWeapon(UFPS_WeaponData* Data, int32 MagazineAmmo = -1, int32 ReserveAmmo = -1);

	UFUNCTION(BlueprintCallable, Category = "FPS|Holder")
	void TossWeapon();

	/** False when the caliber does not match or the reserve is capped. The pickup then stays in the world. */
	UFUNCTION(BlueprintCallable, Category = "FPS|Holder")
	bool AddAmmo(EFPS_AmmoType AmmoType, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "FPS|Holder")
	bool HasWeapon() const { return ActiveWeapon.GetObject() != nullptr; }

	UFUNCTION(BlueprintPure, Category = "FPS|Holder")
	AActor* GetActiveWeaponActor() const { return Cast<AActor>(ActiveWeapon.GetObject()); }

	UFUNCTION(BlueprintPure, Category = "FPS|Holder")
	UFPS_WeaponData* GetActiveWeaponData() const;

	/** Read by the viewmodel feedback to damp everything except the reload pose. */
	UFUNCTION(BlueprintPure, Category = "FPS|Holder")
	float GetAimAlpha() const { return AimAlpha; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Tag on the scene component the weapon is attached to. Leave that component at zero, poses are absolute. */
	UPROPERTY(EditAnywhere, Category = "FPS|Holder")
	FName AttachPointTag = TEXT("WeaponAttachPoint");

	UPROPERTY(EditAnywhere, Category = "FPS|Holder")
	TObjectPtr<UFPS_WeaponData> StartingWeaponData;

	// Aim blend. One alpha drives attach offset location, rotation and camera FOV
	// together. Placement comes entirely from the data asset, timing stays universal here.

	UPROPERTY(EditAnywhere, Category = "FPS|Holder|Aim")
	TObjectPtr<UCurveFloat> AimBlendCurve;

	UPROPERTY(EditAnywhere, Category = "FPS|Holder|Aim", meta = (ClampMin = "0.01"))
	float AimBlendDuration = 0.16f;

	// Toss

	/** Launch speed in cm/s, applied directly as linear velocity so mass never changes the arc. */
	UPROPERTY(EditAnywhere, Category = "FPS|Holder|Toss")
	float TossSpeed = 620.0f;

	/** Degrees above the camera's aim. Zero throws flat along the crosshair. */
	UPROPERTY(EditAnywhere, Category = "FPS|Holder|Toss")
	float TossUpAngle = 18.0f;

	/** Spin in radians per second, applied on a random axis. */
	UPROPERTY(EditAnywhere, Category = "FPS|Holder|Toss")
	float TossSpin = 12.0f;

	UPROPERTY(EditAnywhere, Category = "FPS|Holder|Toss")
	FVector TossSpawnOffset = FVector(90.0f, 20.0f, -35.0f);

	/** A weapon thrown while sprinting drops behind the player without this. */
	UPROPERTY(EditAnywhere, Category = "FPS|Holder|Toss", meta = (ClampMin = "0", ClampMax = "1"))
	float TossVelocityInheritance = 0.6f;

	// Input

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputMappingContext> WeaponMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	int32 MappingContextPriority = 1;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> CycleFireModeAction;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> TossWeaponAction;

private:
	void BindInput();

	UFUNCTION()
	void HandlePawnRestarted(APawn* Pawn);

	void HandleFireStarted();
	void HandleFireCompleted();
	void HandleReloadPressed();
	void HandleAimStarted();
	void HandleAimCompleted();
	void HandleCycleFireModePressed();
	void HandleTossPressed();

	void UpdateFire();
	void UpdateAim(float DeltaTime);
	void ApplyPose();

	void SubscribeToWeapon();
	void UnsubscribeFromWeapon();
	void DestroyActiveWeapon();
	USceneComponent* ResolveAttachPoint();

	UFUNCTION()
	void HandleWeaponAmmoChanged(int32 MagazineAmmo, int32 ReserveAmmo);

	UFUNCTION()
	void HandleWeaponFired(const FFPS_WeaponFireInfo& FireInfo);

	UFUNCTION()
	void HandleWeaponFireModeChanged(EFPS_FireMode NewFireMode);

	UPROPERTY(Transient)
	TScriptInterface<IFPS_Weapon> ActiveWeapon;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> AttachPoint;

	UPROPERTY(Transient)
	TObjectPtr<AFPS_Character> OwningCharacter;

	bool bFireInputHeld = false;
	bool bAimInputHeld = false;

	float AimAlpha = 0.0f;
	bool bInputBound = false;
};
