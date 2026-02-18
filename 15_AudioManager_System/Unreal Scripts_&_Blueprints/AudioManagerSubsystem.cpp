#include "AudioManagerSubsystem.h"
#include "AudioSettingsSave.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

void UAudioManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UWorld* World = GetWorld();
    if (!World) return;

    UObject* Outer = World->GetWorldSettings();

    MusicA = NewObject<UAudioComponent>(Outer);
    MusicA->bAutoActivate = false;
    MusicA->RegisterComponent();
    MusicA->SetVolumeMultiplier(1.f);

    if (MainMix)
    {
        UGameplayStatics::PushSoundMixModifier(World, MainMix);
    }

    AWorldSettings* WS = World->GetWorldSettings();
    USceneComponent* Root = WS->GetRootComponent();

    // 3D SFX pool
    for (int32 i = 0; i < 16; ++i)
    {
        UAudioComponent* Comp = NewObject<UAudioComponent>(Outer);
        Comp->bAutoActivate = false;
        Comp->SetupAttachment(Root);
        Comp->RegisterComponent();
        Comp->bAllowSpatialization = true;
        Comp->bOverrideAttenuation = true;
        Comp->AttenuationOverrides.bAttenuate = true;
        Comp->AttenuationOverrides.bSpatialize = true;
        Comp->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
        Comp->AttenuationOverrides.AttenuationShapeExtents = FVector(400.f); // radius
        Comp->AttenuationOverrides.FalloffDistance = 2000.f; // Audio Falloff Distance
        
        SFXPool.Add(Comp);
    }

}

void UAudioManagerSubsystem::SetVolume(USoundClass* Class, float NormalizedVolume)
{
    if (!Class || !MainMix || !GetWorld()) return;

    UGameplayStatics::SetSoundMixClassOverride(
        GetWorld(),
        MainMix,
        Class,
        NormalizedVolume,
        1.0f,
        0.1f,
        true
    );

    UGameplayStatics::PushSoundMixModifier(GetWorld(), MainMix);
}

void UAudioManagerSubsystem::PlayMusic(USoundBase* NewTrack, float FadeTime)
{
    if (!NewTrack || !MusicA || !GetWorld()) return;

    MusicA->Stop();
    MusicA->SetSound(NewTrack);
    MusicA->SetVolumeMultiplier(1.f);

    if (FadeTime > 0.f)
    {
        MusicA->FadeIn(FadeTime, 1.f);
    }
    else
    {
        MusicA->Play();
    }
}

void UAudioManagerSubsystem::StopMusic(float FadeTime)
{
    if (!MusicA || !GetWorld()) return;

    if (FadeTime > 0.f)
    {
        MusicA->FadeOut(FadeTime, 0.f);
    }
    else
    {
        MusicA->Stop();
    }
}

void UAudioManagerSubsystem::PlaySFX(USoundBase* Clip, float PitchRange)
{
    if (!Clip || !GetWorld()) return;

    const float Pitch = FMath::FRandRange(1.f - PitchRange, 1.f + PitchRange);
    UGameplayStatics::PlaySound2D(GetWorld(), Clip, 1.f, Pitch);
}

void UAudioManagerSubsystem::PlayUI(USoundBase* Clip, float PitchRange)
{
    if (!Clip || !GetWorld()) return;

    const float Pitch = FMath::FRandRange(1.f - PitchRange, 1.f + PitchRange);
    UGameplayStatics::PlaySound2D(GetWorld(), Clip, 1.f, Pitch);
}

UAudioComponent* UAudioManagerSubsystem::GetPooled3DSource()
{
    if (SFXPool.Num() == 0) return nullptr;

    UAudioComponent* Comp = SFXPool[PoolIndex];
    PoolIndex = (PoolIndex + 1) % SFXPool.Num();
    return Comp;
}

void UAudioManagerSubsystem::PlayAtLocation(USoundBase* Clip, FVector Position, float PitchRange)
{
    if (!Clip) return;

    UAudioComponent* Comp = GetPooled3DSource();
    if (!Comp) return;

    Comp->SetWorldLocation(Position);

    const float Pitch = FMath::FRandRange(1.f - PitchRange, 1.f + PitchRange);
    Comp->SetPitchMultiplier(Pitch);

    Comp->SetSound(Clip);
    Comp->Play();
}

void UAudioManagerSubsystem::SaveAudioSettings(float Master, float Music, float SFX, float UI)
{
    UAudioSettingsSave* SaveObj = Cast<UAudioSettingsSave>(
        UGameplayStatics::CreateSaveGameObject(UAudioSettingsSave::StaticClass())
    );

    SaveObj->MasterVolume = Master;
    SaveObj->MusicVolume  = Music;
    SaveObj->SFXVolume    = SFX;
    SaveObj->UIVolume     = UI;

    UGameplayStatics::SaveGameToSlot(SaveObj, TEXT("AudioSettings"), 0);
}

void UAudioManagerSubsystem::LoadAudioSettings(float& Master, float& Music, float& SFX, float& UI)
{
    if (UGameplayStatics::DoesSaveGameExist(TEXT("AudioSettings"), 0))
    {
        UAudioSettingsSave* SaveObj = Cast<UAudioSettingsSave>(
            UGameplayStatics::LoadGameFromSlot(TEXT("AudioSettings"), 0)
        );

        Master = SaveObj->MasterVolume;
        Music  = SaveObj->MusicVolume;
        SFX    = SaveObj->SFXVolume;
        UI     = SaveObj->UIVolume;
    }
    else
    {
        Master = Music = SFX = UI = 100.f;
    }
}

