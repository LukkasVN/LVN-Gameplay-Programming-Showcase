#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "SaveDataEntry.generated.h"

USTRUCT()
struct FSaveDataEntry
{
	GENERATED_BODY()

	UPROPERTY() FGuid GUID;
	UPROPERTY() FString Type;
	UPROPERTY() FString JsonData;
};

