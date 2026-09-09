#include "FPS_ViewmodelFeedbackComponent.h"

#include "FPS_Character.h"
#include "FPS_Weapon.h"
#include "FPS_WeaponHolderComponent.h"

UFPS_ViewmodelFeedbackComponent::UFPS_ViewmodelFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFPS_ViewmodelFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	// The wrong object guard. This belongs on the weapon actor, never on the attach
	// point or the character, where it would overwrite the holder's aim blend.
	if (!GetOwner() || !GetOwner()->Implements<UFPS_Weapon>())
	{
		UE_LOG(LogTemp, Error,
			TEXT("UFPS_ViewmodelFeedbackComponent is on %s, which is not a weapon. ")
			TEXT("It belongs on the weapon actor, not on the attach point or the character. Disabling."),
			*GetNameSafe(GetOwner()));

		SetComponentTickEnabled(false);
		return;
	}

	Pivot = ResolvePivot();
	if (!Pivot)
	{
		UE_LOG(LogTemp, Error,
			TEXT("No scene component tagged %s found on %s. Disabling viewmodel feedback."),
			*PivotTag.ToString(), *GetNameSafe(GetOwner()));

		SetComponentTickEnabled(false);
		return;
	}

	BasePivotLocation = Pivot->GetRelativeLocation();
	BasePivotRotation = Pivot->GetRelativeRotation();
	BasePivotScale = Pivot->GetRelativeScale3D();

	if (AActor* HolderOwner = GetOwner()->GetOwner())
	{
		Holder = HolderOwner->FindComponentByClass<UFPS_WeaponHolderComponent>();
		Character = Cast<AFPS_Character>(HolderOwner);
	}

	SubscribeToHolder();
}

void UFPS_ViewmodelFeedbackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromHolder();
	Super::EndPlay(EndPlayReason);
}

USceneComponent* UFPS_ViewmodelFeedbackComponent::ResolvePivot()
{
	TArray<USceneComponent*> SceneComponents;
	GetOwner()->GetComponents<USceneComponent>(SceneComponents);

	for (USceneComponent* Component : SceneComponents)
	{
		if (Component->ComponentHasTag(PivotTag))
		{
			return Component;
		}
	}

	return nullptr;
}

void UFPS_ViewmodelFeedbackComponent::SubscribeToHolder()
{
	if (Holder)
	{
		Holder->OnWeaponFired.AddDynamic(this, &UFPS_ViewmodelFeedbackComponent::HandleWeaponFired);
		Holder->OnFireModeChanged.AddDynamic(this, &UFPS_ViewmodelFeedbackComponent::HandleFireModeChanged);
	}

	if (Character)
	{
		Character->OnLandedImpact.AddDynamic(this, &UFPS_ViewmodelFeedbackComponent::HandleLandedImpact);
	}
}

void UFPS_ViewmodelFeedbackComponent::UnsubscribeFromHolder()
{
	if (Holder)
	{
		Holder->OnWeaponFired.RemoveAll(this);
		Holder->OnFireModeChanged.RemoveAll(this);
	}

	if (Character)
	{
		Character->OnLandedImpact.RemoveAll(this);
	}
}

// Impulses

void UFPS_ViewmodelFeedbackComponent::HandleWeaponFired(const FFPS_WeaponFireInfo& FireInfo)
{
	// Scale impulses proportional to base scale
	const float ScaleRef = FMath::Max(BasePivotScale.GetAbsMax(), KINDA_SMALL_NUMBER);

	LocationSpring.AddImpulse(FVector(RecoilKickback, 0.0f, RecoilLift) * ScaleRef);
	RotationSpring.AddImpulse(FVector(RecoilPitch, 0.0f, RecoilRoll * RecoilRollSign));
	ScaleSpring.AddImpulse(FVector(RecoilStretch, -RecoilSquash, -RecoilSquash));

	RecoilRollSign *= -1.0f;
}

void UFPS_ViewmodelFeedbackComponent::HandleFireModeChanged(EFPS_FireMode NewFireMode)
{
	RotationSpring.AddImpulse(FVector(0.0f, 0.0f, FireModeRoll));
	ScaleSpring.AddImpulse(FVector(-FireModeSquash, FireModeSquash, 0.0f));
}

void UFPS_ViewmodelFeedbackComponent::HandleLandedImpact(float ImpactSpeed)
{
	const float Alpha = FMath::Clamp(ImpactSpeed / FMath::Max(LandingMaxImpactSpeed, 1.0f), 0.0f, 1.0f);

	LocationSpring.AddImpulse(FVector(0.0f, 0.0f, LandingKick * Alpha));
	RotationSpring.AddImpulse(FVector(LandingPitch * Alpha, 0.0f, 0.0f));
}

// Tick

void UFPS_ViewmodelFeedbackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Pivot)
	{
		return;
	}

	// Reload edges are derived from the generic interface state rather than a
	// dedicated event, so any weapon implementation gets them for free.
	const bool bReloading = GetOwner()->Implements<UFPS_Weapon>()
		? IFPS_Weapon::Execute_IsReloading(GetOwner())
		: false;

	if (bReloading != bWasReloading)
	{
		const float Sign = bReloading ? 1.0f : -1.0f;
		RotationSpring.AddImpulse(FVector(ReloadPunchRotation * Sign, 0.0f, 0.0f));
		LocationSpring.AddImpulse(FVector(0.0f, 0.0f, ReloadPunchLocation * Sign));
		bWasReloading = bReloading;
	}

	ReloadPoseAlpha = FMath::FInterpTo(ReloadPoseAlpha, bReloading ? 1.0f : 0.0f, DeltaTime, ReloadPoseBlendSpeed);

	LocationSpring.Tick(LocationStiffness, LocationDamping, DeltaTime);
	RotationSpring.Tick(RotationStiffness, RotationDamping, DeltaTime);
	ScaleSpring.Tick(ScaleStiffness, ScaleDamping, DeltaTime);

	// Idle drift is deliberately not a weapon bob. A second bob synced to the
	// camera reads as doubled motion rather than as more life, so this runs far
	// below the headbob frequency and never matches its phase.
	DriftTime += DeltaTime * IdleDriftFrequency;
	const FVector Drift(
		0.0f,
		FMath::Sin(DriftTime) * IdleDriftLocation,
		FMath::Sin(DriftTime * 0.73f) * IdleDriftLocation);
	const FVector DriftRot(
		FMath::Sin(DriftTime * 0.61f) * IdleDriftRotation,
		FMath::Sin(DriftTime * 0.47f) * IdleDriftRotation,
		0.0f);

	const float AimAlpha = Holder ? Holder->GetAimAlpha() : 0.0f;
	const float Damp = FMath::Lerp(1.0f, 1.0f - AimDamping, FMath::Clamp(AimAlpha, 0.0f, 1.0f));

	const FVector Location = BasePivotLocation
		+ (LocationSpring.Value + Drift) * Damp
		+ ReloadPoseLocation * ReloadPoseAlpha;

	const FRotator SpringRotation = (RotationSpring.ToRotator() + FRotator(DriftRot.X, DriftRot.Y, DriftRot.Z)) * Damp;
	const FRotator Rotation = BasePivotRotation + SpringRotation + ReloadPoseRotation * ReloadPoseAlpha;

	const FVector Scale = BasePivotScale * (FVector::OneVector + ScaleSpring.Value * Damp);

	Pivot->SetRelativeTransform(FTransform(Rotation, Location, Scale));
}
