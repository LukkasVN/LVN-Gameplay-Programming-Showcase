#include "FPS_AmmoPickup.h"

#include "FPS_WeaponHolderComponent.h"

AFPS_AmmoPickup::AFPS_AmmoPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PickupRoot"));
	SetRootComponent(PickupRoot);
}

void AFPS_AmmoPickup::OnInteract_Implementation(AActor* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	UFPS_WeaponHolderComponent* Holder = Interactor->FindComponentByClass<UFPS_WeaponHolderComponent>();
	if (!Holder)
	{
		return;
	}

	// A refusal leaves the pickup in the world. Destroying it on a caliber mismatch
	// or a capped reserve would consume it for nothing.
	if (!Holder->AddAmmo(AmmoType, Amount))
	{
		return;
	}

	OnCollected(Interactor);
	Destroy();
}

FText AFPS_AmmoPickup::GetInteractPrompt_Implementation()
{
	return InteractPrompt;
}
