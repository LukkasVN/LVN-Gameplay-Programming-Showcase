#pragma once

#include "UObject/Interface.h"
#include "FP_Interactable.generated.h"

UINTERFACE(Blueprintable)
class MECHANICS_TEST_LVN_API UFP_Interactable : public UInterface
{
	GENERATED_BODY()
};

class IFP_Interactable
{
	GENERATED_BODY()

public:

	// Called when player interacts
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void Interact(AActor* Interactor);

	// Optional: called when player looks at object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void OnFocus(AActor* Interactor);

	// Optional: called when player stops looking
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void OnUnfocus(AActor* Interactor);
};

