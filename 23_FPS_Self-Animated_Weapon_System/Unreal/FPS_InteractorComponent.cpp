#include "FPS_InteractorComponent.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "FPS_InteractPrompt.h"
#include "FPS_Interactable.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UFPS_InteractorComponent::UFPS_InteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFPS_InteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedCamera = ResolveCamera();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("UFPS_InteractorComponent expects a Pawn owner, found %s."), *GetNameSafe(GetOwner()));
		return;
	}

	if (OwnerPawn->InputComponent)
	{
		BindInput();
	}
	else
	{
		OwnerPawn->ReceiveRestartedDelegate.AddDynamic(this, &UFPS_InteractorComponent::HandlePawnRestarted);
	}
}

void UFPS_InteractorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetFocus(nullptr);

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->ReceiveRestartedDelegate.RemoveDynamic(this, &UFPS_InteractorComponent::HandlePawnRestarted);
	}

	Super::EndPlay(EndPlayReason);
}

void UFPS_InteractorComponent::HandlePawnRestarted(APawn* Pawn)
{
	BindInput();
}

void UFPS_InteractorComponent::BindInput()
{
	if (bInputBound)
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	if (const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (InteractMappingContext)
			{
				Subsystem->AddMappingContext(InteractMappingContext, MappingContextPriority);
			}
		}
	}

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(OwnerPawn->InputComponent);
	if (!EnhancedInput || !InteractAction)
	{
		return;
	}

	EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &UFPS_InteractorComponent::TryInteract);
	bInputBound = true;
}

UCameraComponent* UFPS_InteractorComponent::ResolveCamera()
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UCameraComponent>() : nullptr;
}

void UFPS_InteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateFocus();
}

void UFPS_InteractorComponent::UpdateFocus()
{
	if (!CachedCamera)
	{
		CachedCamera = ResolveCamera();
		if (!CachedCamera)
		{
			return;
		}
	}

	const FVector Start = CachedCamera->GetComponentLocation();
	const FVector End = Start + CachedCamera->GetForwardVector() * InteractDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FPS_Interact), false, GetOwner());

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, InteractTraceChannel, Params);

	AActor* Candidate = nullptr;
	if (bHit && Hit.GetActor() && Hit.GetActor()->Implements<UFPS_Interactable>())
	{
		Candidate = Hit.GetActor();
	}

	SetFocus(Candidate);
}

void UFPS_InteractorComponent::SetFocus(AActor* NewFocus)
{
	AActor* Previous = FocusedActor.Get();
	if (Previous == NewFocus)
	{
		return;
	}

	if (Previous && Previous->Implements<UFPS_Interactable>())
	{
		IFPS_Interactable::Execute_OnFocusExit(Previous);
	}

	FocusedActor = NewFocus;

	FText Prompt = FText::GetEmpty();
	if (NewFocus)
	{
		IFPS_Interactable::Execute_OnFocusEnter(NewFocus);

		// A prompt is optional, so it lives behind its own interface instead of
		// forcing every interactable to answer.
		if (NewFocus->Implements<UFPS_InteractPrompt>())
		{
			Prompt = IFPS_InteractPrompt::Execute_GetInteractPrompt(NewFocus);
		}
	}

	OnFocusChanged.Broadcast(NewFocus, Prompt);
}

void UFPS_InteractorComponent::TryInteract()
{
	AActor* Target = FocusedActor.Get();
	if (!Target || !Target->Implements<UFPS_Interactable>())
	{
		return;
	}

	IFPS_Interactable::Execute_OnInteract(Target, GetOwner());
}
