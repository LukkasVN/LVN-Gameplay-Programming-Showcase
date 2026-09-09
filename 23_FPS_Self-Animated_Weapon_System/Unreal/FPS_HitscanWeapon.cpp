#include "FPS_HitscanWeapon.h"

#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "FPS_Hittable.h"
#include "FPS_WeaponData.h"
#include "TimerManager.h"

AFPS_HitscanWeapon::AFPS_HitscanWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	SetRootComponent(WeaponRoot);

	MeshPivot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshPivot"));
	MeshPivot->SetupAttachment(WeaponRoot);
	MeshPivot->ComponentTags.Add(TEXT("ViewmodelPivot"));

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(MeshPivot);
}

void AFPS_HitscanWeapon::BeginPlay()
{
	Super::BeginPlay();

	OwnerCamera = ResolveOwnerCamera();
}

void AFPS_HitscanWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAllTimers();
	Super::EndPlay(EndPlayReason);
}

void AFPS_HitscanWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Firing is a transient state that only exists for the frame the trigger was
	// serviced, so the feedback and HUD can read it without a separate event.
	if (State == EFPS_WeaponState::Firing)
	{
		State = EFPS_WeaponState::Idle;
	}
}

UCameraComponent* AFPS_HitscanWeapon::ResolveOwnerCamera()
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UCameraComponent>() : nullptr;
}

void AFPS_HitscanWeapon::InitializeWeapon_Implementation(UFPS_WeaponData* InData, int32 InMagazineAmmo, int32 InReserveAmmo)
{
	WeaponData = InData;
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Error, TEXT("%s was initialized with no weapon data."), *GetName());
		return;
	}

	// Negative means no snapshot was supplied, which is the world pickup case.
	MagazineAmmo = InMagazineAmmo < 0 ? WeaponData->MagazineSize : FMath::Clamp(InMagazineAmmo, 0, WeaponData->MagazineSize);
	ReserveAmmo = InReserveAmmo < 0 ? WeaponData->StartingReserveAmmo : FMath::Clamp(InReserveAmmo, 0, WeaponData->MaxReserveAmmo);

	CurrentFireMode = WeaponData->ResolveInitialFireMode();
	State = EFPS_WeaponState::Idle;
	NextFireTime = 0.0f;
	bIsAiming = false;

	if (!OwnerCamera)
	{
		OwnerCamera = ResolveOwnerCamera();
	}

	BroadcastAmmo();
	OnFireModeChanged.Broadcast(CurrentFireMode);
}

// Firing

void AFPS_HitscanWeapon::Fire_Implementation()
{
	const uint64 Frame = GFrameCounter;
	const bool bPressEdge = (LastFireInputFrame == 0) || (Frame > LastFireInputFrame + 1);
	LastFireInputFrame = Frame;

	if (!WeaponData || State == EFPS_WeaponState::Reloading || State == EFPS_WeaponState::Bursting)
	{
		return;
	}

	switch (CurrentFireMode)
	{
	case EFPS_FireMode::Semi:
		if (bPressEdge)
		{
			FireOneShot();
		}
		break;

	case EFPS_FireMode::Auto:
		FireOneShot();
		break;

	case EFPS_FireMode::Burst:
		if (bPressEdge)
		{
			StartBurst();
		}
		break;

	default:
		break;
	}
}

bool AFPS_HitscanWeapon::HasAmmoForShot() const
{
	return WeaponData && MagazineAmmo >= WeaponData->AmmoPerShot;
}

void AFPS_HitscanWeapon::FireOneShot()
{
	if (!WeaponData || !GetWorld())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now < NextFireTime)
	{
		return;
	}

	if (!HasAmmoForShot())
	{
		return;
	}

	NextFireTime = Now + WeaponData->GetFireInterval();

	MagazineAmmo -= WeaponData->AmmoPerShot;
	BroadcastAmmo();

	if (!OwnerCamera)
	{
		OwnerCamera = ResolveOwnerCamera();
	}

	const FVector Start = OwnerCamera ? OwnerCamera->GetComponentLocation() : GetActorLocation();
	const FRotator AimRotation = OwnerCamera ? OwnerCamera->GetComponentRotation() : GetActorRotation();
	const float Spread = bIsAiming ? WeaponData->AimSpreadDegrees : WeaponData->HipSpreadDegrees;

	FFPS_WeaponFireInfo ShotInfo;
	ShotInfo.Origin = Start;
	ShotInfo.Direction = AimRotation.Vector();

	for (int32 Pellet = 0; Pellet < FMath::Max(1, WeaponData->PelletsPerShot); ++Pellet)
	{
		TraceSinglePellet(Start, AimRotation, Spread, ShotInfo);
	}

	if (State != EFPS_WeaponState::Bursting)
	{
		State = EFPS_WeaponState::Firing;
	}

	OnFired.Broadcast(ShotInfo);
}

