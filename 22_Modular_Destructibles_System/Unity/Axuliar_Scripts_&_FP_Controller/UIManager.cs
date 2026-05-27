using System.Collections;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

/*
    Extension of the LVN Showcase UIManager (15_AudioManager).
    All original audio slider and example counter code is preserved unchanged.
    Added: destructible hover panel display — clearly marked region below.

    - Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
*/

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

    [Header("[Destructible] Hover")]
    [SerializeField] private GameObject destructibleHoverPanel; // Assign a UI panel with Text children for name and HP display.
    [SerializeField] private TextMeshProUGUI hoverNameText;
    [SerializeField] private TextMeshProUGUI hoverHpText;

    // Optional hit cooldown bar example to show how UIManager can handle other UI elements related to destructibles or player feedback.
    [Header("Hit Cooldown Bar")]
    [SerializeField] private RectTransform hitCooldownFillGraphic;
    private Coroutine hitCooldownRoutine;
    private float hitCooldownFullWidth;  // cached on first use so we never lose the reference width

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

        if (destructibleHoverPanel != null)
            destructibleHoverPanel.SetActive(false);

        if (hitCooldownFillGraphic != null)
            hitCooldownFillGraphic.gameObject.SetActive(false);
    }


    #region Load Audio Settings
    public void LoadAudioSettings()
    {
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


    #region Save Audio Settings
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


    #region Destructible Utility Methods

    /// <summary>
    /// Show the hover panel when the player first looks at a destructible.
    /// </summary>
    public void ShowDestructibleHover(string destructibleName, int currentHP, int maxHP)
    {
        if (destructibleHoverPanel == null) return;
        destructibleHoverPanel.SetActive(true);

        if (hoverNameText != null)
            hoverNameText.text = destructibleName;

        if (hoverHpText != null)
            hoverHpText.text = $"{currentHP} / {maxHP}";
    }

    /// <summary>
    /// Update only the HP line each frame while the player keeps looking at a destructible.
    /// </summary>
    public void UpdateDestructibleHover(int currentHP, int maxHP)
    {
        if (hoverHpText != null)
            hoverHpText.text = $"{currentHP} / {maxHP}";
    }

    /// <summary>
    /// Hide the hover panel when the player looks away or the object is destroyed.
    /// </summary>
    public void HideDestructibleHover()
    {
        if (destructibleHoverPanel != null)
            destructibleHoverPanel.SetActive(false);
    }

    public void ShowHitCooldown(float duration)
    {
        if (hitCooldownFillGraphic == null || duration <= 0f) return;

        if (hitCooldownRoutine != null) StopCoroutine(hitCooldownRoutine);
        hitCooldownRoutine = StartCoroutine(HitCooldownRoutine(duration));
    }

    private IEnumerator HitCooldownRoutine(float duration)
    {
        if (hitCooldownFullWidth <= 0f)
            hitCooldownFullWidth = hitCooldownFillGraphic.sizeDelta.x;

        hitCooldownFillGraphic.gameObject.SetActive(true);

        SetBarWidth(hitCooldownFullWidth);
        float elapsed = 0f;

        while (elapsed < duration)
        {
            elapsed += Time.deltaTime;
            SetBarWidth((1f - Mathf.Clamp01(elapsed / duration)) * hitCooldownFullWidth);  // drain to 0
            yield return null;
        }

        SetBarWidth(0f);

        yield return new WaitForSeconds(0.08f);

        hitCooldownFillGraphic.gameObject.SetActive(false);
        hitCooldownRoutine = null;
    }

    private void SetBarWidth(float width)
    {
        Vector2 size = hitCooldownFillGraphic.sizeDelta;
        size.x = width;
        hitCooldownFillGraphic.sizeDelta = size;
    }

    #endregion
}