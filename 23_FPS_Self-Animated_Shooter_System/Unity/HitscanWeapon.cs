using System;
using System.Collections;
using UnityEngine;
using UnityEngine.Events;

[DisallowMultipleComponent]
public class HitscanWeapon : MonoBehaviour, IWeapon
{
    private enum WeaponState { Idle, Firing, Bursting, Reloading }

    private static readonly FireMode[] CycleOrder = { FireMode.Semi, FireMode.Burst, FireMode.Auto };

    [Header("Feedback Hooks")]
    [SerializeField] private UnityEvent onFiredEvent;
    [SerializeField] private HitPointEvent onImpactEvent;

    [Header("Debug")]
    [SerializeField] private bool drawDebugRays = false;
    [SerializeField] private float debugRayDuration = 1.2f;
    [SerializeField] private float debugRayWidth = 0.005f;
    [Range(0f, 1f)][SerializeField] private float debugRayOpacity = 0.5f;
    [SerializeField] private float debugRayStartOffset = 1.2f;
    private static Material debugMaterial;

    private WeaponDataSO data;
    private Camera fireCamera;
    private LayerMask hitMask;

    [Serializable] public class HitPointEvent : UnityEvent<Vector3, Vector3> { }

    private WeaponState state = WeaponState.Idle;
    private int magAmmo;
    private int reserveAmmo;
    private FireMode currentFireMode = FireMode.Semi;
    private bool isAiming;

    private bool triggerHeld;
    private bool triggerHeldLastFrame;
    private float nextFireTime;
    private Coroutine activeRoutine;

    public WeaponDataSO Data => data;
    public int CurrentMagAmmo => magAmmo;
    public int CurrentReserveAmmo => reserveAmmo;
    public FireMode CurrentFireMode => currentFireMode;
    public bool IsReloading => state == WeaponState.Reloading;

    public event Action OnAmmoChanged;
    public event Action<WeaponFireInfo> OnFired;
    public event Action OnFireModeChanged;

    public void Initialize(WeaponDataSO weaponData, Camera camera, LayerMask mask, int startMagAmmo, int startReserveAmmo)
    {
        data = weaponData;
        fireCamera = camera;
        hitMask = mask;

        if (data == null)
        {
            Debug.LogError($"[HitscanWeapon] {name} initialized without WeaponDataSO. Disabling.");
            enabled = false;
            return;
        }

        magAmmo = startMagAmmo < 0 ? data.magSize : Mathf.Clamp(startMagAmmo, 0, data.magSize);
        reserveAmmo = startReserveAmmo < 0 ? data.startingReserve : Mathf.Clamp(startReserveAmmo, 0, data.reserveCap);

        currentFireMode = data.Supports(data.defaultFireMode) ? data.defaultFireMode : NextSupportedMode(FireMode.Semi);
        state = WeaponState.Idle;
        OnAmmoChanged?.Invoke();
    }

    private void OnDisable() => CancelActions();

    public void CancelActions()
    {
        if (activeRoutine != null) StopCoroutine(activeRoutine);
        activeRoutine = null;

        StopAllCoroutines();

        state = WeaponState.Idle;
        triggerHeld = false;
        triggerHeldLastFrame = false;
        isAiming = false;
    }


    private void LateUpdate()
    {
        if (data == null) return;

        if (state == WeaponState.Firing && Time.time >= nextFireTime)
            state = WeaponState.Idle;

        bool pressedThisFrame = triggerHeld && !triggerHeldLastFrame;

        if (state == WeaponState.Idle)
            HandleTrigger(pressedThisFrame);

        triggerHeldLastFrame = triggerHeld;
        triggerHeld = false;
    }

    private void HandleTrigger(bool pressedThisFrame)
    {
        switch (currentFireMode)
        {
            case FireMode.Semi:
                if (pressedThisFrame) TryFire();
                break;

            case FireMode.Auto:
                if (triggerHeld) TryFire();
                break;

            case FireMode.Burst:
                if (pressedThisFrame) StartBurst();
                break;
        }
    }

    private void TryFire()
    {
        if (magAmmo < data.ammoPerShot)
        {
            if (data.autoReloadOnEmpty) Reload();
            return;
        }

        FireShot();
        BeginFireCooldown();
    }

    private void StartBurst()
    {
        if (magAmmo < data.ammoPerShot)
        {
            if (data.autoReloadOnEmpty) Reload();
            return;
        }

        activeRoutine = StartCoroutine(BurstRoutine());
    }

    private IEnumerator BurstRoutine()
    {
        state = WeaponState.Bursting;

        for (int i = 0; i < data.burstCount; i++)
        {
            if (magAmmo < data.ammoPerShot) break;

            FireShot();
            if (i < data.burstCount - 1)
                yield return new WaitForSeconds(data.burstDelay);
        }

        activeRoutine = null;
        BeginFireCooldown();
    }

