#include "RadioComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

URadioComponent::URadioComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URadioComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		RadioManager = World->GetSubsystem<URadioStationManager>();

		if (RadioManager)
			RadioManager->OnTrackChanged.AddDynamic(this, &URadioComponent::OnManagerTrackChanged);

		World->GetTimerManager().SetTimer(TimerHandle_PlaybackCheck, this,
			&URadioComponent::CheckPlaybackState, 0.5f, true);
	}
}

void URadioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (RadioManager)
		RadioManager->OnTrackChanged.RemoveDynamic(this, &URadioComponent::OnManagerTrackChanged);

	if (AudioComponent)
		AudioComponent->OnAudioFinished.RemoveDynamic(this, &URadioComponent::OnAudioFinished);

	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(TimerHandle_PlaybackCheck);

	Super::EndPlay(EndPlayReason);
}

void URadioComponent::InitializeAudioComponent(UAudioComponent* InAudioComponent)
{
	if (!InAudioComponent) return;

	AudioComponent = InAudioComponent;
	AudioComponent->bUseAttachParentBound = false;
	AudioComponent->bAllowSpatialization  = true;
	AudioComponent->SetVolumeMultiplier(DefaultVolume);

	if (RadioManager)
		AudioComponent->AttenuationSettings = RadioManager->GetRadioAttenuation();

	AudioComponent->OnAudioFinished.AddDynamic(this, &URadioComponent::OnAudioFinished);
}

void URadioComponent::PlayRadio()
{
	if (!RadioManager || !AudioComponent || bIsPlaying) return;

	if (RadioManager->GetStationCount() == 0) return;

	if (CurrentStationIndex >= RadioManager->GetStationCount())
		CurrentStationIndex = 0;

	if (RadioManager->GetStation(CurrentStationIndex).Tracks.Num() == 0) return;

	if (!RadioManager->IsStationActive(CurrentStationIndex))
		RadioManager->SetStationActive(CurrentStationIndex, AudioComponent);

	USoundWave* SavedClip = RadioManager->GetActiveStationClip(CurrentStationIndex);

	if (SavedClip)
	{
		USoundWave* ClipToPlay = RadioManager->GetFreshClipForStation(CurrentStationIndex);
		if (!ClipToPlay) return;

		if (SongHistory.Num() == 0 || SongHistory.Last() != SavedClip)
			SongHistory.Add(SavedClip);

		bIsPlaying = true;
		AudioComponent->SetSound(ClipToPlay);

		if (Cast<USoundWaveProcedural>(ClipToPlay))
			AudioComponent->Play();
		else
			AudioComponent->Play(RadioManager->GetStationTime(CurrentStationIndex));

		OnRadioStateChanged.Broadcast(bIsPlaying, CurrentStationIndex);
	}
	else
	{
		PlayNextTrackInStation();
	}
}

void URadioComponent::TurnOffRadio()
{
	if (!AudioComponent) return;

	bIsPlaying = false;
	AudioComponent->Stop();
	OnRadioStateChanged.Broadcast(bIsPlaying, CurrentStationIndex);
}

void URadioComponent::ChangeStation(int32 NewStationIndex)
{
	if (!RadioManager || !AudioComponent) return;
	if (!RadioManager->GetStation(NewStationIndex).Tracks.IsValidIndex(0)) return;

	if (bIsPlaying)
	{
		AudioComponent->Stop();
		bIsPlaying = false;
	}

	CurrentStationIndex = NewStationIndex;
	SongHistory.Empty();

	RadioManager->SetStationActive(CurrentStationIndex, AudioComponent);

	USoundWave* SavedClip = RadioManager->GetActiveStationClip(CurrentStationIndex);

	if (SavedClip)
	{
		USoundWave* ClipToPlay = RadioManager->GetFreshClipForStation(CurrentStationIndex);
		if (!ClipToPlay) return;

		SongHistory.Add(SavedClip);
		bIsPlaying = true;
		AudioComponent->SetSound(ClipToPlay);

		if (Cast<USoundWaveProcedural>(ClipToPlay))
			AudioComponent->Play();
		else
			AudioComponent->Play(RadioManager->GetStationTime(CurrentStationIndex));

		OnRadioStateChanged.Broadcast(bIsPlaying, CurrentStationIndex);
	}
	else
	{
		PlayNextTrackInStation();
	}
}

void URadioComponent::NextStation()
{
	if (!RadioManager) return;

	int32 Next = CurrentStationIndex + 1;
	if (Next >= RadioManager->GetStationCount()) Next = 0;
	ChangeStation(Next);
}

void URadioComponent::PreviousStation()
{
	if (!RadioManager) return;

	int32 Prev = CurrentStationIndex - 1;
	if (Prev < 0) Prev = RadioManager->GetStationCount() - 1;
	ChangeStation(Prev);
}

void URadioComponent::NextTrack()
{
	if (!RadioManager || !AudioComponent) return;
	if (RadioManager->GetStation(CurrentStationIndex).Tracks.Num() == 0) return;

	bSwitchingTrack = true;
	AudioComponent->Stop();
	RadioManager->AdvanceStationTrack(CurrentStationIndex);
}

