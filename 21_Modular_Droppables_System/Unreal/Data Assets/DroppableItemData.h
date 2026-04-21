#pragma once

#include "CoreMinimal.h"
#include "ItemData.h"
#include "DroppableItemData.generated.h"

/**
 * Data asset for items that can be dropped in the world.
 */
UCLASS(BlueprintType)
class MECHANICS_TEST_LVN_API UDroppableItemData : public UItemData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop Settings")
	int32 BaseQuantity = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop Settings")
	int32 MinDropQuantity = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop Settings")
	int32 MaxDropQuantity = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collection Settings")
	float CollectDelay = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collection Settings")
	float CollectSpeed = 300.f;
};