using UnityEngine;

// Universal viewmodel juice. Belongs on the WEAPON PREFAB ROOT and nowhere else: on the socket it
// overwrites WeaponHolder's aim blend and every pose value on the data asset stops working.
//
// Does not bob: the camera already owns headbob, and a second synced bob reads as doubled motion.

[DisallowMultipleComponent]
public class WeaponViewmodelFeedback : MonoBehaviour
{
    [Header("Idle Drift")]
    [SerializeField] private bool hasIdleDrift = true;
    [SerializeField] private float driftFrequency = 1.1f;
    [SerializeField] private Vector2 driftAmplitude = new Vector2(0.006f, 0.004f);
    [SerializeField] private float driftRollAngle = 0.8f;

    [Header("Sway Tilt")]
    [SerializeField] private float tiltRollAngle = 5f;
    [SerializeField] private float tiltPitchAngle = 2.5f;
    [SerializeField] private float tiltLerpSpeed = 8f;

    [Header("Fire Recoil")]
    [SerializeField] private float recoilKickImpulse = 2.2f;
    [SerializeField] private float recoilPitchImpulse = 180f;
    [SerializeField] private float recoilRollImpulse = 70f;
    [SerializeField] private Vector3 recoilStretchImpulse = new Vector3(-2.5f, -2.5f, 6f);
    [SerializeField] private float recoilStiffness = 170f;
    [SerializeField] private float recoilDamping = 11f;

    [Header("Reload Pose")]
    [SerializeField] private float reloadTiltAngle = 40f;
    [SerializeField] private float reloadRollAngle = 12f;
    [SerializeField] private Vector3 reloadOffset = new Vector3(0.02f, -0.09f, -0.05f);
    [SerializeField] private Vector3 reloadSquash = new Vector3(0.18f, -0.22f, 0.1f);
    [SerializeField] private Vector3 reloadPunchImpulse = new Vector3(-2f, 3.5f, -2.5f);
    [SerializeField] private float reloadEnterSpeed = 11f;
    [SerializeField] private float reloadExitSpeed = 7f;

    [Header("Fire Mode Switch")]
    [SerializeField] private float switchRollImpulse = 130f;
    [SerializeField] private float switchPitchImpulse = 45f;
    [SerializeField] private Vector3 switchSquashImpulse = new Vector3(2.5f, -3f, 1.5f);
    [SerializeField] private float switchKickImpulse = 0.8f;

    [Header("Landing Impact")]
    [SerializeField] private float landingDipImpulse = 0.7f;
    [SerializeField] private Vector3 landingSquashImpulse = new Vector3(1f, -1.4f, 0.35f);
    [SerializeField] private float landingPitchImpulse = -35f;

    [Header("Aim Damping")]
    [Range(0f, 1f)][SerializeField] private float aimFeedbackScale = 0.15f;

    [Header("Scale")]
    [SerializeField] private float squashIntensity = 1f;
    [SerializeField] private float minScaleFactor = 0.25f;

    private FPS_Controller controller;
    private WeaponHolder holder;
    private IWeapon weapon;

    private Vector3 basePosition;
    private Quaternion baseRotation;
    private Vector3 baseScale;
    private bool hasBasePose;

    private Vector3 impulsePosition, impulsePositionVel;
    private Vector3 impulseScale, impulseScaleVel;
    private float impulsePitch, impulsePitchVel;
    private float impulseRoll, impulseRollVel;

    private float swayRoll, swayPitch;
    private float reloadBlend;
    private float driftTimer;

    private bool wasReloading;
    private int recoilRollSign = 1;
    private FPS_Controller.MovementState lastState;

    private void Awake()
    {
        controller = GetComponentInParent<FPS_Controller>();
        holder = GetComponentInParent<WeaponHolder>();

        weapon = GetComponent<IWeapon>();
    }

    private void Start()
    {
        if (!hasBasePose) CaptureBasePose();
    }

    // Called by WeaponHolder once the spawned transform is final. Explicit rather than relying on
    // Awake, which runs during Instantiate while the transform is still whatever the reparent produced.
    public void CaptureBasePose()
    {
        basePosition = transform.localPosition;
        baseRotation = transform.localRotation;
        baseScale = transform.localScale;
        hasBasePose = true;
    }

    private void OnEnable()
    {
        if (holder != null)
        {
            holder.OnWeaponFired += HandleWeaponFired;
            holder.OnFireModeChanged += HandleFireModeChanged;
        }
        if (controller != null) lastState = controller.CurrentState;
        ResetLayers();
    }

    private void OnDisable()
    {
        if (holder != null)
        {
            holder.OnWeaponFired -= HandleWeaponFired;
            holder.OnFireModeChanged -= HandleFireModeChanged;
        }
        ResetLayers();
    }

    private void LateUpdate()
    {
        if (!hasBasePose) return;

        float damp = holder != null ? Mathf.Lerp(1f, aimFeedbackScale, holder.AimProgress) : 1f;
        float dt = Mathf.Min(Time.deltaTime, 0.033f);

        TrackLanding();
        TrackReload();
        UpdateSway(damp);
        UpdateDrift();
        TickSprings(dt);
        Compose(damp);
    }

    private void HandleWeaponFired(IWeapon firedWeapon, WeaponFireInfo info)
    {
        impulsePositionVel += Vector3.back * recoilKickImpulse;
        impulsePitchVel += recoilPitchImpulse;
        impulseRollVel += recoilRollImpulse * recoilRollSign;
        impulseScaleVel += recoilStretchImpulse;

        recoilRollSign = -recoilRollSign;
    }

