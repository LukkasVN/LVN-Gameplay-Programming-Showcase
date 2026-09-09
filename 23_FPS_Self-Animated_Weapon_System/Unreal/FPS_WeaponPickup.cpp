#include "FPS_WeaponPickup.h"

#include "FPS_WeaponData.h"
#include "FPS_WeaponHolderComponent.h"

AFPS_WeaponPickup::AFPS_WeaponPickup()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFPS_WeaponPickup::SetPayload(UFPS_WeaponData* InWeaponData, int32 InMagazineAmmo, int32 InReserveAmmo)
{
	WeaponData = InWeaponData;
	MagazineAmmo = InMagazineAmmo;
	ReserveAmmo = InReserveAmmo;
}

void AFPS_WeaponPickup::OnInteract_Implementation(AActor* Interactor)
{
	if (!Interactor || !WeaponData)
	{
		return;
	}

	UFPS_WeaponHolderComponent* Holder = Interactor->FindComponentByClass<UFPS_WeaponHolderComponent>();
	if (!Holder)
	{
		return;
	}

	Holder->TryPickUpWeapon(WeaponData, MagazineAmmo, ReserveAmmo);

	OnCollected(Interactor);
	Destroy();
}

FText AFPS_WeaponPickup::GetInteractPrompt_Implementation()
{
	const FText WeaponName = WeaponData ? WeaponData->DisplayName : FText::GetEmpty();
	return FText::Format(InteractPromptFormat, WeaponName);
}