void AFPS_HitscanWeapon::TraceSinglePellet(const FVector& Start, const FRotator& AimRotation, float SpreadDegrees, FFPS_WeaponFireInfo& OutInfo)
{
	const FVector Direction = FMath::VRandCone(AimRotation.Vector(), FMath::DegreesToRadians(SpreadDegrees));
	const FVector End = Start + Direction * WeaponData->Range;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FPS_WeaponTrace), true, GetOwner());
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, WeaponTraceChannel, Params);

	if (bHit)
	{
		if (!OutInfo.bDidHit)
		{
			OutInfo.bDidHit = true;
			OutInfo.Hit = Hit;
			OutInfo.Direction = Direction;
		}

		if (AActor* HitActor = Hit.GetActor())
		{
			if (HitActor->Implements<UFPS_Hittable>())
			{
				IFPS_Hittable::Execute_TakeHit(HitActor, WeaponData->Damage, Hit.ImpactPoint, Direction);
			}
		}

		OnImpact.Broadcast(Hit.ImpactPoint, Hit.ImpactNormal);
	}

	if (bDrawDebugTrace)
	{
		const FVector DebugStart = Start + Direction * DebugTraceStartOffset;
		const FVector DebugEnd = bHit ? Hit.ImpactPoint : End;
		DrawDebugLine(GetWorld(), DebugStart, DebugEnd, FColor::Yellow, false, DebugTraceLifetime, 0, 0.6f);
	}
}

void AFPS_HitscanWeapon::StartBurst()
{
	if (!WeaponData || !HasAmmoForShot())
	{
		return;
	}

	State = EFPS_WeaponState::Bursting;
	BurstShotsRemaining = FMath::Max(1, WeaponData->BurstCount);

	BurstTick();

	if (BurstShotsRemaining > 0)
	{
		GetWorldTimerManager().SetTimer(
			BurstTimerHandle, this, &AFPS_HitscanWeapon::BurstTick, WeaponData->BurstInterval, true);
	}
	else
	{
		State = EFPS_WeaponState::Idle;
	}
}

void AFPS_HitscanWeapon::BurstTick()
{
	if (!WeaponData)
	{
		return;
	}

	NextFireTime = 0.0f;
	FireOneShot();

	--BurstShotsRemaining;

	if (BurstShotsRemaining <= 0 || !HasAmmoForShot())
	{
		GetWorldTimerManager().ClearTimer(BurstTimerHandle);
		BurstShotsRemaining = 0;
		State = EFPS_WeaponState::Idle;

		if (GetWorld())
		{
			NextFireTime = GetWorld()->GetTimeSeconds() + WeaponData->GetFireInterval();
		}
	}
}

// Reload

void AFPS_HitscanWeapon::Reload_Implementation()
{
	if (!WeaponData || State == EFPS_WeaponState::Reloading)
	{
		return;
	}

	if (MagazineAmmo >= WeaponData->MagazineSize || ReserveAmmo <= 0)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(BurstTimerHandle);
	BurstShotsRemaining = 0;

	State = EFPS_WeaponState::Reloading;
	GetWorldTimerManager().SetTimer(
		ReloadTimerHandle, this, &AFPS_HitscanWeapon::FinishReload, WeaponData->ReloadDuration, false);
}

void AFPS_HitscanWeapon::FinishReload()
{
	if (!WeaponData)
	{
		return;
	}

	const int32 Needed = WeaponData->MagazineSize - MagazineAmmo;
	const int32 Taken = FMath::Min(Needed, ReserveAmmo);

	MagazineAmmo += Taken;
	ReserveAmmo -= Taken;

	State = EFPS_WeaponState::Idle;
	BroadcastAmmo();
}

void AFPS_HitscanWeapon::CancelActions_Implementation()
{
	ClearAllTimers();
	BurstShotsRemaining = 0;
	State = EFPS_WeaponState::Idle;
}

void AFPS_HitscanWeapon::ClearAllTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BurstTimerHandle);
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}
}

// Aim and fire mode

void AFPS_HitscanWeapon::AimIn_Implementation()
{
	// Only spread is affected here. Pose and FOV are the holder's job.
	bIsAiming = true;
}

void AFPS_HitscanWeapon::AimOut_Implementation()
{
	bIsAiming = false;
}

void AFPS_HitscanWeapon::CycleFireMode_Implementation()
{
	if (!WeaponData || State != EFPS_WeaponState::Idle)
	{
		return;
	}

	EFPS_FireMode Candidate = CurrentFireMode;
	for (int32 Step = 0; Step < 3; ++Step)
	{
		Candidate = FPS_FireModeUtils::NextInCycle(Candidate);
		if (WeaponData->SupportsFireMode(Candidate))
		{
			break;
		}
	}

	if (Candidate == CurrentFireMode || !WeaponData->SupportsFireMode(Candidate))
	{
		return;
	}

	CurrentFireMode = Candidate;
	OnFireModeChanged.Broadcast(CurrentFireMode);
}

// Ammo

int32 AFPS_HitscanWeapon::AddReserveAmmo_Implementation(int32 Amount)
{
	if (!WeaponData || Amount <= 0)
	{
		return 0;
	}

	const int32 Space = WeaponData->MaxReserveAmmo - ReserveAmmo;
	const int32 Accepted = FMath::Clamp(Amount, 0, FMath::Max(0, Space));

	if (Accepted > 0)
	{
		ReserveAmmo += Accepted;
		BroadcastAmmo();
	}

	return Accepted;
}

void AFPS_HitscanWeapon::BroadcastAmmo()
{
	OnAmmoChanged.Broadcast(MagazineAmmo, ReserveAmmo);
}
