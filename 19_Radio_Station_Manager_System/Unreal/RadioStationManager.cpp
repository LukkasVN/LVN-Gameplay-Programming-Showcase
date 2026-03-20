#include "RadioStationManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "TimerManager.h"
#include "Sound/SoundWaveProcedural.h"
#include "Misc/FileHelper.h"
#include "Sound/SoundAttenuation.h"

URadioStationManager::URadioStationManager()
	: Super()
{
}

void URadioStationManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Stations.Empty();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TimerHandle_TrackUpdate, this,
			&URadioStationManager::TrackActiveStations, 0.1f, true);

		World->GetTimerManager().SetTimer(TimerHandle_Cleanup, this,
			&URadioStationManager::CleanupInactiveStations, 1.0f, true);
	}

	SetupProceduralAttenuation();
}

void URadioStationManager::SetupProceduralAttenuation()
{
	ProceduralAttenuation = NewObject<USoundAttenuation>(GetTransientPackage());

	FSoundAttenuationSettings& Settings = ProceduralAttenuation->Attenuation;
	Settings.bAttenuate               = true;
	Settings.bSpatialize              = true;
	Settings.AttenuationShape         = EAttenuationShape::Sphere;
	Settings.AttenuationShapeExtents  = FVector(200.f, 0.f, 0.f);
	Settings.FalloffDistance          = 3000.f;
	Settings.dBAttenuationAtMax       = -60.f;
	Settings.DistanceAlgorithm        = EAttenuationDistanceModel::Logarithmic;
	Settings.SpatializationAlgorithm  = ESoundSpatializationAlgorithm::SPATIALIZATION_Default;
	Settings.bEnableOcclusion         = false;
	Settings.bEnableReverbSend        = false;
}

void URadioStationManager::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_TrackUpdate);
		World->GetTimerManager().ClearTimer(TimerHandle_Cleanup);
	}

	Stations.Empty();
	ActiveStations.Empty();
	PCMDataCache.Empty();
	TrackDisplayNames.Empty();

	Super::Deinitialize();
}

int32 URadioStationManager::GetStationCount() const
{
	return Stations.Num();
}

FRadioStation URadioStationManager::GetStation(int32 StationIndex) const
{
	return Stations.IsValidIndex(StationIndex) ? Stations[StationIndex] : FRadioStation();
}

TArray<FRadioStation> URadioStationManager::GetAllStations() const
{
	return Stations;
}

void URadioStationManager::AddStation(const FString& StationName, const TArray<USoundWave*>& Tracks)
{
	if (StationName.IsEmpty()) return;

	if (StationName.Equals(TEXT("Custom"), ESearchCase::IgnoreCase)) return;

	FRadioStation NewStation;
	NewStation.StationName = StationName;
	NewStation.Tracks      = Tracks;

	if (Stations.Num() > 0 && Stations.Last().StationName.Equals(TEXT("Custom")))
		Stations.Insert(NewStation, Stations.Num() - 1);
	else
		Stations.Add(NewStation);
}

void URadioStationManager::AddTrackToStation(int32 StationIndex, USoundWave* Track)
{
	if (!Stations.IsValidIndex(StationIndex) || !Track) return;
	Stations[StationIndex].Tracks.Add(Track);
}

void URadioStationManager::ClearAllStations()
{
	Stations.Empty();
	ActiveStations.Empty();
	PCMDataCache.Empty();
	TrackDisplayNames.Empty();

	FRadioStation CustomStation;
	CustomStation.StationName = TEXT("Custom");
	Stations.Add(CustomStation);
}

void URadioStationManager::SetStationActive(int32 StationIndex, UAudioComponent* AudioComponent)
{
	if (!Stations.IsValidIndex(StationIndex)) return;

	if (ActiveStations.Contains(StationIndex))
	{
		ActiveStations[StationIndex].LastAccessTime = FPlatformTime::Seconds();
	}
	else
	{
		FActiveStationData NewData;
		NewData.StationIndex      = StationIndex;
		NewData.LastAccessTime    = FPlatformTime::Seconds();
		NewData.LastUpdateTime    = FPlatformTime::Seconds();
		NewData.PlaybackStartTime = FPlatformTime::Seconds();
		ActiveStations.Add(StationIndex, NewData);
	}
}

