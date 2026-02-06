#include "TranslationBPFunctionLibrary.h"

#include "TranslationManager.h"

void UTranslationBlueprintLibrary::ApplyTextToWidget(UWidget* Widget, const FString& Value)
{
	if (!Widget) return;

	if (UTextBlock* TB = Cast<UTextBlock>(Widget))
	{
		TB->SetText(FText::FromString(Value));
		return;
	}

	if (URichTextBlock* RTB = Cast<URichTextBlock>(Widget))
	{
		RTB->SetText(FText::FromString(Value));
		return;
	}

	if (UEditableText* ET = Cast<UEditableText>(Widget))
	{
		ET->SetText(FText::FromString(Value));
		return;
	}

	if (UEditableTextBox* ETB = Cast<UEditableTextBox>(Widget))
	{
		ETB->SetText(FText::FromString(Value));
		return;
	}

	if (UMultiLineEditableText* MET = Cast<UMultiLineEditableText>(Widget))
	{
		MET->SetText(FText::FromString(Value));
		return;
	}

	if (UMultiLineEditableTextBox* METB = Cast<UMultiLineEditableTextBox>(Widget))
	{
		METB->SetText(FText::FromString(Value));
		return;
	}
}

void UTranslationBlueprintLibrary::PreviewWidgetText(UWidget* Widget, const FLocalizedText& Localized, ELanguage PreviewLang)
{
	ApplyTextToWidget(Widget, Localized.Get(PreviewLang));
}

void UTranslationBlueprintLibrary::RestoreWidgetText(UWidget* Widget, const FLocalizedText& Localized)
{
	if (!Widget) return;

	if (UWorld* World = Widget->GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UTranslationManager* Manager = GI->GetSubsystem<UTranslationManager>())
			{
				ApplyTextToWidget(Widget, Localized.Get(Manager->CurrentLanguage));
			}
		}
	}
}


