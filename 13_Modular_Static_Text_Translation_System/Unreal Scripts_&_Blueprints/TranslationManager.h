#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LanguageEnum.h"
#include "TranslationManager.generated.h"


/* --------------------------------------------------------------------------
   Unreal Static Text Translation System
   Author: Lucas Varela Negro
   --------------------------------------------------------------------------

   Mini‑Guide
   ----
   • FLocalizedText
	   - Stores per‑language strings
	   - Get(Language) returns the correct one

   • UTranslationManager (GameInstanceSubsystem)
	   - Loads/saves CurrentLanguage from GGameUserSettingsIni
	   - Broadcasts OnLanguageChanged when language changes

   • UTranslationBlueprintLibrary [Used in UMG Widgets Blueprints to preview/restore/update text]
	   - Applies localized text to UWidgets
	   - Supports preview + restore for UMG design time

   • UTranslatableStaticTextComponent
	   - Holds FLocalizedText
	   - Subscribes to OnLanguageChanged
	   - Updates in‑world text when language changes

   Usage
   -----
   1. UTranslationManager auto‑instantiates as a GameInstanceSubsystem. (If you have a custom GameInstance class, make sure to add it to the list of subsystems)
   2. Use UTranslationBlueprintLibrary to set or preview widget text.
   3. Add UTranslatableStaticTextComponent for in‑world text.
   4. Call SetLanguage() to update all subscribed texts.

	Hope this helps you implement a simple localization system in Unreal Engine ^^
	Feel free to expand it with anything you need, like more languages, custom fonts, etc.

   -------------------------------------------------------------------------- */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChanged, ELanguage, NewLanguage);

UCLASS(Blueprintable, BlueprintType)
class MECHANICS_TEST_LVN_API UTranslationManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnLanguageChanged OnLanguageChanged;

	UPROPERTY(BlueprintReadOnly)
	ELanguage CurrentLanguage = ELanguage::English; // Default to English

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	void SetLanguage(ELanguage NewLanguage);

private:

	void LoadLanguage();
	void SaveLanguage() const;
};
