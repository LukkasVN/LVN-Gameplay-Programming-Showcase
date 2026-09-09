#include "FPS_WeaponHolderComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Curves/CurveFloat.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "FPS_Character.h"
#include "FPS_WeaponData.h"
#include "FPS_WeaponPickup.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UFPS_WeaponHolderComponent::UFPS_WeaponHolderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFPS_WeaponHolderComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<AFPS_Character>(GetOwner());
	AttachPoint = ResolveAttachPoint();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("UFPS_WeaponHolderComponent expects a Pawn owner, found %s."), *GetNameSafe(GetOwner()));
		return;
	}

	if (OwnerPawn->InputComponent)
	{
		BindInput();
	}
	else
	{
		OwnerPawn->ReceiveRestartedDelegate.AddDynamic(this, &UFPS_WeaponHolderComponent::HandlePawnRestarted);
	}

	if (StartingWeaponData)
	{
		EquipWeapon(StartingWeaponData);
	}
}

void UFPS_WeaponHolderComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromWeapon();

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->ReceiveRestartedDelegate.RemoveDynamic(this, &UFPS_WeaponHolderComponent::HandlePawnRestarted);
	}

	if (OwningCharacter)
	{
		OwningCharacter->ClearCameraZoomOverride();
	}

	Super::EndPlay(EndPlayReason);
}

USceneComponent* UFPS_WeaponHolderComponent::ResolveAttachPoint()
{
	if (!GetOwner())
	{
		return nullptr;
	}

	TArray<USceneComponent*> SceneComponents;
	GetOwner()->GetComponents<USceneComponent>(SceneComponents);

	for (USceneComponent* Component : SceneComponents)
	{
		if (Component->ComponentHasTag(AttachPointTag))
		{
			return Component;
		}
	}

	// Falling back to the camera keeps the system runnable before the attach point
	// is added, but poses will be authored against the camera origin instead.
	UE_LOG(LogTemp, Warning,
		TEXT("No scene component tagged %s found on %s. Falling back to the camera."),
		*AttachPointTag.ToString(), *GetNameSafe(GetOwner()));

	return GetOwner()->FindComponentByClass<UCameraComponent>();
}

// Input

void UFPS_WeaponHolderComponent::HandlePawnRestarted(APawn* Pawn)
{
	BindInput();
}

void UFPS_WeaponHolderComponent::BindInput()
{
	if (bInputBound)
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	if (const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (WeaponMappingContext)
			{
				Subsystem->AddMappingContext(WeaponMappingContext, MappingContextPriority);
			}
		}
	}

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(OwnerPawn->InputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	if (FireAction)
	{
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &UFPS_WeaponHolderComponent::HandleFireStarted);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &UFPS_WeaponHolderComponent::HandleFireCompleted);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Canceled, this, &UFPS_WeaponHolderComponent::HandleFireCompleted);
	}
	if (ReloadAction)
	{
		EnhancedInput->BindAction(ReloadAction, ETriggerEvent::Started, this, &UFPS_WeaponHolderComponent::HandleReloadPressed);
	}
	if (AimAction)
	{
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &UFPS_WeaponHolderComponent::HandleAimStarted);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &UFPS_WeaponHolderComponent::HandleAimCompleted);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Canceled, this, &UFPS_WeaponHolderComponent::HandleAimCompleted);
	}
	if (CycleFireModeAction)
	{
		EnhancedInput->BindAction(CycleFireModeAction, ETriggerEvent::Started, this, &UFPS_WeaponHolderComponent::HandleCycleFireModePressed);
	}
	if (TossWeaponAction)
	{
		EnhancedInput->BindAction(TossWeaponAction, ETriggerEvent::Started, this, &UFPS_WeaponHolderComponent::HandleTossPressed);
	}

	bInputBound = true;
}

void UFPS_WeaponHolderComponent::HandleFireStarted() { bFireInputHeld = true; }
void UFPS_WeaponHolderComponent::HandleFireCompleted() { bFireInputHeld = false; }
void UFPS_WeaponHolderComponent::HandleAimStarted() { bAimInputHeld = true; }
void UFPS_WeaponHolderComponent::HandleAimCompleted() { bAimInputHeld = false; }

