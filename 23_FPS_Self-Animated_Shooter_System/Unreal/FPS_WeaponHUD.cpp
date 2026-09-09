#include "FPS_WeaponHUD.h"

#include "FPS_Character.h"
#include "FPS_InteractorComponent.h"
#include "FPS_Weapon.h"
#include "FPS_WeaponData.h"
#include "FPS_WeaponHolderComponent.h"

void UFPS_WeaponHUD::NativeConstruct()
{
	Super::NativeConstruct();

	ResolveReferences();

	if (Holder)
	{
		Holder->OnAmmoChanged.AddDynamic(this, &UFPS_WeaponHUD::HandleAmmoChanged);
		Holder->OnActiveWeaponChanged.AddDynamic(this, &UFPS_WeaponHUD::HandleActiveWeaponChanged);
		Holder->OnFireModeChanged.AddDynamic(this, &UFPS_WeaponHUD::HandleFireModeChanged);
		Holder->OnAmmoRejected.AddDynamic(this, &UFPS_WeaponHUD::HandleAmmoRejected);

		UFPS_WeaponData* Data = Holder->GetActiveWeaponData();
		OnWeaponUpdated(Data);

		if (AActor* WeaponActor = Holder->GetActiveWeaponActor())
		{
			OnAmmoUpdated(
				IFPS_Weapon::Execute_GetMagazineAmmo(WeaponActor),
				IFPS_Weapon::Execute_GetReserveAmmo(WeaponActor));
			OnFireModeUpdated(IFPS_Weapon::Execute_GetCurrentFireMode(WeaponActor));
		}
		else
		{
			OnAmmoUpdated(0, 0);
		}
	}

	if (Interactor)
	{
		Interactor->OnFocusChanged.AddDynamic(this, &UFPS_WeaponHUD::HandleFocusChanged);
	}

	OnInteractPromptUpdated(FText::GetEmpty(), false);
}

void UFPS_WeaponHUD::NativeDestruct()
{
	if (Holder)
	{
		Holder->OnAmmoChanged.RemoveAll(this);
		Holder->OnActiveWeaponChanged.RemoveAll(this);
		Holder->OnFireModeChanged.RemoveAll(this);
		Holder->OnAmmoRejected.RemoveAll(this);
	}

	if (Interactor)
	{
		Interactor->OnFocusChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UFPS_WeaponHUD::ResolveReferences()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		return;
	}

	Holder = OwningPawn->FindComponentByClass<UFPS_WeaponHolderComponent>();
	Interactor = OwningPawn->FindComponentByClass<UFPS_InteractorComponent>();
	Character = Cast<AFPS_Character>(OwningPawn);
}

void UFPS_WeaponHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Character)
	{
		return;
	}

	const bool bReady = Character->IsDashReady();
	const float Remaining = Character->GetDashCooldownRemaining();

	if (bReady && bDashWasReady)
	{
		return;
	}

	if (!bReady || !FMath::IsNearlyEqual(Remaining, LastDashRemaining, 0.01f) || bReady != bDashWasReady)
	{
		OnDashUpdated(Character->GetDashCooldownNormalized(), Remaining, bReady);
		LastDashRemaining = Remaining;
		bDashWasReady = bReady;
	}
}

void UFPS_WeaponHUD::HandleAmmoChanged(int32 MagazineAmmo, int32 ReserveAmmo)
{
	OnAmmoUpdated(MagazineAmmo, ReserveAmmo);
}

void UFPS_WeaponHUD::HandleActiveWeaponChanged(UFPS_WeaponData* WeaponData)
{
	OnWeaponUpdated(WeaponData);
}

void UFPS_WeaponHUD::HandleFireModeChanged(EFPS_FireMode NewFireMode)
{
	OnFireModeUpdated(NewFireMode);
}

void UFPS_WeaponHUD::HandleAmmoRejected(EFPS_AmmoType RejectedType)
{
	OnAmmoRejected(RejectedType);
}

void UFPS_WeaponHUD::HandleFocusChanged(AActor* FocusedActor, const FText& Prompt)
{
	OnInteractPromptUpdated(Prompt, FocusedActor != nullptr && !Prompt.IsEmpty());
}
