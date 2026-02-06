#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LocalizedText.h"
#include "LanguageEnum.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableText.h"
#include "Components/MultiLineEditableTextBox.h"
#include "TranslationBPFunctionLibrary.generated.h"

UCLASS()
class MECHANICS_TEST_LVN_API UTranslationBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:

	static void ApplyTextToWidget(UWidget* Widget, const FString& Value);

public:

	UFUNCTION(BlueprintCallable, Category="Localization")
	static void PreviewWidgetText(UWidget* Widget, const FLocalizedText& Localized, ELanguage PreviewLang);

	UFUNCTION(BlueprintCallable, Category="Localization")
	static void RestoreWidgetText(UWidget* Widget, const FLocalizedText& Localized);
};



