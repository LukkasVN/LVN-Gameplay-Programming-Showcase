#pragma once
#include "LanguageEnum.h"
#include "LocalizedText.generated.h"

USTRUCT(BlueprintType)
struct FLocalizedText
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Galician;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Spanish;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString English;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString French;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString German;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Chinese;

	FString Get(ELanguage Lang) const
	{
		switch (Lang)
		{
		case ELanguage::Galician: return Galician;
		case ELanguage::Spanish: return Spanish;
		case ELanguage::French: return French;
		case ELanguage::German: return German;
		case ELanguage::Chinese: return Chinese;
		default: return English;
		}
	}
};
