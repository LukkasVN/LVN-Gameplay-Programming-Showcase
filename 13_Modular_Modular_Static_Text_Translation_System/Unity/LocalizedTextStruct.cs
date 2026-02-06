using UnityEngine;

[System.Serializable]
public struct LocalizedText
{
    [Multiline]
    public string Galician;
    [Multiline]
    public string Spanish;
    [Multiline]
    public string English;
    [Multiline]
    public string French;
    [Multiline]
    public string German;
    [Multiline]
    public string Chinese;


    public string Get(LanguageEnum lang)
    {
        switch (lang)
        {
            case LanguageEnum.Galician: return Galician;
            case LanguageEnum.Spanish: return Spanish;
            case LanguageEnum.French: return French;
            case LanguageEnum.German: return German;
            case LanguageEnum.Chinese: return Chinese;
            //Feel free to expand with more languages but have in mind you have to update "LanguageEnum" as well.
            default: return English;
        }
    }
}