void UFPS_WeaponHolderComponent::HandleReloadPressed()
{
	if (UObject* Weapon = ActiveWeapon.GetObject())
	{
		IFPS_Weapon::Execute_Reload(Weapon);
	}
}

void UFPS_WeaponHolderComponent::HandleCycleFireModePressed()
{
	if (UObject* Weapon = ActiveWeapon.GetObject())
	{
		IFPS_Weapon::Execute_CycleFireMode(Weapon);
	}
}

void UFPS_WeaponHolderComponent::HandleTossPressed()
{
	TossWeapon();
}

// Tick

void UFPS_WeaponHolderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateFire();
	UpdateAim(DeltaTime);
	ApplyPose();
}

void UFPS_WeaponHolderComponent::UpdateFire()
{
	if (!bFireInputHeld)
	{
		return;
	}

	if (UObject* Weapon = ActiveWeapon.GetObject())
	{
		IFPS_Weapon::Execute_Fire(Weapon);
	}
}

void UFPS_WeaponHolderComponent::UpdateAim(float DeltaTime)
{
	UObject* Weapon = ActiveWeapon.GetObject();

	const bool bReloading = Weapon && IFPS_Weapon::Execute_IsReloading(Weapon);
	const bool bDashing = OwningCharacter && OwningCharacter->IsDashing();

	const bool bWantsAim = bAimInputHeld && Weapon && !bReloading && !bDashing;

	const float Rate = 1.0f / FMath::Max(AimBlendDuration, KINDA_SMALL_NUMBER);
	const float Target = bWantsAim ? 1.0f : 0.0f;
	AimAlpha = FMath::FInterpConstantTo(AimAlpha, Target, DeltaTime, Rate);

	if (Weapon)
	{
		const bool bWeaponAiming = bWantsAim;
		if (bWeaponAiming)
		{
			IFPS_Weapon::Execute_AimIn(Weapon);
		}
		else
		{
			IFPS_Weapon::Execute_AimOut(Weapon);
		}
	}
}

void UFPS_WeaponHolderComponent::ApplyPose()
{
	AActor* WeaponActor = GetActiveWeaponActor();
	UFPS_WeaponData* Data = GetActiveWeaponData();

	if (!WeaponActor || !Data || !AttachPoint)
	{
		if (OwningCharacter && AimAlpha <= 0.0f)
		{
			OwningCharacter->ClearCameraZoomOverride();
		}
		return;
	}

	// One alpha drives location, rotation and FOV together off a shared curve.
	const float Curved = AimBlendCurve ? AimBlendCurve->GetFloatValue(AimAlpha) : AimAlpha;

	const FVector Location = FMath::Lerp(Data->HipPose.GetLocation(), Data->AimPose.GetLocation(), Curved);
	const FQuat Rotation = FQuat::Slerp(Data->HipPose.GetRotation(), Data->AimPose.GetRotation(), Curved).GetNormalized();
	const FVector Scale = FMath::Lerp(Data->HipPose.GetScale3D(), Data->AimPose.GetScale3D(), Curved);

	WeaponActor->SetActorRelativeTransform(FTransform(Rotation, Location, Scale));

	if (OwningCharacter)
	{
		if (Curved > KINDA_SMALL_NUMBER)
		{
			const float FOV = FMath::Lerp(OwningCharacter->GetBaseFOV(), Data->AimFOV, Curved);
			OwningCharacter->SetCameraZoomOverride(FOV);
		}
		else
		{
			OwningCharacter->ClearCameraZoomOverride();
		}
	}
}

// Equip and toss

UFPS_WeaponData* UFPS_WeaponHolderComponent::GetActiveWeaponData() const
{
	UObject* Weapon = ActiveWeapon.GetObject();
	return Weapon ? IFPS_Weapon::Execute_GetWeaponData(Weapon) : nullptr;
}

