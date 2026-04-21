using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class UIManager : MonoBehaviour
{
    public static UIManager Instance;

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

    [Header("Examples")]
    private int coinCount = 0;
    private int gemCount = 0;
    private int keyCount = 0;
    [SerializeField] private TextMeshProUGUI coinCountText;
    [SerializeField] private TextMeshProUGUI gemCountText;
    [SerializeField] private TextMeshProUGUI keyCountText;

    private void Awake()
    {
        if (Instance != null && Instance != this)
        {
            Destroy(gameObject);
            return;
        }
        Instance = this;
    }

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

    #region Examples
        public void AddCoin(int amount)
        {
            if (amount <= 0 || coinCountText == null)
            {
                Debug.LogWarning($"Invalid coin amount ({amount}) or missing UI reference. Amount must be positive and coinCountText must be assigned.");
                return;
            }
            coinCount += amount;
            coinCountText.text = $"x{coinCount}";
        }

        public void AddGem(int amount)
        {
            if (amount <= 0 || gemCountText == null)
            {
                Debug.LogWarning($"Invalid gem amount ({amount}) or missing UI reference. Amount must be positive and gemCountText must be assigned.");
                return;
            }
            gemCount += amount;
            gemCountText.text = $"x{gemCount}";
        }

        public void AddKey(int amount)
        {
            if (amount <= 0 || keyCountText == null)
            {
                Debug.LogWarning($"Invalid key amount ({amount}) or missing UI reference. Amount must be positive and keyCountText must be assigned.");
                return;
            }
            keyCount += amount;
            keyCountText.text = $"x{keyCount}";
        }
    #endregion
}