    private void BeginFireCooldown()
    {
        nextFireTime = Time.time + data.TimeBetweenShots;
        state = WeaponState.Firing;
    }

    // Firing

    private void FireShot()
    {
        magAmmo = Mathf.Max(0, magAmmo - data.ammoPerShot);
        OnAmmoChanged?.Invoke();

        WeaponFireInfo primary = default;
        float spread = isAiming ? data.aimSpread : data.hipSpread;

        for (int i = 0; i < data.pelletsPerShot; i++)
        {
            Vector3 origin = fireCamera.transform.position;
            Vector3 direction = ApplySpread(fireCamera.transform.forward, spread);

            WeaponFireInfo info = new WeaponFireInfo { origin = origin, direction = direction };

            if (Physics.Raycast(origin, direction, out RaycastHit hit, data.range, hitMask, QueryTriggerInteraction.Ignore))
            {
                info.didHit = true;
                info.hit = hit;

                if (hit.collider.TryGetComponent(out IHittable hittable))
                    hittable.TakeHit(data.damage, hit.point, direction);

                DrawDebugTrace(origin, hit.point, true);
                onImpactEvent?.Invoke(hit.point, hit.normal);
            }
            else
            {
                DrawDebugTrace(origin, origin + direction * data.range, false);
            }

            if (i == 0) primary = info;
        }

        onFiredEvent?.Invoke();
        OnFired?.Invoke(primary);
    }

    // Debug that plays in both Editor and Game view.
    private void DrawDebugTrace(Vector3 from, Vector3 to, bool didHit)
    {
        if (!drawDebugRays) return;

        Vector3 start = Vector3.MoveTowards(from, to, debugRayStartOffset);
        if ((to - start).sqrMagnitude < 0.0001f) return;

        if (debugMaterial == null)
            debugMaterial = new Material(Shader.Find("Sprites/Default")) { name = "DebugTracer" };

        GameObject tracer = new GameObject("DebugTracer");
        LineRenderer line = tracer.AddComponent<LineRenderer>();
        line.material = debugMaterial;
        line.useWorldSpace = true;
        line.widthMultiplier = debugRayWidth;
        line.positionCount = 2;
        line.SetPosition(0, start);
        line.SetPosition(1, to);
        line.shadowCastingMode = UnityEngine.Rendering.ShadowCastingMode.Off;
        line.receiveShadows = false;

        Color color = didHit ? Color.green : Color.red;
        color.a = debugRayOpacity;
        line.startColor = color;
        line.endColor = color;

        tracer.AddComponent<DebugTracerFade>().Begin(line, color, debugRayDuration);
    }

    private Vector3 ApplySpread(Vector3 forward, float spreadAngle)
    {
        if (spreadAngle <= 0f) return forward;

        Vector2 offset = UnityEngine.Random.insideUnitCircle * spreadAngle;
        Transform cam = fireCamera.transform;

        forward = Quaternion.AngleAxis(offset.x, cam.up) * forward;
        forward = Quaternion.AngleAxis(offset.y, cam.right) * forward;
        return forward;
    }

    // IWeapon implementation. The holder calls these, never the player directly.

    public void Fire() => triggerHeld = true;

    public void Reload()
    {
        if (state == WeaponState.Reloading || state == WeaponState.Bursting) return;
        if (magAmmo >= data.magSize || reserveAmmo <= 0) return;

        activeRoutine = StartCoroutine(ReloadRoutine());
    }

    private IEnumerator ReloadRoutine()
    {
        state = WeaponState.Reloading;
        yield return new WaitForSeconds(data.reloadDuration);

        int needed = data.magSize - magAmmo;
        int taken = Mathf.Min(needed, reserveAmmo);
        magAmmo += taken;
        reserveAmmo -= taken;

        activeRoutine = null;
        state = WeaponState.Idle;
        OnAmmoChanged?.Invoke();
    }

    public void AimIn() => isAiming = true;

    public void AimOut() => isAiming = false;

    public void CycleFireMode()
    {
        if (state != WeaponState.Idle) return;

        FireMode previous = currentFireMode;
        currentFireMode = NextSupportedMode(currentFireMode);

        if (currentFireMode != previous)
            OnFireModeChanged?.Invoke();
    }

    private FireMode NextSupportedMode(FireMode from)
    {
        int start = Array.IndexOf(CycleOrder, from);
        if (start < 0) start = 0;

        for (int step = 1; step <= CycleOrder.Length; step++)
        {
            FireMode candidate = CycleOrder[(start + step) % CycleOrder.Length];
            if (data.Supports(candidate)) return candidate;
        }

        return from;
    }

    public int AddReserveAmmo(int amount)
    {
        if (amount <= 0) return 0;

        int space = data.reserveCap - reserveAmmo;
        int added = Mathf.Min(space, amount);
        if (added <= 0) return 0;

        reserveAmmo += added;
        OnAmmoChanged?.Invoke();
        return added;
    }
}
