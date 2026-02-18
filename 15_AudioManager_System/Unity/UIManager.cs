using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class UIManager : MonoBehaviour
{
    [Header("Sliders")]
    [SerializeField] private Slider masterSlider;
    [SerializeField] private Slider musicSlider;
    [SerializeField] private Slider sfxSlider;
    [SerializeField] private Slider uiSlider;

    [Header("Labels")]
    [SerializeField] private TextMeshProUGUI masterText;
    [SerializeField] private TextMeshProUGUI musicText;
    [SerializeField] private TextMeshProUGUI sfxText;
    [SerializeField] private TextMeshProUGUI uiText;

    private AudioManager audioManager => AudioManager.Instance;


    private void Start()
    {
        Application.targetFrameRate = 144;
        LoadAudioSettings();
    }


    #region Load
    public void LoadAudioSettings()
    {
        // Safe load: if PlayerPrefs are corrupted or missing, fallback to defaults
        var (master, music, sfx, ui) = SafeLoad();

        ApplySlider(masterSlider, masterText, master);
        ApplySlider(musicSlider, musicText, music);
        ApplySlider(sfxSlider, sfxText, sfx);
        ApplySlider(uiSlider, uiText, ui);

        audioManager.SetMixerVolume(master, audioManager.masterParam);
        audioManager.SetMixerVolume(music, audioManager.musicParam);
        audioManager.SetMixerVolume(sfx, audioManager.sfxParam);
        audioManager.SetMixerVolume(ui, audioManager.uiParam);
    }

    private (int master, int music, int sfx, int ui) SafeLoad()
    {
        try
        {
            return audioManager.LoadVolumes();
        }
        catch
        {
            Debug.LogWarning("Audio settings corrupted. Resetting PlayerPrefs. [If you use PlayerPrefs for other data, consider implementing a more robust save system or namespacing your keys.]");
            PlayerPrefs.DeleteAll();
            return (100, 100, 100, 100);
        }
    }
    #endregion


    #region Save
    public void SaveAudioSettings()
    {
        int master = Mathf.RoundToInt(masterSlider.value);
        int music = Mathf.RoundToInt(musicSlider.value);
        int sfx = Mathf.RoundToInt(sfxSlider.value);
        int ui = Mathf.RoundToInt(uiSlider.value);

        audioManager.SaveVolumes(master, music, sfx, ui);
    }
    #endregion


    #region Slider Updates
    public void OnSliderValueChangedSetVolume(Slider slider)
    {
        int value = Mathf.RoundToInt(slider.value);

        if (slider == masterSlider)
        {
            masterText.text = value.ToString();
            audioManager.SetMixerVolume(value, audioManager.masterParam);
        }
        else if (slider == musicSlider)
        {
            musicText.text = value.ToString();
            audioManager.SetMixerVolume(value, audioManager.musicParam);
        }
        else if (slider == sfxSlider)
        {
            sfxText.text = value.ToString();
            audioManager.SetMixerVolume(value, audioManager.sfxParam);
        }
        else if (slider == uiSlider)
        {
            uiText.text = value.ToString();
            audioManager.SetMixerVolume(value, audioManager.uiParam);
        }
    }
    #endregion


    #region Helpers
    private void ApplySlider(Slider slider, TextMeshProUGUI label, int value)
    {
        slider.value = value;
        label.text = value.ToString();
    }
    #endregion
}
