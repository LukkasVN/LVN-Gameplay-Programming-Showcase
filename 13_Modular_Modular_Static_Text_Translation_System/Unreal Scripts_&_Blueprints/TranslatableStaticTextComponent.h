#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LanguageEnum.h"
#include "LocalizedText.h"
#include "TranslatableStaticTextComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API UTranslatableStaticTextComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLocalizedText LocalizedText;

	virtual void BeginPlay() override;

	UFUNCTION()
	void UpdateText(ELanguage Lang);
};
