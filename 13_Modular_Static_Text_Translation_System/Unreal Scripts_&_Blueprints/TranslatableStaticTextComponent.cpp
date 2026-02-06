#include "TranslatableStaticTextComponent.h"
#include "TranslationManager.h"
#include "Components/TextRenderComponent.h"

void UTranslatableStaticTextComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UTranslationManager* Manager = GI->GetSubsystem<UTranslationManager>())
            {
                Manager->OnLanguageChanged.AddDynamic(this, &UTranslatableStaticTextComponent::UpdateText);
                UpdateText(Manager->CurrentLanguage);
            }
        }
    }
}

void UTranslatableStaticTextComponent::UpdateText(ELanguage Lang)
{
    FString Value = LocalizedText.Get(Lang);

    UTranslationManager* Manager = GetWorld()->GetGameInstance()->GetSubsystem<UTranslationManager>();

    if (UTextRenderComponent* TR = GetOwner()->FindComponentByClass<UTextRenderComponent>())
    {
        TR->SetText(FText::FromString(Value));
    }

}
