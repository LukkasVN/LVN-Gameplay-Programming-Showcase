// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_StartProneTransition.h"
#include "PlayerCharacter.h"

void UAnimNotify_StartProneTransition::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->StartProneTransition();
	}
}
