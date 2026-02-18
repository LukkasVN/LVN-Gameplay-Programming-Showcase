#pragma once


#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Components/AudioComponent.h"
#include "AudioManagerSubsystem.generated.h"


UCLASS(BlueprintType)
class MECHANICS_TEST_LVN_API UAudioManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	// Must-set Sound Classes and Mixer for custom pitch & Audio
    UFUNCTION(BlueprintCallable)
    void SetVolume(USoundClass* Class, float NormalizedVolume);

    UFUNCTION(BlueprintCallable)
    void SetMainMix(USoundMix* Mix) { MainMix = Mix; }

    UFUNCTION(BlueprintCallable)
    void SetMasterClass(USoundClass* Class) { MasterClass = Class; }

    UFUNCTION(BlueprintCallable)
    void SetMusicClass(USoundClass* Class) { MusicClass = Class; }

    UFUNCTION(BlueprintCallable)
    void SetSFXClass(USoundClass* Class) { SFXClass = Class; }

    UFUNCTION(BlueprintCallable)
    void SetUIClass(USoundClass* Class) { UIClass = Class; }

    // Music Methods
    UFUNCTION(BlueprintCallable)
    void PlayMusic(USoundBase* NewTrack, float FadeTime = 1.f);

    UFUNCTION(BlueprintCallable)
    void StopMusic(float FadeTime = 1.f);

    // SFX & UI Play sound methods
    UFUNCTION(BlueprintCallable)
    void PlaySFX(USoundBase* Clip, float PitchRange = 0.0f);

    UFUNCTION(BlueprintCallable)
    void PlayUI(USoundBase* Clip, float PitchRange = 0.0f);

    // 3D Audio Play Method
    UFUNCTION(BlueprintCallable)
    void PlayAtLocation(USoundBase* Clip, FVector Position, float PitchRange = 0.3f);

    // Save & Load Methods for AudioSettings
    UFUNCTION(BlueprintCallable)
    void SaveAudioSettings(float Master, float Music, float SFX, float UI);

    UFUNCTION(BlueprintCallable)
    void LoadAudioSettings(float& Master, float& Music, float& SFX, float& UI);


private:
    UPROPERTY()
    USoundMix* MainMix = nullptr;

    UPROPERTY()
    USoundClass* MasterClass = nullptr;

    UPROPERTY()
    USoundClass* MusicClass = nullptr;

    UPROPERTY()
    USoundClass* SFXClass = nullptr;

    UPROPERTY()
    USoundClass* UIClass = nullptr;

    UPROPERTY()
    UAudioComponent* MusicA = nullptr;

    // 3D Audio Pool
    UPROPERTY()
    TArray<UAudioComponent*> SFXPool;

    int32 PoolIndex = 0;
    UAudioComponent* GetPooled3DSource();
};
