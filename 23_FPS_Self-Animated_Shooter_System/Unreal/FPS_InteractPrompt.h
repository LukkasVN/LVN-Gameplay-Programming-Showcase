#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPS_InteractPrompt.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UFPS_InteractPrompt : public UInterface
{
	GENERATED_BODY()
};

class MECHANICS_TEST_LVN_API IFPS_InteractPrompt
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Interactable")
	FText GetInteractPrompt();
};
