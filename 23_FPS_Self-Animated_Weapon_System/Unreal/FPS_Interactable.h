#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPS_Interactable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UFPS_Interactable : public UInterface
{
	GENERATED_BODY()
};

class MECHANICS_TEST_LVN_API IFPS_Interactable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Interactable")
	void OnFocusEnter();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Interactable")
	void OnFocusExit();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Interactable")
	void OnInteract(AActor* Interactor);
};
