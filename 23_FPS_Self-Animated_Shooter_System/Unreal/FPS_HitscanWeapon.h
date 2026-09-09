#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "FPS_Weapon.h"
#include "FPS_WeaponData.h"
#include "FPS_WeaponTypes.h"
#include "FPS_HitscanWeapon.generated.h"

class UCameraComponent;
class UFPS_WeaponData;

UENUM(BlueprintType)
enum class EFPS_WeaponState : uint8
{
	Idle,
	Firing,
	Bursting,
	Reloading
};

UCLASS()
class MECHANICS_TEST_LVN_API AFPS_HitscanWeapon : public AActor, public IFPS_Weapon
{
	GENERATED_BODY()

public:
	AFPS_HitscanWeapon();

	virtual void Tick(float DeltaSeconds) override;

	// IFPS_Weapon

	virtual UFPS_WeaponData* GetWeaponData_Implementation() override { return WeaponData; }
	virtual int32 GetMagazineAmmo_Implementation() override { return MagazineAmmo; }
	virtual int32 GetReserveAmmo_Implementation() override { return ReserveAmmo; }
	virtual EFPS_FireMode GetCurrentFireMode_Implementation() override { return CurrentFireMode; }
	virtual bool IsReloading_Implementation() override { return State == EFPS_WeaponState::Reloading; }

	virtual void InitializeWeapon_Implementation(UFPS_WeaponData* InData, int32 InMagazineAmmo, int32 InReserveAmmo) override;
	virtual void Fire_Implementation() override;
	virtual void Reload_Implementation() override;
	virtual void CancelActions_Implementation() override;
	virtual void AimIn_Implementation() override;
	virtual void AimOut_Implementation() override;
	virtual void CycleFireMode_Implementation() override;
	virtual int32 AddReserveAmmo_Implementation(int32 Amount) override;

	virtual FFPS_OnWeaponAmmoChanged& GetOnAmmoChanged() override { return OnAmmoChanged; }
	virtual FFPS_OnWeaponFired& GetOnFired() override { return OnFired; }
	virtual FFPS_OnWeaponFireModeChanged& GetOnFireModeChanged() override { return OnFireModeChanged; }

	// Integration points

	UPROPERTY(BlueprintAssignable, Category = "FPS|Weapon|Events")
	FFPS_OnWeaponAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "FPS|Weapon|Events")
	FFPS_OnWeaponFired OnFired;

	UPROPERTY(BlueprintAssignable, Category = "FPS|Weapon|Events")
	FFPS_OnWeaponFireModeChanged OnFireModeChanged;

	/** Broadcast once per pellet, so a shotgun spawns eight impacts from one shot. */
	UPROPERTY(BlueprintAssignable, Category = "FPS|Weapon|Events")
	FFPS_OnWeaponImpact OnImpact;

	UFUNCTION(BlueprintPure, Category = "FPS|Weapon")
	EFPS_WeaponState GetWeaponState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "FPS|Weapon")
	bool IsAiming() const { return bIsAiming; }

	/** Where the viewmodel feedback drives the mesh. Leave the mesh itself parented under this. */
	UFUNCTION(BlueprintPure, Category = "FPS|Weapon")
	USceneComponent* GetMeshPivot() const { return MeshPivot; }

	UFUNCTION(BlueprintPure, Category = "FPS|Weapon")
	USceneComponent* GetMuzzlePoint() const { return MuzzlePoint; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	TObjectPtr<USceneComponent> WeaponRoot;

	/** Tagged for the feedback component to find, so the feedback never casts to a weapon type. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	TObjectPtr<USceneComponent> MeshPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	TObjectPtr<USceneComponent> MuzzlePoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	TObjectPtr<UFPS_WeaponData> WeaponData;

	/** A dedicated channel rather than an object type array, so level geometry and targets answer the same query. */
	UPROPERTY(EditAnywhere, Category = "FPS|Weapon|Trace")
	TEnumAsByte<ECollisionChannel> WeaponTraceChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditAnywhere, Category = "FPS|Weapon|Debug")
	bool bDrawDebugTrace = false;

	UPROPERTY(EditAnywhere, Category = "FPS|Weapon|Debug")
	float DebugTraceLifetime = 0.4f;

	/** Pushed forward from the camera so the line does not clip the near plane. */
	UPROPERTY(EditAnywhere, Category = "FPS|Weapon|Debug")
	float DebugTraceStartOffset = 60.0f;

private:
	void FireOneShot();
	void TraceSinglePellet(const FVector& Start, const FRotator& AimRotation, float SpreadDegrees, FFPS_WeaponFireInfo& OutInfo);
	void StartBurst();
	void BurstTick();
	void FinishReload();
	void ClearAllTimers();
	bool HasAmmoForShot() const;
	UCameraComponent* ResolveOwnerCamera();
	void BroadcastAmmo();

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> OwnerCamera;

	EFPS_WeaponState State = EFPS_WeaponState::Idle;
	EFPS_FireMode CurrentFireMode = EFPS_FireMode::Semi;

	int32 MagazineAmmo = 0;
	int32 ReserveAmmo = 0;
	bool bIsAiming = false;

	float NextFireTime = 0.0f;
	int32 BurstShotsRemaining = 0;

	FTimerHandle BurstTimerHandle;
	FTimerHandle ReloadTimerHandle;

	/**
	 * Fire() only arrives on frames where the input is held, so a press edge is
	 * simply a call that did not follow another call on the previous frame.
	 * Comparing frame numbers this way is independent of whether input is processed
	 * before or after this actor ticks.
	 */
	uint64 LastFireInputFrame = 0;
};
