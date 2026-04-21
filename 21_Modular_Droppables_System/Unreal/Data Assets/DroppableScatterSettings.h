#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DroppableScatterSettings.generated.h"

/**
 * Settings for the scatter effect on drop.
 */
UCLASS(BlueprintType)
class MECHANICS_TEST_LVN_API UDroppableScatterSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter")
	bool bEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter")
	float ForceMin = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter")
	float ForceMax = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter")
	float UpForce = 200.f;

	// Multiplier applied to scatter force to generate angular velocity
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter")
	float SpinForceMultiplier = 0.3f;
};