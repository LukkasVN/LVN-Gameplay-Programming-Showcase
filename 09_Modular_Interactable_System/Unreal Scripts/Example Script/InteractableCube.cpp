// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableCube.h"

#include "Components/SphereComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"

#include "Components/TextRenderComponent.h"

AInteractableCube::AInteractableCube()
{
	HoverText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HoverText"));
	HoverStayText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HoverStayText"));

	HoverText->SetupAttachment(RootComponent);
	HoverStayText->SetupAttachment(RootComponent);

	HoverText->SetHorizontalAlignment(EHTA_Center);
	HoverStayText->SetHorizontalAlignment(EHTA_Center);

	HoverText->SetTextRenderColor(FColor::White);
	HoverStayText->SetTextRenderColor(FColor::Yellow);

	HoverText->SetWorldSize(20.f);
	HoverStayText->SetWorldSize(20.f);

	HoverText->SetVisibility(false);
	HoverStayText->SetVisibility(false);
}

void AInteractableCube::BeginPlay()
{
	Super::BeginPlay();

	Mesh = FindComponentByClass<UStaticMeshComponent>();
	EnablePhysicsMode(Mesh);

	if (Mesh)
	{
		HoverText->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
		HoverStayText->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void AInteractableCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsHovered){RotateText(DeltaTime);}
}


void AInteractableCube::OnInteract()
{
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(true);

		FVector RandomDir = FMath::VRand();
		RandomDir.Z = FMath::Abs(RandomDir.Z);

		Mesh->AddImpulse(RandomDir * 1000.f, NAME_None, true);

		// Optional: random torque
		FVector RandomTorque = FMath::VRand() * 20.f;
		Mesh->AddAngularImpulseInRadians(RandomTorque, NAME_None, true);
	}
}

void AInteractableCube::OnHoverEnter()
{
	ShowText(HoverText);
	ShowText(HoverStayText);
}

void AInteractableCube::OnHoverStay()
{
	float Delta = GetWorld()->GetDeltaSeconds();
	SpinText(Delta);
}

void AInteractableCube::OnHoverExit()
{
	HideText(HoverText);
	HideText(HoverStayText);
}

void AInteractableCube::ShowText(UTextRenderComponent* Text)
{
	if (Text)
		Text->SetVisibility(true);
}

void AInteractableCube::HideText(UTextRenderComponent* Text)
{
	if (Text)
		Text->SetVisibility(false);
}

void AInteractableCube::RotateText(float DeltaTime)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	FVector PlayerLocation = Pawn->GetActorLocation();

	auto Rotate = [&](UTextRenderComponent* Text)
	{
		if (!Text || !Text->IsVisible()) return;

		FVector Loc = Text->GetComponentLocation();
		FRotator Rot = (PlayerLocation - Loc).Rotation();
		Rot.Pitch = 0.f;
		Rot.Roll = 0.f;

		Text->SetWorldRotation(Rot);
	};

	Rotate(HoverText);
}


void AInteractableCube::SpinText(float DeltaTime)
{
	const float Speed = 120.f;

	auto Spin = [&](UTextRenderComponent* Text)
	{
		if (!Text || !Text->IsVisible()) return;

		FRotator Rot = Text->GetComponentRotation();
		Rot.Yaw += Speed * DeltaTime;
		Text->SetWorldRotation(Rot);
	};

	Spin(HoverStayText);
}