void URadioStationManager::RegisterTrack(int32 StationIndex, USoundWave* Clip)
{
	if (!ActiveStations.Contains(StationIndex) || !Clip) return;

	FActiveStationData& Data  = ActiveStations[StationIndex];
	Data.CurrentClip          = Clip;
	Data.LastClip             = Clip;
	Data.PlaybackTime         = 0.0f;
	Data.PlaybackStartTime    = FPlatformTime::Seconds();
	Data.bIsPlaying           = true;
	Data.LastAccessTime       = FPlatformTime::Seconds();
	Data.LastUpdateTime       = FPlatformTime::Seconds();
}

void URadioStationManager::UpdateStationTime(int32 StationIndex, float Time, bool bIsPlaying)
{
	if (!ActiveStations.Contains(StationIndex)) return;

	FActiveStationData& Data = ActiveStations[StationIndex];
	Data.PlaybackTime        = Time;
	Data.bIsPlaying          = bIsPlaying;
	Data.LastAccessTime      = FPlatformTime::Seconds();
	Data.LastUpdateTime      = FPlatformTime::Seconds();
}

float URadioStationManager::GetStationTime(int32 StationIndex) const
{
	return ActiveStations.Contains(StationIndex) ? ActiveStations[StationIndex].PlaybackTime : 0.0f;
}

USoundWave* URadioStationManager::GetActiveStationClip(int32 StationIndex) const
{
	return ActiveStations.Contains(StationIndex) ? ActiveStations[StationIndex].CurrentClip : nullptr;
}

bool URadioStationManager::IsStationActive(int32 StationIndex) const
{
	return ActiveStations.Contains(StationIndex);
}

USoundWave* URadioStationManager::GetFreshClipForStation(int32 StationIndex) const
{
	if (!ActiveStations.Contains(StationIndex)) return nullptr;

	USoundWave* TemplateClip = ActiveStations[StationIndex].CurrentClip;
	if (!TemplateClip) return nullptr;

	// Check via pointer map — custom tracks decoded from WAV are registered here
	// PCMDataCache is keyed by filename, not by the auto-generated wave name
	if (const FString* TrackName = TrackDisplayNames.Find(TemplateClip))
	{
		float SavedTime = ActiveStations[StationIndex].PlaybackTime;
		return CreateProceduralWaveFromCache(*TrackName, SavedTime);
	}

	return TemplateClip;
}

FString URadioStationManager::GetTrackDisplayName(USoundWave* Clip) const
{
	if (!Clip) return FString(TEXT("Song ???"));

	if (const FString* DisplayName = TrackDisplayNames.Find(Clip))
		return *DisplayName;

	return Clip->GetName();
}

void URadioStationManager::AdvanceStationTrack(int32 StationIndex)
{
	if (!Stations.IsValidIndex(StationIndex) || !ActiveStations.Contains(StationIndex)) return;

	FRadioStation& Station = Stations[StationIndex];
	if (Station.Tracks.Num() == 0) return;

	FActiveStationData& ActiveData = ActiveStations[StationIndex];
	USoundWave* NewClip            = nullptr;
	int32 Attempts                 = 0;

	do
	{
		NewClip = Station.Tracks[FMath::RandRange(0, Station.Tracks.Num() - 1)];
		Attempts++;
	} while (NewClip == ActiveData.LastClip && Attempts < MAX_TRACK_ATTEMPTS);

	if (NewClip)
	{
		RegisterTrack(StationIndex, NewClip);
		OnTrackChanged.Broadcast(StationIndex, NewClip);
	}
}

void URadioStationManager::TrackActiveStations()
{
	double CurrentTime = FPlatformTime::Seconds();

	for (auto& [StationIndex, ActiveData] : ActiveStations)
	{
		if (ActiveData.bIsPlaying && ActiveData.CurrentClip)
		{
			ActiveData.PlaybackTime = static_cast<float>(CurrentTime - ActiveData.PlaybackStartTime);
			ActiveData.LastAccessTime = CurrentTime;
		}
	}
}

void URadioStationManager::CleanupInactiveStations()
{
	double CurrentTime = FPlatformTime::Seconds();
	TArray<int32> ToRemove;

	for (auto& [StationIndex, ActiveData] : ActiveStations)
	{
		if (CurrentTime - ActiveData.LastAccessTime > STATION_CLEANUP_TIME)
			ToRemove.Add(StationIndex);
	}

	for (int32 Index : ToRemove)
		ActiveStations.Remove(Index);
}

