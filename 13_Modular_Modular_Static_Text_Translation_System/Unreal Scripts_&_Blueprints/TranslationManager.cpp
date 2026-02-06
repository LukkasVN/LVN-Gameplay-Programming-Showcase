#include "TranslationManager.h"
#include "Misc/ConfigCacheIni.h"

static const TCHAR* GLanguageSection = TEXT("Language");
static const TCHAR* GLanguageKey     = TEXT("CurrentLanguage");

void UTranslationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("TranslationManager instance: %s"), *GetClass()->GetName());

    LoadLanguage();

    OnLanguageChanged.Broadcast(CurrentLanguage);
}

void UTranslationManager::SetLanguage(ELanguage NewLanguage)
{
    if (CurrentLanguage == NewLanguage)
        return;

    CurrentLanguage = NewLanguage;
    SaveLanguage();

    OnLanguageChanged.Broadcast(NewLanguage);
}

void UTranslationManager::LoadLanguage()
{
    int32 SavedValue = static_cast<int32>(ELanguage::English);

    if (GConfig)
    {
        GConfig->GetInt(GLanguageSection, GLanguageKey, SavedValue, GGameUserSettingsIni);
    }

    if (StaticEnum<ELanguage>()->IsValidEnumValue(SavedValue))
        CurrentLanguage = static_cast<ELanguage>(SavedValue);
    else
        CurrentLanguage = ELanguage::English;
}

void UTranslationManager::SaveLanguage() const
{
    if (GConfig)
    {
        GConfig->SetInt(GLanguageSection, GLanguageKey, static_cast<int32>(CurrentLanguage), GGameUserSettingsIni);
        GConfig->Flush(false, GGameUserSettingsIni);
    }
}