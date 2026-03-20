#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundWaveProcedural.h"
#include "Sound/SoundAttenuation.h"
#include "Containers/Map.h"
#include "RadioStationManager.generated.h"

USTRUCT(BlueprintType)
struct FRadioStation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StationName = "Station 1";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USoundWave*> Tracks;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FString> AudioFilePaths;
};

USTRUCT(BlueprintType)
struct FActiveStationData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 StationIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USoundWave* CurrentClip = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float PlaybackTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsPlaying = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double LastAccessTime = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double LastUpdateTime = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 TracksPlayedCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USoundWave* LastClip = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double PlaybackStartTime = 0.0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrackChanged, int32, StationIndex, USoundWave*, NewClip);

UCLASS()
class MECHANICS_TEST_LVN_API URadioStationManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	URadioStationManager();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Station Management
	UFUNCTION(BlueprintCallable, Category = "Radio|Setup")
	void AddStation(const FString& StationName, const TArray<USoundWave*>& Tracks);

	UFUNCTION(BlueprintCallable, Category = "Radio|Setup")
	void AddTrackToStation(int32 StationIndex, USoundWave* Track);

	UFUNCTION(BlueprintCallable, Category = "Radio|Setup")
	void ClearAllStations();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio|Setup")
	TArray<FRadioStation> GetAllStations() const;

	UFUNCTION(BlueprintCallable, Category = "Radio")
	int32 GetStationCount() const;

	UFUNCTION(BlueprintCallable, Category = "Radio")
	FRadioStation GetStation(int32 StationIndex) const;

	// Active Station Tracking
	UFUNCTION(BlueprintCallable, Category = "Radio")
	void SetStationActive(int32 StationIndex, class UAudioComponent* AudioComponent);

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void RegisterTrack(int32 StationIndex, USoundWave* Clip);

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void UpdateStationTime(int32 StationIndex, float Time, bool bIsPlaying);

	UFUNCTION(BlueprintCallable, Category = "Radio")
	float GetStationTime(int32 StationIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Radio")
	USoundWave* GetActiveStationClip(int32 StationIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Radio")
	bool IsStationActive(int32 StationIndex) const;

	// Returns a ready-to-play wave — fresh procedural for custom tracks, direct ref for engine assets
	UFUNCTION(BlueprintCallable, Category = "Radio")
	USoundWave* GetFreshClipForStation(int32 StationIndex) const;

	// Picks next random track and broadcasts to all radios on this station
	UFUNCTION(BlueprintCallable, Category = "Radio")
	void AdvanceStationTrack(int32 StationIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio")
	FString GetTrackDisplayName(USoundWave* Clip) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio")
	USoundAttenuation* GetRadioAttenuation() const { return ProceduralAttenuation; }

	UPROPERTY(BlueprintAssignable, Category = "Radio")
	FOnTrackChanged OnTrackChanged;

	// Custom Station
	UFUNCTION(BlueprintCallable, Category = "Radio|Setup")
	void CreateCustomStation();

	UFUNCTION(BlueprintCallable, Category = "Radio|Custom")
	TArray<FString> GetCustomStationAudioFilePaths() const;

	UFUNCTION(BlueprintCallable, Category = "Radio|Custom")
	void RefreshCustomFolder();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio|Custom")
	FString GetCustomMusicFolderPath() const;

	UFUNCTION(BlueprintCallable, Category = "Radio|Custom")
	void LoadCustomMusicFromFolder(const FString& FolderPath);

protected:
	struct FWaveFormatEx
	{
		uint16 wFormatTag;
		uint16 nChannels;
		uint32 nSamplesPerSec;
		uint32 nAvgBytesPerSec;
		uint16 nBlockAlign;
		uint16 wBitsPerSample;
	};

	struct FCachedPCMData
	{
		TArray<uint8> PCMBytes;
		uint32        SampleRate  = 0;
		uint16        NumChannels = 0;
		float         Duration    = 0.f;
	};

	TMap<FString, FCachedPCMData> PCMDataCache;

	USoundWave* DecodeWAVFile(const FString& FilePath, const FString& FileName);
	USoundWaveProcedural* CreateProceduralWaveFromCache(const FString& Name, float StartTimeSeconds = 0.f) const;

private:
	UPROPERTY()
	TArray<FRadioStation> Stations;

	UPROPERTY()
	TMap<int32, FActiveStationData> ActiveStations;

	UPROPERTY()
	USoundAttenuation* ProceduralAttenuation = nullptr;

	// Maps template wave pointer -> original filename, used for display and cache lookup
	TMap<USoundWave*, FString> TrackDisplayNames;

	FTimerHandle TimerHandle_TrackUpdate;
	FTimerHandle TimerHandle_Cleanup;

	void SetupProceduralAttenuation();
	void TrackActiveStations();
	void CleanupInactiveStations();

	static constexpr float STATION_CLEANUP_TIME = 180.0f;
	static constexpr int32 MAX_TRACK_ATTEMPTS   = 5;
};