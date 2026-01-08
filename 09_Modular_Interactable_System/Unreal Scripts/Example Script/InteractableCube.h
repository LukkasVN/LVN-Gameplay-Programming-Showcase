// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "Components/TextRenderComponent.h"
#include "InteractableCube.generated.h"

/**
 * 
 */

class UWidgetComponent;
class UTextBlock;

UCLASS()
class MECHANICS_TEST_LVN_API AInteractableCube : public AInteractable
{
	GENERATED_BODY()
protected:
	AInteractableCube();
	
	virtual void BeginPlay() override;
	void Tick(float DeltaTime);
	virtual void OnInteract() override;
	virtual void OnHoverEnter() override;
	virtual void OnHoverStay() override;
	virtual void OnHoverExit() override;

private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true"));
	UTextRenderComponent* HoverText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true"))
	UTextRenderComponent* HoverStayText;

	void ShowText(UTextRenderComponent* Text);
	void HideText(UTextRenderComponent* Text);

	void RotateText(float DeltaTime); // billboard toward player
	void SpinText(float DeltaTime); // rotate in place
		
	// Mesh reference for physics
	UPROPERTY()
	UStaticMeshComponent* Mesh;
};

