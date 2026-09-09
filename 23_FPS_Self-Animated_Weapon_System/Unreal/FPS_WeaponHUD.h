#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPS_WeaponTypes.h"
#include "FPS_WeaponHUD.generated.h"

class AFPS_Character;
class UFPS_InteractorComponent;
class UFPS_WeaponData;
class UFPS_WeaponHolderComponent;

UCLASS(Abstract)
class MECHANICS_TEST_LVN_API UFPS_WeaponHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Update hooks. Implement these in the Widget Blueprint.

	UFUNCTION(BlueprintImplementableEvent, Category = "FPS|HUD")
	void OnAmmoUpdated(int32 MagazineAmmo, int32 ReserveAmmo);

	/** WeaponData is null when the player is empty handed, which is the cue to hide the block. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FPS|HUD")
	void OnWeaponUpdated(UFPS_WeaponData* WeaponData);

	UFUNCTION(BlueprintImplementableEvent, Category = "FPS|HUD")
	void OnFireModeUpdated(EFPS_FireMode NewFireMode);

	UFUNCTION(BlueprintImplementableEvent, Category = "FPS|HUD")
	void OnInteractPromptUpdated(const FText& Prompt, bool bVisible);

	/** Flash the ammo counter. Fires when a pickup could not be taken. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FPS|HUD")
	void OnAmmoRejected(EFPS_AmmoType RejectedType);

	UFUNCTION(BlueprintImplementableEvent, Category = "FPS|HUD")
	void OnDashUpdated(float Normalized, float RemainingSeconds, bool bReady);

protected:
	UFUNCTION()
	void HandleAmmoChanged(int32 MagazineAmmo, int32 ReserveAmmo);

	UFUNCTION()
	void HandleActiveWeaponChanged(UFPS_WeaponData* WeaponData);

	UFUNCTION()
	void HandleFireModeChanged(EFPS_FireMode NewFireMode);

	UFUNCTION()
	void HandleAmmoRejected(EFPS_AmmoType RejectedType);

	UFUNCTION()
	void HandleFocusChanged(AActor* FocusedActor, const FText& Prompt);

private:
	void ResolveReferences();

	UPROPERTY(Transient)
	TObjectPtr<UFPS_WeaponHolderComponent> Holder;

	UPROPERTY(Transient)
	TObjectPtr<UFPS_InteractorComponent> Interactor;

	UPROPERTY(Transient)
	TObjectPtr<AFPS_Character> Character;

	bool bDashWasReady = true;
	float LastDashRemaining = -1.0f;
};