void UFPS_WeaponHolderComponent::EquipWeapon(UFPS_WeaponData* Data, int32 MagazineAmmo, int32 ReserveAmmo)
{
	if (!Data || !Data->WeaponActorClass || !GetWorld())
	{
		return;
	}

	if (!AttachPoint)
	{
		AttachPoint = ResolveAttachPoint();
		if (!AttachPoint)
		{
			return;
		}
	}

	DestroyActiveWeapon();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Spawned = GetWorld()->SpawnActor<AActor>(
		Data->WeaponActorClass, AttachPoint->GetComponentTransform(), SpawnParams);

	if (!Spawned)
	{
		return;
	}

	if (!Spawned->Implements<UFPS_Weapon>())
	{
		UE_LOG(LogTemp, Error,
			TEXT("%s does not implement IFPS_Weapon and cannot be equipped."), *GetNameSafe(Data->WeaponActorClass));
		Spawned->Destroy();
		return;
	}

	Spawned->AttachToComponent(AttachPoint, FAttachmentTransformRules::SnapToTargetIncludingScale);

	ActiveWeapon.SetObject(Spawned);
	ActiveWeapon.SetInterface(Cast<IFPS_Weapon>(Spawned));
	SubscribeToWeapon();

	IFPS_Weapon::Execute_InitializeWeapon(Spawned, Data, MagazineAmmo, ReserveAmmo);

	AimAlpha = 0.0f;
	ApplyPose();

	OnActiveWeaponChanged.Broadcast(Data);
	OnAmmoChanged.Broadcast(
		IFPS_Weapon::Execute_GetMagazineAmmo(Spawned),
		IFPS_Weapon::Execute_GetReserveAmmo(Spawned));
	OnFireModeChanged.Broadcast(IFPS_Weapon::Execute_GetCurrentFireMode(Spawned));
}

void UFPS_WeaponHolderComponent::TryPickUpWeapon(UFPS_WeaponData* Data, int32 MagazineAmmo, int32 ReserveAmmo)
{
	if (!Data)
	{
		return;
	}

	// Tossing first means a pickup is never refused for lack of room.
	if (HasWeapon())
	{
		TossWeapon();
	}

	EquipWeapon(Data, MagazineAmmo, ReserveAmmo);
}

void UFPS_WeaponHolderComponent::TossWeapon()
{
	AActor* WeaponActor = GetActiveWeaponActor();
	if (!WeaponActor || !GetWorld())
	{
		return;
	}

	IFPS_Weapon::Execute_CancelActions(WeaponActor);

	UFPS_WeaponData* Data = IFPS_Weapon::Execute_GetWeaponData(WeaponActor);
	const int32 Magazine = IFPS_Weapon::Execute_GetMagazineAmmo(WeaponActor);
	const int32 Reserve = IFPS_Weapon::Execute_GetReserveAmmo(WeaponActor);

	const UCameraComponent* Camera = GetOwner() ? GetOwner()->FindComponentByClass<UCameraComponent>() : nullptr;

	const FVector AimForward = Camera ? Camera->GetForwardVector() : WeaponActor->GetActorForwardVector();
	const FVector AimRight = Camera ? Camera->GetRightVector() : WeaponActor->GetActorRightVector();
	const FVector AimOrigin = Camera ? Camera->GetComponentLocation() : WeaponActor->GetActorLocation();

	const FVector SpawnLocation = AimOrigin
		+ AimForward * TossSpawnOffset.X
		+ AimRight * TossSpawnOffset.Y
		+ FVector::UpVector * TossSpawnOffset.Z;

	const FVector TossDirection = AimForward.RotateAngleAxis(-TossUpAngle, AimRight).GetSafeNormal();

	const FTransform SpawnTransform(WeaponActor->GetActorRotation(), SpawnLocation);

	DestroyActiveWeapon();

	if (OwningCharacter)
	{
		OwningCharacter->ClearCameraZoomOverride();
	}

	OnActiveWeaponChanged.Broadcast(nullptr);
	OnAmmoChanged.Broadcast(0, 0);

	if (!Data || !Data->PickupActorClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = Cast<APawn>(GetOwner());

	AActor* SpawnedPickup = GetWorld()->SpawnActor<AActor>(
		Data->PickupActorClass, SpawnTransform, SpawnParams);

	AFPS_WeaponPickup* Pickup = Cast<AFPS_WeaponPickup>(SpawnedPickup);
	if (!Pickup)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("PickupActorClass on %s is not an AFPS_WeaponPickup, the tossed ammo snapshot was lost."),
			*GetNameSafe(Data));
		return;
	}

	Pickup->SetPayload(Data, Magazine, Reserve);

	UPrimitiveComponent* Body = Cast<UPrimitiveComponent>(Pickup->GetRootComponent());
	if (!Body || !Body->IsSimulatingPhysics())
	{
		TArray<UPrimitiveComponent*> Primitives;
		Pickup->GetComponents<UPrimitiveComponent>(Primitives);

		Body = nullptr;
		for (UPrimitiveComponent* Primitive : Primitives)
		{
			if (Primitive->IsSimulatingPhysics())
			{
				Body = Primitive;
				break;
			}
		}
	}

	if (!Body)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s has no component with Simulate Physics enabled, so it cannot be tossed ")
			TEXT("and will only drop. Tick Simulate Physics on its mesh in the pickup Blueprint."),
			*GetNameSafe(Pickup));
		return;
	}

	FVector Velocity = TossDirection * TossSpeed;
	if (GetOwner())
	{
		Velocity += GetOwner()->GetVelocity() * TossVelocityInheritance;
	}

	Body->WakeAllRigidBodies();
	Body->SetPhysicsLinearVelocity(Velocity);
	Body->SetPhysicsAngularVelocityInRadians(FMath::VRand() * TossSpin);
}

