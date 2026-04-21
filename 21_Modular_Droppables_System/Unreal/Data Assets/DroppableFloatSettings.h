#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DroppableFloatSettings.generated.h"

/**
 * Settings for the floating idle effect after landing.
 */
UCLASS(BlueprintType)
class MECHANICS_TEST_LVN_API UDroppableFloatSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Float")
	bool bEnabled = true;

	// Height above ground contact point to hover at
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Float")
	float GroundOffset = 10.f;

	// Sine wave amplitude
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Float")
	float BobHeight = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Float")
	float BobSpeed = 1.5f;

	// Degrees per second
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Float")
	float RotationSpeed = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Float")
	float TransitionDuration = 0.4f;
};