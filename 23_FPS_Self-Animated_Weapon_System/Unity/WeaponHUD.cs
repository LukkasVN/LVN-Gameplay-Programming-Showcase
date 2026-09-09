using TMPro;
using UnityEngine;
using UnityEngine.UI;

[DisallowMultipleComponent]
public class WeaponHUD : MonoBehaviour
{
    [Header("References")]
    [SerializeField] private WeaponHolder holder;
    [SerializeField] private PlayerInteractor interactor;
    [SerializeField] private FPS_Controller controller;

    [Header("Widgets")]
    [SerializeField] private GameObject root;
    [SerializeField] private TextMeshProUGUI ammoText;
    [SerializeField] private TextMeshProUGUI weaponNameText;
    [SerializeField] private TextMeshProUGUI fireModeText;
    [SerializeField] private Image weaponIcon;

    [Header("Interact Prompt")]
    [SerializeField] private GameObject promptRoot;
    [SerializeField] private TextMeshProUGUI promptText;

    [Header("Ammo Rejection Flash")]
    [SerializeField] private Color rejectFlashColor = new Color(1f, 0.25f, 0.25f);
    [SerializeField] private float rejectFlashDuration = 0.45f;
    [SerializeField] private float rejectShakeAmount = 6f;

    [Header("Dash Cooldown")]
    [SerializeField] private TextMeshProUGUI dashText;
    [SerializeField] private string dashReadyLabel = "Dash Ready";
    [SerializeField] private string dashCooldownFormat = "{0:0.00}s";
    [SerializeField] private Color dashChargingColor = new Color(1f, 1f, 1f, 0.35f);
    [SerializeField] private Color dashReadyColor = Color.white;

    [Header("Format")]
    [SerializeField] private string ammoFormat = "{0} / {1}";

    private bool wasDashReady = true;
    private string currentPrompt = string.Empty;
    private Coroutine rejectRoutine;
    private Color ammoBaseColor = Color.white;
    private Vector3 ammoBasePosition;

    private void Start()
    {
        if (holder == null) holder = WeaponHolder.Local;

        if (holder == null)
        {
            Debug.LogError("[WeaponHUD] No WeaponHolder found. Disabling.");
            enabled = false;
            return;
        }

        if (interactor == null) interactor = holder.GetComponent<PlayerInteractor>();
        if (controller == null) controller = holder.GetComponent<FPS_Controller>();

        if (root != null && (root == gameObject || transform.IsChildOf(root.transform)))
        {
            Debug.LogError("[WeaponHUD] This component sits under the widget root it deactivates. " +
                           "Unequipping would disable the HUD permanently, since nothing would be " +
                           "left running to re-enable it. Move the root to a sibling object.");
        }

        if (ammoText != null)
        {
            ammoBaseColor = ammoText.color;
            ammoBasePosition = ammoText.rectTransform.anchoredPosition;
        }

        Subscribe();
        Refresh(holder.ActiveWeapon);
        RefreshPrompt(interactor != null ? interactor.CurrentPrompt : string.Empty);
    }

    private void OnEnable()
    {
        if (holder != null) Subscribe();
    }

    private void OnDisable()
    {
        if (holder != null)
        {
            holder.OnAmmoChanged -= Refresh;
            holder.OnActiveWeaponChanged -= Refresh;
            holder.OnFireModeChanged -= Refresh;
            holder.OnAmmoRejected -= HandleAmmoRejected;
        }

        if (interactor != null)
            interactor.OnPromptChanged -= RefreshPrompt;
    }

    private void Subscribe()
    {
        holder.OnAmmoChanged -= Refresh;
        holder.OnActiveWeaponChanged -= Refresh;
        holder.OnFireModeChanged -= Refresh;

        holder.OnAmmoChanged += Refresh;
        holder.OnActiveWeaponChanged += Refresh;
        holder.OnFireModeChanged += Refresh;

        holder.OnAmmoRejected -= HandleAmmoRejected;
        holder.OnAmmoRejected += HandleAmmoRejected;

        if (interactor == null) return;
        interactor.OnPromptChanged -= RefreshPrompt;
        interactor.OnPromptChanged += RefreshPrompt;
    }

    private void Update()
    {
        UpdateDashCooldown();
    }


    private void Refresh(IWeapon weapon)
    {
        bool hasWeapon = weapon != null && weapon.Data != null;

        if (root != null)
            root.SetActive(hasWeapon);

        if (!hasWeapon) return;

        if (ammoText != null)
            ammoText.text = string.Format(ammoFormat, weapon.CurrentMagAmmo, weapon.CurrentReserveAmmo);

        if (weaponNameText != null)
            weaponNameText.text = weapon.Data.weaponName;

        if (fireModeText != null)
            fireModeText.text = "Mode: " + weapon.CurrentFireMode.ToString().ToUpperInvariant();

        if (weaponIcon != null)
        {
            weaponIcon.sprite = weapon.Data.weaponIcon;
            weaponIcon.enabled = weapon.Data.weaponIcon != null;
        }
    }


    private void RefreshPrompt(string prompt)
    {
        currentPrompt = prompt;

        if (promptText != null && !string.IsNullOrEmpty(prompt))
            promptText.text = prompt;

        if (promptRoot != null)
            promptRoot.SetActive(!string.IsNullOrEmpty(currentPrompt));
    }


    private void HandleAmmoRejected()
    {
        if (ammoText == null) return;

        if (rejectRoutine != null) StopCoroutine(rejectRoutine);
        rejectRoutine = StartCoroutine(RejectFlashRoutine());
    }

    private System.Collections.IEnumerator RejectFlashRoutine()
    {
        float elapsed = 0f;

        while (elapsed < rejectFlashDuration)
        {
            elapsed += Time.deltaTime;
            float t = elapsed / rejectFlashDuration;

            ammoText.color = Color.Lerp(rejectFlashColor, ammoBaseColor, t * t);

            float decay = 1f - t;
            float offset = Mathf.Sin(elapsed * 45f) * rejectShakeAmount * decay * decay;
            ammoText.rectTransform.anchoredPosition = ammoBasePosition + Vector3.right * offset;

            yield return null;
        }

        ammoText.color = ammoBaseColor;
        ammoText.rectTransform.anchoredPosition = ammoBasePosition;
        rejectRoutine = null;
    }


    private void UpdateDashCooldown()
    {
        if (controller == null || dashText == null) return;

        float remaining = controller.DashCooldownRemaining;
        bool isReady = remaining <= 0f;

        if (isReady && wasDashReady) return;

        dashText.text = isReady ? dashReadyLabel : string.Format(dashCooldownFormat, remaining);
        dashText.color = isReady ? dashReadyColor : dashChargingColor;

        wasDashReady = isReady;
    }
}
