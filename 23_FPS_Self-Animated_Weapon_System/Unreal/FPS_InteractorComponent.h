#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "FPS_InteractorComponent.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFPS_OnInteractFocusChanged, AActor*, FocusedActor, const FText&, Prompt);

UCLASS(ClassGroup = (FPS), meta = (BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API UFPS_InteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFPS_InteractorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Broadcasts a null actor and an empty text when focus is lost. */
	UPROPERTY(BlueprintAssignable, Category = "FPS|Interact|Events")
	FFPS_OnInteractFocusChanged OnFocusChanged;

	UFUNCTION(BlueprintPure, Category = "FPS|Interact")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	UFUNCTION(BlueprintCallable, Category = "FPS|Interact")
	void TryInteract();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "FPS|Interact")
	float InteractDistance = 320.0f;

	/** A dedicated channel, so interactables are not filtered out of a generic visibility trace. */
	UPROPERTY(EditAnywhere, Category = "FPS|Interact")
	TEnumAsByte<ECollisionChannel> InteractTraceChannel = ECC_GameTraceChannel2;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputMappingContext> InteractMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	int32 MappingContextPriority = 1;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Input")
	TObjectPtr<UInputAction> InteractAction;

private:
	void BindInput();

	UFUNCTION()
	void HandlePawnRestarted(APawn* Pawn);
	void UpdateFocus();
	void SetFocus(AActor* NewFocus);
	UCameraComponent* ResolveCamera();

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> FocusedActor;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> CachedCamera;

	bool bInputBound = false;
};
