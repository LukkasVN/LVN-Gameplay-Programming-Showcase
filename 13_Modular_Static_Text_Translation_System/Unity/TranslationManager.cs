using UnityEngine;
using System;
using TMPro;

/* --------------------------------------------------------------------------
   Unity Translation System
   Author: Lucas Varela Negro
   --------------------------------------------------------------------------

   Mini-Guide
   ----
   • TranslationManager (Singleton)
       - Loads saved language from PlayerPrefs
       - Stores current language
       - Allows different fonts per language
       - Provides GetFontFor(language)
       - Notifies listeners via OnLanguageChanged

   • LocalizedText (struct)
       - Holds per‑language strings
       - Get(lang) returns the correct one

   • TranslatableStaticText (component)
       - Subscribes to OnLanguageChanged
       - Updates text + font when language changes
       - Allows optional custom font override

   Usage
   -----
   1. Modify LocalizedText struct to add your desired languages.
   2. Create TranslationManager in your scene and set up language fonts.
   3. For each translatable text, add TranslatableStaticText component and fill in the LocalizedText fields.
   4. To change language at runtime, call TranslationManager.Instance.SetLanguage(newLanguage).
   5. (Optional) Use the customFont flag and customFont

   Hope this helps you easily manage static text translations in your Unity projects! Feel free to expand with more languages and features as needed ^^

-------------------------------------------------------------------------- */

[Serializable]
public struct LanguageFont
{
    public LanguageEnum language;
    public TMP_FontAsset font;
}

public class TranslationManager : MonoBehaviour
{
    public static TranslationManager Instance { get; private set; }

    public LanguageEnum CurrentLanguage { get; private set; }

    public event Action<LanguageEnum> OnLanguageChanged;

    [SerializeField] private LanguageFont[] languageFonts;

    const string PlayerPrefsKey = "SelectedLanguage";

    void Awake()
    {
        if (Instance != null && Instance != this)
        {
            Destroy(gameObject);
            return;
        }

        Instance = this;
        DontDestroyOnLoad(gameObject);

        LoadLanguage();
    }

    void LoadLanguage()
    {
        if (PlayerPrefs.HasKey(PlayerPrefsKey))
        {
            CurrentLanguage = (LanguageEnum)PlayerPrefs.GetInt(PlayerPrefsKey);
        }
        else
        {
            CurrentLanguage = LanguageEnum.English;
        }
    }

    public void SetLanguage(LanguageEnum newLanguage)
    {
        if (CurrentLanguage == newLanguage)
            return;

        CurrentLanguage = newLanguage;
        PlayerPrefs.SetInt(PlayerPrefsKey, (int)newLanguage);
        PlayerPrefs.Save();

        OnLanguageChanged?.Invoke(CurrentLanguage);
    }

    public TMP_FontAsset GetFontFor(LanguageEnum lang)
    {
        for (int i = 0; i < languageFonts.Length; i++)
        {
            if (languageFonts[i].language == lang)
                return languageFonts[i].font;
        }

        return null;
    }
}
