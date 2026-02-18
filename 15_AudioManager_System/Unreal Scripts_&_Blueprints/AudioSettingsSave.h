#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AudioSettingsSave.generated.h"

UCLASS()
class MECHANICS_TEST_LVN_API UAudioSettingsSave : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	float MasterVolume = 1.f;

	UPROPERTY()
	float MusicVolume = 1.f;

	UPROPERTY()
	float SFXVolume = 1.f;

	UPROPERTY()
	float UIVolume = 1.f;
};
