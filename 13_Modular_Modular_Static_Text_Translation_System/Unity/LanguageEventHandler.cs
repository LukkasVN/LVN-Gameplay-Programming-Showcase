using UnityEngine;

public class LanguageEventHandler : MonoBehaviour
{
    [SerializeField] LanguageEnum language;
    [SerializeField] private TranslatableStaticText[] previewableTexts;

    public void ApplyLanguage()
    {
        TranslationManager.Instance.SetLanguage(language);
    }

    public void Preview()
    {
        foreach (var t in previewableTexts) t.PreviewLanguage(language);
    }
    public void Restore()
    {
        foreach (var t in previewableTexts) t.RestoreCurrentLanguage();
    }
}