void URadioStationManager::CreateCustomStation()
{
	if (Stations.Num() > 0 && Stations.Last().StationName.Equals(TEXT("Custom"))) return;

	FRadioStation CustomStation;
	CustomStation.StationName = TEXT("Custom");
	Stations.Add(CustomStation);

	LoadCustomMusicFromFolder(GetCustomMusicFolderPath());
}

void URadioStationManager::RefreshCustomFolder()
{
	if (Stations.Num() == 0) return;

	int32 CustomIndex            = Stations.Num() - 1;
	FRadioStation& CustomStation = Stations[CustomIndex];

	if (!CustomStation.StationName.Equals(TEXT("Custom"))) return;

	CustomStation.Tracks.Empty();
	CustomStation.AudioFilePaths.Empty();
	PCMDataCache.Empty();
	TrackDisplayNames.Empty();

	if (ActiveStations.Contains(CustomIndex))
		ActiveStations.Remove(CustomIndex);

	LoadCustomMusicFromFolder(GetCustomMusicFolderPath());
}

FString URadioStationManager::GetCustomMusicFolderPath() const
{
	return FPaths::ProjectSavedDir() / TEXT("CustomGameRadioMusic");
}

void URadioStationManager::LoadCustomMusicFromFolder(const FString& FolderPath)
{
	if (FolderPath.IsEmpty() || Stations.Num() == 0) return;

	int32 CustomIndex            = Stations.Num() - 1;
	FRadioStation& CustomStation = Stations[CustomIndex];

	if (!CustomStation.StationName.Equals(TEXT("Custom"))) return;

	CustomStation.Tracks.Empty();
	CustomStation.AudioFilePaths.Empty();

	FString AbsolutePath = FPaths::ConvertRelativePathToFull(FolderPath);

	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*AbsolutePath)) return;

	class FAudioFileVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FAudioFileVisitor(FRadioStation& OutStation, URadioStationManager* Manager)
			: Station(OutStation), ManagerPtr(Manager) {}

		virtual bool Visit(const TCHAR* FilePath, bool bIsDirectory) override
		{
			if (!bIsDirectory && FPaths::GetExtension(FilePath, false).ToLower() == TEXT("wav"))
			{
				FString FileName = FPaths::GetBaseFilename(FilePath);
				USoundWave* Decoded = ManagerPtr->DecodeWAVFile(FString(FilePath), FileName);
				if (Decoded) Station.Tracks.Add(Decoded);
			}
			return true;
		}

	private:
		FRadioStation&        Station;
		URadioStationManager* ManagerPtr;
	};

	FAudioFileVisitor Visitor(CustomStation, this);
	FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*AbsolutePath, Visitor);
}

TArray<FString> URadioStationManager::GetCustomStationAudioFilePaths() const
{
	if (Stations.Num() > 0 && Stations.Last().StationName.Equals(TEXT("Custom")))
		return Stations.Last().AudioFilePaths;

	return TArray<FString>();
}

