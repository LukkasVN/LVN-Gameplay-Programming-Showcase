using System;
using UnityEngine;

[DisallowMultipleComponent]
public class WeaponHolder : MonoBehaviour
{
    // Single player convenience lookup. World pickups use it because IFP_Interactable.OnInteract
    // carries no reference to the interactor.
    public static WeaponHolder Local { get; private set; }

    [Header("References")]
    [SerializeField] private FPS_Controller controller;
    [SerializeField] private Camera fireCamera;
    [SerializeField] private Transform viewmodelSocket;
    [SerializeField] private Transform tossOrigin;
    [SerializeField] private LayerMask hitMask = ~0;

    [Header("Loadout")]
    [SerializeField] private WeaponDataSO startingWeapon;

    [Header("Aim Timing")]
    [SerializeField] private float aimDuration = 0.15f;
    [SerializeField] private AnimationCurve aimCurve = AnimationCurve.EaseInOut(0f, 0f, 1f, 1f);
    [SerializeField] private bool blockAimWhileDashing = true;

    [Header("Toss")]
    [SerializeField] private float tossForce = 5f;
    [SerializeField] private float tossUpwardBias = 2.2f;
    [SerializeField] private float tossTorque = 9f;
    [Range(0f, 1f)][SerializeField] private float tossInheritPlayerVelocity = 0.8f;

    private WeaponDataSO currentData;
    private IWeapon currentWeapon;
    private GameObject currentViewmodel;

    private float aimProgress;
    private bool wasAiming;
    private Vector3 lastPlayerPosition;
    private Vector3 playerVelocity;

    public IWeapon ActiveWeapon => currentWeapon;
    public WeaponDataSO ActiveWeaponData => currentData;
    public bool HasWeapon => currentWeapon != null;

    // Read by WeaponViewmodelFeedback to damp its juice while aiming.
    public float AimProgress => aimProgress;

    public event Action<IWeapon> OnAmmoChanged;
    public event Action<IWeapon> OnActiveWeaponChanged;
    public event Action<IWeapon, WeaponFireInfo> OnWeaponFired;
    public event Action<IWeapon> OnFireModeChanged;

    // Raised when an ammo pickup could not be used, wrong caliber or already capped.
    public event Action OnAmmoRejected;

    private void Awake()
    {
        Local = this;

        if (controller == null) controller = GetComponent<FPS_Controller>();
        if (fireCamera == null && controller != null) fireCamera = controller.GetPlayerCamera();
        if (tossOrigin == null && fireCamera != null) tossOrigin = fireCamera.transform;

        lastPlayerPosition = transform.position;
    }

    private void OnDestroy()
    {
        if (Local == this) Local = null;
    }

    private void Start()
    {
        if (viewmodelSocket == null || fireCamera == null)
        {
            Debug.LogError("[WeaponHolder] Viewmodel socket or camera reference is missing.");
            enabled = false;
            return;
        }

        if (startingWeapon != null)
            Equip(startingWeapon, -1, -1);

        OnActiveWeaponChanged?.Invoke(currentWeapon);
        OnAmmoChanged?.Invoke(currentWeapon);
    }

    private void Update()
    {
        TrackPlayerVelocity();
        HandleWeaponInput();

        if (InputManager.Instance.IsTossingWeapon)
            TossWeapon();
    }

    private void LateUpdate()
    {
        ApplyAim();
    }

    private void TrackPlayerVelocity()
    {
        if (Time.deltaTime <= 0f) return;
        playerVelocity = (transform.position - lastPlayerPosition) / Time.deltaTime;
        lastPlayerPosition = transform.position;
    }

    // Input

    private void HandleWeaponInput()
    {
        if (currentWeapon == null) return;

        if (InputManager.Instance.IsFiring) currentWeapon.Fire();
        if (InputManager.Instance.IsReloading) currentWeapon.Reload();
        if (InputManager.Instance.IsSwitchingFireMode) currentWeapon.CycleFireMode();

        bool wantsAim = InputManager.Instance.IsAiming && !IsAimBlocked();
        if (wantsAim != wasAiming)
        {
            if (wantsAim) currentWeapon.AimIn();
            else currentWeapon.AimOut();
            wasAiming = wantsAim;
        }
    }

    private bool IsAimBlocked()
    {
        if (currentWeapon != null && currentWeapon.IsReloading) return true;

        return blockAimWhileDashing
            && controller != null
            && controller.CurrentState == FPS_Controller.MovementState.Dashing;
    }

    // Aim

    private void ApplyAim()
    {
        float target = wasAiming && currentWeapon != null ? 1f : 0f;
        float step = aimDuration <= 0f ? 1f : Time.deltaTime / aimDuration;
        aimProgress = Mathf.MoveTowards(aimProgress, target, step);

        if (currentData == null)
        {
            if (controller != null) controller.CameraZoomOverride = null;
            return;
        }

        float t = aimCurve.Evaluate(aimProgress);

        viewmodelSocket.localPosition = Vector3.Lerp(currentData.hipPosition, currentData.aimPosition, t);
        viewmodelSocket.localRotation = Quaternion.Slerp(
            Quaternion.Euler(currentData.hipRotation), Quaternion.Euler(currentData.aimRotation), t);

        if (controller == null) return;

        if (t <= 0.001f)
        {
            controller.CameraZoomOverride = null;
            return;
        }

        controller.CameraZoomOverride = Mathf.Lerp(controller.BaseFov, currentData.aimFov, t);
    }

