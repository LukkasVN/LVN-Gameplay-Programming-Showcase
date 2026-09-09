#include "FPS_Target.h"

#include "TimerManager.h"

AFPS_Target::AFPS_Target()
{
	PrimaryActorTick.bCanEverTick = true;

	TargetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TargetRoot"));
	SetRootComponent(TargetRoot);
}

void AFPS_Target::BeginPlay()
{
	Super::BeginPlay();

	BaseScale = TargetRoot->GetRelativeScale3D();
}

void AFPS_Target::TakeHit_Implementation(float Damage, FVector HitPoint, FVector HitDirection)
{
	++HitCount;

	StartPhase(EPopPhase::Up);

	OnTargetHit.Broadcast(HitCount, HitPoint);

	if (HitCount >= HitsToDestroy && !DestroyTimerHandle.IsValid())
	{
		OnTargetDestroyed.Broadcast();
		GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AFPS_Target::HandleDestroy, FMath::Max(DestroyDelay, 0.01f), false);
	}
}

void AFPS_Target::StartPhase(EPopPhase NewPhase)
{
	Phase = NewPhase;
	PhaseElapsed = 0.0f;
	PhaseStartScale = CurrentScale;
}

float AFPS_Target::GetPhaseDuration() const
{
	switch (Phase)
	{
	case EPopPhase::Up:		return PopUpDuration;
	case EPopPhase::Down:	return PopDownDuration;
	case EPopPhase::Return:	return PopReturnDuration;
	default:				return 0.0f;
	}
}

float AFPS_Target::GetPhaseTargetScale() const
{
	switch (Phase)
	{
	case EPopPhase::Up:		return PopUpScale;
	case EPopPhase::Down:	return PopDownScale;
	case EPopPhase::Return:	return 1.0f;
	default:				return 1.0f;
	}
}

void AFPS_Target::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Phase == EPopPhase::Idle)
	{
		return;
	}

	PhaseElapsed += DeltaSeconds;

	const float Duration = FMath::Max(GetPhaseDuration(), KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(PhaseElapsed / Duration, 0.0f, 1.0f);

	CurrentScale = FMath::Lerp(PhaseStartScale, GetPhaseTargetScale(), Alpha);
	TargetRoot->SetRelativeScale3D(BaseScale * CurrentScale);

	if (Alpha < 1.0f)
	{
		return;
	}

	switch (Phase)
	{
	case EPopPhase::Up:
		StartPhase(EPopPhase::Down);
		break;

	case EPopPhase::Down:
		StartPhase(EPopPhase::Return);
		break;

	default:
		Phase = EPopPhase::Idle;
		CurrentScale = 1.0f;
		TargetRoot->SetRelativeScale3D(BaseScale);
		break;
	}
}

void AFPS_Target::HandleDestroy()
{
	Destroy();
}