USoundWave* URadioStationManager::DecodeWAVFile(const FString& FilePath, const FString& FileName)
{
	TArray<uint8> WAVData;
	if (!FFileHelper::LoadFileToArray(WAVData, *FilePath)) return nullptr;
	if (WAVData.Num() < 44) return nullptr;

	if (WAVData[0] != 'R' || WAVData[1] != 'I' || WAVData[2] != 'F' || WAVData[3] != 'F' ||
		WAVData[8] != 'W' || WAVData[9] != 'A' || WAVData[10] != 'V' || WAVData[11] != 'E')
		return nullptr;

	FWaveFormatEx WaveFormat = {};
	bool  bFoundFmt          = false;
	int32 ChunkPos           = 12;

	while (ChunkPos + 8 <= WAVData.Num())
	{
		uint32 ChunkID = 0, ChunkSize = 0;
		FMemory::Memcpy(&ChunkID,   &WAVData[ChunkPos],     4);
		FMemory::Memcpy(&ChunkSize, &WAVData[ChunkPos + 4], 4);

		if (ChunkID == 0x20746d66)
		{
			if (ChunkPos + 8 + 16 > WAVData.Num()) return nullptr;
			const uint8* Fmt = &WAVData[ChunkPos + 8];
			FMemory::Memcpy(&WaveFormat.wFormatTag,      Fmt +  0, 2);
			FMemory::Memcpy(&WaveFormat.nChannels,       Fmt +  2, 2);
			FMemory::Memcpy(&WaveFormat.nSamplesPerSec,  Fmt +  4, 4);
			FMemory::Memcpy(&WaveFormat.nAvgBytesPerSec, Fmt +  8, 4);
			FMemory::Memcpy(&WaveFormat.nBlockAlign,     Fmt + 12, 2);
			FMemory::Memcpy(&WaveFormat.wBitsPerSample,  Fmt + 14, 2);
			bFoundFmt = true;
		}

		ChunkPos += 8 + static_cast<int32>(ChunkSize);
		if (bFoundFmt) break;
	}

	if (!bFoundFmt || WaveFormat.wFormatTag != 1 || WaveFormat.wBitsPerSample != 16) return nullptr;

	int32 AudioDataOffset = 0, AudioDataSize = 0;
	ChunkPos = 12;

	while (ChunkPos + 8 <= WAVData.Num())
	{
		uint32 ChunkID = 0, ChunkSize = 0;
		FMemory::Memcpy(&ChunkID,   &WAVData[ChunkPos],     4);
		FMemory::Memcpy(&ChunkSize, &WAVData[ChunkPos + 4], 4);

		if (ChunkID == 0x61746164)
		{
			AudioDataOffset = ChunkPos + 8;
			AudioDataSize   = static_cast<int32>(ChunkSize);
			break;
		}
		ChunkPos += 8 + static_cast<int32>(ChunkSize);
	}

	if (AudioDataSize <= 0 || AudioDataOffset + AudioDataSize > WAVData.Num()) return nullptr;

	FCachedPCMData CachedData;
	CachedData.PCMBytes.SetNumUninitialized(AudioDataSize);
	FMemory::Memcpy(CachedData.PCMBytes.GetData(), &WAVData[AudioDataOffset], AudioDataSize);
	CachedData.SampleRate  = WaveFormat.nSamplesPerSec;
	CachedData.NumChannels = WaveFormat.nChannels;
	CachedData.Duration    = static_cast<float>(AudioDataSize) / static_cast<float>(WaveFormat.nAvgBytesPerSec);
	PCMDataCache.Add(FileName, MoveTemp(CachedData));

	USoundWaveProcedural* SoundWave = CreateProceduralWaveFromCache(FileName);
	if (SoundWave)
	{
		// Store pointer -> filename so GetFreshClipForStation can identify custom tracks by pointer
		TrackDisplayNames.Add(SoundWave, FileName);
	}

	return SoundWave;
}

USoundWaveProcedural* URadioStationManager::CreateProceduralWaveFromCache(const FString& Name, float StartTimeSeconds) const
{
	const FCachedPCMData* Cached = PCMDataCache.Find(Name);
	if (!Cached) return nullptr;

	USoundWaveProcedural* Wave = NewObject<USoundWaveProcedural>(GetTransientPackage(), NAME_None);
	Wave->SetSampleRate(Cached->SampleRate);
	Wave->NumChannels        = Cached->NumChannels;
	Wave->Duration           = Cached->Duration;
	Wave->bLooping           = false;
	Wave->bStreaming          = false;
	Wave->SoundGroup         = SOUNDGROUP_Default;
	Wave->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;

	if (ProceduralAttenuation)
		Wave->AttenuationSettings = ProceduralAttenuation;

	const int32 BytesPerSecond = Cached->SampleRate * Cached->NumChannels * 2;
	const int32 BlockAlign     = Cached->NumChannels * 2;
	int32 ByteOffset           = FMath::FloorToInt(StartTimeSeconds * BytesPerSecond);
	ByteOffset = (ByteOffset / BlockAlign) * BlockAlign;
	ByteOffset = FMath::Clamp(ByteOffset, 0, Cached->PCMBytes.Num());

	Wave->QueueAudio(Cached->PCMBytes.GetData() + ByteOffset, Cached->PCMBytes.Num() - ByteOffset);

	return Wave;
}