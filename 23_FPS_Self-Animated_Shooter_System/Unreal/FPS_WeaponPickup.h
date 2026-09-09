#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPS_InteractPrompt.h"
#include "FPS_Interactable.h"
#include "FPS_WeaponPickup.generated.h"

class UFPS_WeaponData;

UCLASS()
class MECHANICS_TEST_LVN_API AFPS_WeaponPickup : public AActor, public IFPS_Interactable, public IFPS_InteractPrompt
{
	GENERATED_BODY()

public:
	AFPS_WeaponPickup();

	virtual void OnFocusEnter_Implementation() override {}
	virtual void OnFocusExit_Implementation() override {}
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation() override;

	/** Negative values mean no snapshot, so the weapon falls back to its data asset. */
	UFUNCTION(BlueprintCallable, Category = "FPS|Pickup")
	void SetPayload(UFPS_WeaponData* InWeaponData, int32 InMagazineAmmo, int32 InReserveAmmo);

	UFUNCTION(BlueprintPure, Category = "FPS|Pickup")
	UFPS_WeaponData* GetWeaponData() const { return WeaponData; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Pickup")
	TObjectPtr<UFPS_WeaponData> WeaponData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Pickup")
	int32 MagazineAmmo = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Pickup")
	int32 ReserveAmmo = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Pickup")
	FText InteractPromptFormat = NSLOCTEXT("FPS", "WeaponPickupPrompt", "Pick up {0}");

	UFUNCTION(BlueprintImplementableEvent, Category = "FPS|Pickup")
	void OnCollected(AActor* Interactor);
};