bool UFPS_WeaponHolderComponent::AddAmmo(EFPS_AmmoType AmmoType, int32 Amount)
{
	UObject* Weapon = ActiveWeapon.GetObject();
	UFPS_WeaponData* Data = GetActiveWeaponData();

	if (!Weapon || !Data || Data->AmmoType != AmmoType)
	{
		OnAmmoRejected.Broadcast(AmmoType);
		return false;
	}

	const bool bWasDry = IFPS_Weapon::Execute_GetMagazineAmmo(Weapon) <= 0;
	const int32 Accepted = IFPS_Weapon::Execute_AddReserveAmmo(Weapon, Amount);

	if (Accepted <= 0)
	{
		OnAmmoRejected.Broadcast(AmmoType);
		return false;
	}

	if (bWasDry)
	{
		IFPS_Weapon::Execute_Reload(Weapon);
	}

	return true;
}

void UFPS_WeaponHolderComponent::DestroyActiveWeapon()
{
	UnsubscribeFromWeapon();

	if (AActor* WeaponActor = GetActiveWeaponActor())
	{
		WeaponActor->Destroy();
	}

	ActiveWeapon.SetObject(nullptr);
	ActiveWeapon.SetInterface(nullptr);
	AimAlpha = 0.0f;
}

void UFPS_WeaponHolderComponent::SubscribeToWeapon()
{
	if (IFPS_Weapon* Native = ActiveWeapon.GetInterface())
	{
		Native->GetOnAmmoChanged().AddDynamic(this, &UFPS_WeaponHolderComponent::HandleWeaponAmmoChanged);
		Native->GetOnFired().AddDynamic(this, &UFPS_WeaponHolderComponent::HandleWeaponFired);
		Native->GetOnFireModeChanged().AddDynamic(this, &UFPS_WeaponHolderComponent::HandleWeaponFireModeChanged);
	}
}

void UFPS_WeaponHolderComponent::UnsubscribeFromWeapon()
{
	if (IFPS_Weapon* Native = ActiveWeapon.GetInterface())
	{
		Native->GetOnAmmoChanged().RemoveAll(this);
		Native->GetOnFired().RemoveAll(this);
		Native->GetOnFireModeChanged().RemoveAll(this);
	}
}

void UFPS_WeaponHolderComponent::HandleWeaponAmmoChanged(int32 MagazineAmmo, int32 ReserveAmmo)
{
	OnAmmoChanged.Broadcast(MagazineAmmo, ReserveAmmo);
}

void UFPS_WeaponHolderComponent::HandleWeaponFired(const FFPS_WeaponFireInfo& FireInfo)
{
	OnWeaponFired.Broadcast(FireInfo);
}

void UFPS_WeaponHolderComponent::HandleWeaponFireModeChanged(EFPS_FireMode NewFireMode)
{
	OnFireModeChanged.Broadcast(NewFireMode);
}
