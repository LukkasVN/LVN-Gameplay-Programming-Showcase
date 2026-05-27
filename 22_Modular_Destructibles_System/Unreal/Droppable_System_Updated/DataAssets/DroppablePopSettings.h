#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DroppablePopSettings.generated.h"

/**
 * Settings for the pop scale effect on collection.
 */
UCLASS(BlueprintType)
class MECHANICS_TEST_LVN_API UDroppablePopSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pop")
	bool bEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pop")
	float Scale = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pop")
	float Duration = 0.15f;
};
