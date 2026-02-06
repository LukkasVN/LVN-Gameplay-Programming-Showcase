using UnityEngine;
using TMPro;

public class TranslatableStaticText : MonoBehaviour
{
    [SerializeField] LocalizedText localizedText;

    public bool customFont = false; // Set this to true if you want to use a custom font for the text, otherwise it will use the default font for the language.
    public TMP_FontAsset customFontAsset; // Assign your custom font asset in the inspector if customFont is true.
    TextMeshProUGUI tmpUGUI;
    TextMeshPro tmp;
    UnityEngine.UI.Text uiText;

    void Awake()
    {
        tmpUGUI = GetComponent<TextMeshProUGUI>();
        tmp = GetComponent<TextMeshPro>();
        uiText = GetComponent<UnityEngine.UI.Text>();
    }

    void Start()
    {
        if (TranslationManager.Instance != null)
        {
            TranslationManager.Instance.OnLanguageChanged += UpdateText;
            UpdateText(TranslationManager.Instance.CurrentLanguage);
        }
    }

    void UpdateText(LanguageEnum lang)
    {
        string value = localizedText.Get(lang);

        if (tmpUGUI != null)
            tmpUGUI.text = value;

        if (tmp != null)
            tmp.text = value;

        if (uiText != null)
            uiText.text = value;

        var font = TranslationManager.Instance.GetFontFor(lang);

        if (font != null && !customFont)
        {
            if (tmpUGUI != null)
                tmpUGUI.font = font;

            if (tmp != null)
                tmp.font = font;
        }
        else if (font != null && customFont)
        {
            if (tmpUGUI != null){
                tmpUGUI.font = customFontAsset;
            }
            if (tmp != null){
                tmp.font = customFontAsset;
            }
        }
    }

    public void PreviewLanguage(LanguageEnum lang)
    {
        UpdateText(lang);
    }

    public void RestoreCurrentLanguage()
    {
        UpdateText(TranslationManager.Instance.CurrentLanguage);
    }


}
