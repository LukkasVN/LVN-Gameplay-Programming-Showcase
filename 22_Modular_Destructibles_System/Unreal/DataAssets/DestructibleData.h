// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DestructibleData.generated.h"

// DestructibleData.h
UCLASS(BlueprintType)
class MECHANICS_TEST_LVN_API UDestructibleData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity")
	FString DestructibleName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity", meta=(MultiLine=true))
	FString Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity")
	TObjectPtr<UTexture2D> UIIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats", meta=(ClampMin="1"))
	int32 HitPoints = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats", meta=(ClampMin="0"))
	int32 TierRequired = 0;  // 0 = any weapon

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit Pop Effect")
	bool bUseHitPopEffect = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit Pop Effect", meta=(ClampMin="0.05", ClampMax="0.5"))
	float PopEffectIntensity = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit Pop Effect", meta=(ClampMin="0.05", ClampMax="0.3"))
	float PopEffectDuration = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Destruction")
	TSubclassOf<AActor> DestroyedActorClass;  // optional spawned-on-destroy actor (Can be modified to be a list so you can randomize destroyed Actors)

	// EXPANSION: Audio / Particles
	// Wire through your own systems in ADestructibleActor:
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	// USoundBase* HitSFX;
	// USoundBase* DestroySFX;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Example Feedback")
	TObjectPtr<class UNiagaraSystem> HitParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Example Feedback")
	TObjectPtr<class UNiagaraSystem> DestroyParticle;
};
