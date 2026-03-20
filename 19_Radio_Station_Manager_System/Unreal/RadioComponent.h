#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RadioStationManager.h"
#include "RadioComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRadioStateChanged, bool, bIsPlaying, int32, StationIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRadioClipChanged, USoundWave*, NewClip, FString, ClipName);

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API URadioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URadioComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Radio|Setup")
	void InitializeAudioComponent(UAudioComponent* InAudioComponent);

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void PlayRadio();

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void TurnOffRadio();

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void ChangeStation(int32 NewStationIndex);

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void NextTrack();

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void PreviousTrack();

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void NextStation();

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void PreviousStation();

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void RefreshCustomFolder();

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void OpenCustomMusicFolder();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio")
	bool IsRadioPlaying() const { return bIsPlaying; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio")
	int32 GetCurrentStationIndex() const { return CurrentStationIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio")
	USoundWave* GetCurrentClip() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio")
	float GetCurrentPlaybackTime() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio")
	int32 GetStationCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio")
	FString GetCurrentStationName() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Radio")
	FString GetCurrentClipName() const;

	UPROPERTY(BlueprintAssignable, Category = "Radio")
	FOnRadioStateChanged OnRadioStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Radio")
	FOnRadioClipChanged OnRadioClipChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio|Audio")
	float DefaultVolume = 0.75f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Radio|State")
	bool bIsPlaying = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Radio|State")
	int32 CurrentStationIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Radio|State")
	TArray<USoundWave*> SongHistory;

private:
	UPROPERTY()
	class UAudioComponent* AudioComponent;

	UPROPERTY()
	class URadioStationManager* RadioManager;

	// Guards CheckPlaybackState from interfering during intentional track switches
	bool bSwitchingTrack = false;

	FTimerHandle TimerHandle_PlaybackCheck;

	void PlayNextTrackInStation();

	UFUNCTION()
	void CheckPlaybackState();

	UFUNCTION()
	void OnAudioFinished();

	UFUNCTION()
	void OnManagerTrackChanged(int32 StationIndex, USoundWave* NewClip);
};