void URadioComponent::PreviousTrack()
{
	if (!AudioComponent || !RadioManager || SongHistory.Num() < 2) return;

	bSwitchingTrack = true;
	AudioComponent->Stop();

	SongHistory.RemoveAt(SongHistory.Num() - 1);
	USoundWave* PrevClip = SongHistory.Last();

	if (PrevClip)
	{
		RadioManager->RegisterTrack(CurrentStationIndex, PrevClip);

		USoundWave* ClipToPlay = RadioManager->GetFreshClipForStation(CurrentStationIndex);
		if (ClipToPlay)
		{
			OnRadioClipChanged.Broadcast(PrevClip, RadioManager->GetTrackDisplayName(PrevClip));
			AudioComponent->SetSound(ClipToPlay);
			AudioComponent->Play();
		}
	}

	bSwitchingTrack = false;
}

void URadioComponent::PlayNextTrackInStation()
{
	if (!RadioManager || !AudioComponent) return;
	if (RadioManager->GetStation(CurrentStationIndex).Tracks.Num() == 0) return;

	if (!RadioManager->IsStationActive(CurrentStationIndex))
		RadioManager->SetStationActive(CurrentStationIndex, AudioComponent);

	bSwitchingTrack = true;
	bIsPlaying = true; 
	AudioComponent->Stop();
	RadioManager->AdvanceStationTrack(CurrentStationIndex);
}

void URadioComponent::OnManagerTrackChanged(int32 StationIndex, USoundWave* NewClip)
{
	if (StationIndex != CurrentStationIndex || !NewClip || !AudioComponent) return;

	if (!bIsPlaying && !bSwitchingTrack) return;

	AudioComponent->Stop();
	SongHistory.Add(NewClip);

	USoundWave* ClipToPlay = RadioManager->GetFreshClipForStation(CurrentStationIndex);
	if (ClipToPlay)
	{
		OnRadioClipChanged.Broadcast(NewClip, RadioManager->GetTrackDisplayName(NewClip));
		bIsPlaying = true;
		AudioComponent->SetSound(ClipToPlay);
		AudioComponent->Play();
	}

	bSwitchingTrack = false;
}

void URadioComponent::OnAudioFinished()
{
	if (!bIsPlaying || !RadioManager) return;

	USoundWave* CurrentClip = RadioManager->GetActiveStationClip(CurrentStationIndex);
	if (CurrentClip && !Cast<USoundWaveProcedural>(CurrentClip))
	{
		float PlaybackTime = RadioManager->GetStationTime(CurrentStationIndex);
		float Duration     = CurrentClip->Duration;

		if (Duration > 0.f && PlaybackTime < Duration - 2.f)
			return;
	}

	PlayNextTrackInStation();
}

void URadioComponent::CheckPlaybackState()
{
	if (!bIsPlaying || !AudioComponent || !RadioManager || bSwitchingTrack) return;
	if (AudioComponent->IsPlaying()) return;

	USoundWave* CurrentClip = RadioManager->GetActiveStationClip(CurrentStationIndex);
	if (!CurrentClip) return;

	float PlaybackTime = RadioManager->GetStationTime(CurrentStationIndex);
	float Duration     = CurrentClip->Duration;

	// Natural end fallback - OnAudioFinished is unreliable for procedural waves
	if (Duration > 0.f && PlaybackTime >= Duration - 2.f)
	{
		PlayNextTrackInStation();
		return;
	}

	// Unexpected stop - resume from saved time (distance cull recovery)
	USoundWave* ClipToPlay = RadioManager->GetFreshClipForStation(CurrentStationIndex);
	if (ClipToPlay)
	{
		AudioComponent->SetSound(ClipToPlay);

		if (Cast<USoundWaveProcedural>(ClipToPlay))
			AudioComponent->Play();
		else
			AudioComponent->Play(PlaybackTime);
	}
}

void URadioComponent::RefreshCustomFolder()
{
	if (RadioManager) RadioManager->RefreshCustomFolder();
}

void URadioComponent::OpenCustomMusicFolder()
{
	if (!RadioManager) return;

	FString CustomMusicPath = RadioManager->GetCustomMusicFolderPath();

	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*CustomMusicPath))
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*CustomMusicPath);

	RadioManager->LoadCustomMusicFromFolder(CustomMusicPath);
	FPlatformProcess::ExploreFolder(*FPaths::ConvertRelativePathToFull(CustomMusicPath));
}

USoundWave* URadioComponent::GetCurrentClip() const
{
	return AudioComponent ? Cast<USoundWave>(AudioComponent->GetSound()) : nullptr;
}

float URadioComponent::GetCurrentPlaybackTime() const
{
	return RadioManager ? RadioManager->GetStationTime(CurrentStationIndex) : 0.0f;
}

int32 URadioComponent::GetStationCount() const
{
	return RadioManager ? RadioManager->GetStationCount() : 0;
}

FString URadioComponent::GetCurrentStationName() const
{
	if (RadioManager)
		return RadioManager->GetStation(CurrentStationIndex).StationName;
	return FString(TEXT("Unknown"));
}

FString URadioComponent::GetCurrentClipName() const
{
	if (RadioManager)
	{
		USoundWave* Clip = RadioManager->GetActiveStationClip(CurrentStationIndex);
		if (Clip) return RadioManager->GetTrackDisplayName(Clip);
	}
	return FString(TEXT("Song ???"));
}