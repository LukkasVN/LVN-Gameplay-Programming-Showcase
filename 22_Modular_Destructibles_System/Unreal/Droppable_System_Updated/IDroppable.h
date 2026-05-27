#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IDroppable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UDroppable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that can be dropped and picked up.
 */
class MECHANICS_TEST_LVN_API IDroppable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Droppable")
	UDroppableItemData* GetItemData() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Droppable")
	bool GetIsCollectible() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Droppable")
	bool GetIsBeingCollected() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Droppable")
	void SetItemData(UDroppableItemData* Data);
};