#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveDataEntry.h"
#include "SaveGameData.generated.h"

UCLASS()
class MECHANICS_TEST_LVN_API USaveGameData : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() TArray<FSaveDataEntry> Entries;
};