    // Ammo

    public bool AddAmmo(AmmoType type, int amount)
    {
        if (amount <= 0 || currentWeapon == null || currentData == null || currentData.ammoType != type)
        {
            OnAmmoRejected?.Invoke();
            return false;
        }

        if (currentWeapon.AddReserveAmmo(amount) <= 0)
        {
            OnAmmoRejected?.Invoke();
            return false;
        }

        if (currentWeapon.CurrentMagAmmo <= 0)
            currentWeapon.Reload();

        return true;
    }

    // Pickup and Toss

    public bool TryPickUpWeapon(WeaponDataSO data, int magAmmo, int reserveAmmo)
    {
        if (data == null) return false;

        if (currentWeapon != null)
            TossWeapon();

        Equip(data, magAmmo, reserveAmmo);

        OnActiveWeaponChanged?.Invoke(currentWeapon);
        OnAmmoChanged?.Invoke(currentWeapon);
        return true;
    }

    public void TossWeapon()
    {
        if (currentWeapon == null || currentData == null) return;

        if (currentData.pickupPrefab == null)
        {
            Debug.LogWarning($"[WeaponHolder] {currentData.weaponName} has no pickupPrefab, toss cancelled.");
            return;
        }

        currentWeapon.CancelActions();

        Transform source = currentViewmodel != null ? currentViewmodel.transform : tossOrigin;
        GameObject dropped = Instantiate(currentData.pickupPrefab, source.position, source.rotation);

        if (dropped.TryGetComponent(out WeaponPickup pickup))
            pickup.InitializeFromToss(currentData, currentWeapon.CurrentMagAmmo, currentWeapon.CurrentReserveAmmo);

        if (dropped.TryGetComponent(out Rigidbody body))
        {
            Vector3 throwVelocity = tossOrigin.forward * tossForce
                                  + Vector3.up * tossUpwardBias
                                  + playerVelocity * tossInheritPlayerVelocity;

            body.AddForce(throwVelocity, ForceMode.VelocityChange);
            body.AddTorque(UnityEngine.Random.onUnitSphere * tossTorque, ForceMode.VelocityChange);
        }

        Unequip();

        OnActiveWeaponChanged?.Invoke(null);
        OnAmmoChanged?.Invoke(null);
    }


    private void Equip(WeaponDataSO data, int magAmmo, int reserveAmmo)
    {
        if (data.viewmodelPrefab == null)
        {
            Debug.LogError($"[WeaponHolder] {data.weaponName} has no viewmodelPrefab assigned.");
            return;
        }

        Unequip();

        GameObject viewmodel = Instantiate(data.viewmodelPrefab, viewmodelSocket, false);
        viewmodel.transform.localPosition = Vector3.zero;
        viewmodel.transform.localRotation = Quaternion.identity;

        if (viewmodel.TryGetComponent(out WeaponViewmodelFeedback feedback))
            feedback.CaptureBasePose();

        if (!viewmodel.TryGetComponent(out IWeapon weapon))
        {
            Debug.LogError($"[WeaponHolder] {data.weaponName} viewmodel prefab has no IWeapon on its root.");
            Destroy(viewmodel);
            return;
        }

        weapon.Initialize(data, fireCamera, hitMask, magAmmo, reserveAmmo);
        weapon.OnAmmoChanged += HandleWeaponAmmoChanged;
        weapon.OnFired += HandleWeaponFired;
        weapon.OnFireModeChanged += HandleFireModeChanged;

        currentData = data;
        currentWeapon = weapon;
        currentViewmodel = viewmodel;

        wasAiming = false;
        aimProgress = 0f;
    }

    private void Unequip()
    {
        if (currentWeapon != null)
        {
            currentWeapon.OnAmmoChanged -= HandleWeaponAmmoChanged;
            currentWeapon.OnFired -= HandleWeaponFired;
            currentWeapon.OnFireModeChanged -= HandleFireModeChanged;
        }

        if (currentViewmodel != null)
            Destroy(currentViewmodel);

        currentData = null;
        currentWeapon = null;
        currentViewmodel = null;

        wasAiming = false;
        aimProgress = 0f;

        if (controller != null)
            controller.CameraZoomOverride = null;
    }

    private void HandleWeaponAmmoChanged() => OnAmmoChanged?.Invoke(currentWeapon);

    private void HandleWeaponFired(WeaponFireInfo info) => OnWeaponFired?.Invoke(currentWeapon, info);

    private void HandleFireModeChanged() => OnFireModeChanged?.Invoke(currentWeapon);
}
