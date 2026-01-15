// Fill out your copyright notice in the Description page of Project Settings.


#include "ReferencesHUD.h"
#include "DialogueWidget.h"

// References the DialogueWidget instance
void AReferencesHUD::ReferenceWidget(UDialogueWidget* Widget)
{
	DialogueWidget = Widget;

	if (!DialogueWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReferencesHUD: DialogueWidget is null"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ReferencesHUD: DialogueWidget asigned CORRECTLY"));
		DialogueWidget->SetVisibility(ESlateVisibility::Visible);
	}
}