    private void HandleFireModeChanged(IWeapon changedWeapon)
    {
        impulseRollVel += switchRollImpulse;
        impulsePitchVel += switchPitchImpulse;
        impulseScaleVel += switchSquashImpulse;
        impulsePositionVel += Vector3.back * switchKickImpulse;
    }

    private void TrackLanding()
    {
        if (controller == null) return;

        FPS_Controller.MovementState state = controller.CurrentState;

        if (lastState == FPS_Controller.MovementState.Airborne && state == FPS_Controller.MovementState.Grounded)
        {
            impulsePositionVel += Vector3.down * landingDipImpulse;
            impulseScaleVel += landingSquashImpulse;
            impulsePitchVel += landingPitchImpulse;
        }

        lastState = state;
    }

    private void TrackReload()
    {
        bool reloading = weapon != null && weapon.IsReloading;

        if (reloading != wasReloading)
        {
            impulseScaleVel += reloadPunchImpulse * (reloading ? 1f : -1f);
            impulsePitchVel += reloading ? recoilPitchImpulse * 0.35f : -recoilPitchImpulse * 0.25f;
            wasReloading = reloading;
        }

        float target = reloading ? 1f : 0f;
        float speed = reloading ? reloadEnterSpeed : reloadExitSpeed;
        reloadBlend = Mathf.Lerp(reloadBlend, target, Time.deltaTime * speed);
    }


    private void UpdateSway(float damp)
    {
        Vector2 move = InputManager.Instance.MoveInput;

        swayRoll = Mathf.LerpAngle(swayRoll, -move.x * tiltRollAngle * damp, Time.deltaTime * tiltLerpSpeed);
        swayPitch = Mathf.LerpAngle(swayPitch, -move.y * tiltPitchAngle * damp, Time.deltaTime * tiltLerpSpeed);
    }

    private void UpdateDrift()
    {
        if (!hasIdleDrift) return;
        driftTimer += Time.deltaTime * driftFrequency;
    }

    private void TickSprings(float dt)
    {
        Spring(ref impulsePosition, ref impulsePositionVel, recoilStiffness, recoilDamping, dt);
        Spring(ref impulseScale, ref impulseScaleVel, recoilStiffness, recoilDamping, dt);
        Spring(ref impulsePitch, ref impulsePitchVel, recoilStiffness, recoilDamping, dt);
        Spring(ref impulseRoll, ref impulseRollVel, recoilStiffness, recoilDamping, dt);
    }

    private static void Spring(ref float value, ref float velocity, float stiffness, float damping, float dt)
    {
        velocity += -value * stiffness * dt;
        velocity -= velocity * damping * dt;
        value += velocity * dt;
    }

    private static void Spring(ref Vector3 value, ref Vector3 velocity, float stiffness, float damping, float dt)
    {
        Spring(ref value.x, ref velocity.x, stiffness, damping, dt);
        Spring(ref value.y, ref velocity.y, stiffness, damping, dt);
        Spring(ref value.z, ref velocity.z, stiffness, damping, dt);
    }

    private void Compose(float damp)
    {
        Vector3 drift = Vector3.zero;
        float driftRoll = 0f;

        if (hasIdleDrift)
        {
            drift = new Vector3(
                Mathf.Sin(driftTimer) * driftAmplitude.x,
                Mathf.Sin(driftTimer * 1.7f) * driftAmplitude.y,
                0f);
            driftRoll = Mathf.Sin(driftTimer * 0.8f) * driftRollAngle;
        }

        Vector3 reloadPos = reloadOffset * reloadBlend;
        Vector3 reloadScale = reloadSquash * reloadBlend;
        float reloadPitch = reloadTiltAngle * reloadBlend;
        float reloadRoll = reloadRollAngle * reloadBlend;

        Vector3 position = basePosition
                         + reloadPos
                         + (drift + impulsePosition) * damp;

        float pitch = reloadPitch + (swayPitch + impulsePitch) * damp;
        float roll = reloadRoll + (swayRoll + impulseRoll + driftRoll) * damp;

        // Proportional to base scale, so the same values read identically on a pistol and a rifle.
        Vector3 deform = (reloadScale + impulseScale * damp) * squashIntensity;
        Vector3 scale = baseScale + Vector3.Scale(baseScale, deform);
        scale.x = Mathf.Max(scale.x, baseScale.x * minScaleFactor);
        scale.y = Mathf.Max(scale.y, baseScale.y * minScaleFactor);
        scale.z = Mathf.Max(scale.z, baseScale.z * minScaleFactor);

        transform.localPosition = position;
        transform.localRotation = baseRotation * Quaternion.Euler(pitch, 0f, roll);
        transform.localScale = scale;
    }

    private void ResetLayers()
    {
        impulsePosition = impulsePositionVel = Vector3.zero;
        impulseScale = impulseScaleVel = Vector3.zero;
        impulsePitch = impulsePitchVel = 0f;
        impulseRoll = impulseRollVel = 0f;
        swayRoll = swayPitch = 0f;
        reloadBlend = 0f;
        driftTimer = 0f;
        wasReloading = false;

        if (!hasBasePose) return;

        transform.localPosition = basePosition;
        transform.localRotation = baseRotation;
        transform.localScale = baseScale;
    }
}
