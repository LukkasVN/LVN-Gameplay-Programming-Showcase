#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPS_InteractPrompt.h"
#include "FPS_Interactable.h"
#include "FPS_WeaponTypes.h"
#include "FPS_AmmoPickup.generated.h"


UCLASS()
class MECHANICS_TEST_LVN_API AFPS_AmmoPickup : public AActor, public IFPS_Interactable, public IFPS_InteractPrompt
{
	GENERATED_BODY()

public:
	AFPS_AmmoPickup();

	virtual void OnFocusEnter_Implementation() override {}
	virtual void OnFocusExit_Implementation() override {}
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS|Pickup")
	TObjectPtr<USceneComponent> PickupRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Pickup")
	EFPS_AmmoType AmmoType = EFPS_AmmoType::Pistol;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Pickup", meta = (ClampMin = "1"))
	int32 Amount = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Pickup")
	FText InteractPrompt = NSLOCTEXT("FPS", "AmmoPickupPrompt", "Pick up ammo");

	/** Fires only when the holder accepted, so effects never play on a refused pickup. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FPS|Pickup")
	void OnCollected(AActor* Interactor);
};
