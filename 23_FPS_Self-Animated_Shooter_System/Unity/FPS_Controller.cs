using System.Collections;
using UnityEngine;

/*
    LVN Showcase FPS_Controller (FPS Weapon System entry).
    Standalone sibling to FP_Controller from previous entries, not a rewrite of it. Keeps only the
    core movement plumbing that a first person controller genuinely needs
    (move, look, jump, ground check). All additions below
    (tilt, landing dip, dash, headbob, landing FOV punch) are new logic
    built specifically for this entry.

    Camera feel runs on impulse springs rather than animation curves. A curve that
    starts at full value snaps in one frame and reads as a stutter, and it also has to
    fight whatever smoothing the layer underneath is already doing. A spring eases in
    from zero on its own and settles with a small overshoot, which is the part that
    actually sells weight.

    Weapon system integration point: CurrentState and CameraZoomOverride are exposed
    so an external system can react to movement state and take control of FOV
    without this script knowing anything about weapons exist.

    - Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
*/

[RequireComponent(typeof(CharacterController))]
public class FPS_Controller : MonoBehaviour
{
    [Header("References")]
    [SerializeField] private Camera playerCamera;
    private CharacterController cc;

    [Header("Movement")]
    [SerializeField] private float walkSpeed = 4.5f;
    [SerializeField] private float runSpeed = 7f;
    [SerializeField] private float jumpForce = 8.5f;
    [Range(0f, 1f)][SerializeField] private float coyoteTime = 0.12f;
    private bool wasRunningOnGround;
    private float coyoteTimer = 0f;
    private bool isRunningState = false;
    private Vector3 velocity;

    [Header("[Movement] Gravity")]
    [SerializeField] private float gravity = -38f;
    [SerializeField] private float fallGravityMultiplier = 1.9f;
    [SerializeField] private float maxFallSpeed = 45f;
    [SerializeField] private float groundStickForce = 4f;

    [Header("[Movement] Ground Check")]
    [SerializeField] private Transform groundCheck;
    [Range(0.05f, 0.5f)][SerializeField] private float groundCheckRadius = 0.2f;
    [SerializeField] private LayerMask groundMask;
    private bool isGrounded;
    private bool wasGrounded;
    private float airTime;

    [Header("Look")]
    [Range(0.1f, 5f)][SerializeField] private float lookSensitivity = 0.3f;
    [Range(40f, 90f)][SerializeField] private float pitchClamp = 70f;
    [SerializeField] private float baseFov = 70f;
    [SerializeField] private float runFov = 84f;
    [SerializeField] private float fovLerpSpeed = 12f;
    private float pitch;
    private Vector3 cameraBaseLocalPos;
    private float smoothedBaseFov;

    [Header("[Look] Strafe Tilt")]
    [SerializeField] private float maxTiltAngle = 6f;
    [SerializeField] private float tiltLerpSpeed = 11f;
    [SerializeField] private float lookTiltAmount = 0.22f;
    [SerializeField] private float maxLookTiltAngle = 2.5f;
    private float currentRoll;
    private float lookRoll;

    [Header("[Look] Landing Camera Dip")]
    [SerializeField] private float landingDipImpulse = 2.4f;
    [SerializeField] private float dipStiffness = 150f;
    [SerializeField] private float dipDamping = 13f;
    [SerializeField] private float landingImpactMinSpeed = 1.5f;
    [SerializeField] private float landingImpactMaxSpeed = 14f;
    [SerializeField] private float minAirTimeForLanding = 0.08f;
    private float dipOffset;
    private float dipVelocity;

    [Header("[Look] Landing FOV Punch")]
    [SerializeField] private float fovPunchImpulse = 110f;
    [SerializeField] private float fovPunchStiffness = 160f;
    [SerializeField] private float fovPunchDamping = 15f;
    private float fovPunchOffset;
    private float fovPunchVelocity;

    [Header("[Look] Headbob")]
    [SerializeField] private bool hasHeadbob = true;
    [SerializeField] private float walkBobFrequency = 8.5f;
    [SerializeField] private float walkBobAmplitude = 0.09f;
    [SerializeField] private float runBobFrequency = 12f;
    [SerializeField] private float runBobAmplitude = 0.19f;
    [Range(0f, 1.5f)][SerializeField] private float bobHorizontalRatio = 1f;
    [Range(0f, 1f)][SerializeField] private float forwardSwayScale = 0.12f;
    [SerializeField] private float forwardBobBoost = 1.25f;
    [SerializeField] private float bobRollAngle = 2.2f;
    [SerializeField] private float bobSmoothing = 14f;
    private Vector3 bobOffset;
    private float bobRoll;
    private float bobTimer;

    [Header("Dash")]
    [SerializeField] private float dashSpeed = 22f;
    [SerializeField] private float dashDuration = 0.18f;
    [SerializeField] private float dashCooldown = 1.2f;
    public float DashCooldownRemaining => Mathf.Max(0f, dashCooldownTimer);
    [SerializeField] private bool allowAirDash = true;
    private bool isDashing;
    private float dashCooldownTimer;
    private Vector3 dashDirection;

    [Header("[Dash] Camera Feel")]
    [SerializeField] private float dashFovAmount = 20f;
    [SerializeField] private float dashFovInSpeed = 24f;
    [SerializeField] private float dashFovOutSpeed = 6f;
    [SerializeField] private float dashTiltAngle = 7f;
    [SerializeField] private float dashTiltInSpeed = 20f;
    [SerializeField] private float dashTiltOutSpeed = 7f;
    private float dashFovOffset;
    private float dashRoll;

    public enum MovementState { Grounded, Airborne, Dashing }
    public MovementState CurrentState { get; private set; }

    public float? CameraZoomOverride { get; set; }

    public float BaseFov => baseFov;

    private bool allowMovement = true;
    private bool allowLook = true;

    // Lifecycle

    private void Awake()
    {
        cc = GetComponent<CharacterController>();
    }

    private void Start()
    {
        if (playerCamera == null)
        {
            Debug.LogError("[FPS_Controller] Player Camera reference is missing.");
            enabled = false;
            return;
        }

        playerCamera.fieldOfView = baseFov;
        smoothedBaseFov = baseFov;
        cameraBaseLocalPos = playerCamera.transform.localPosition;
    }

    private void Update()
    {
        HandleMovement();
        TickDashCooldown();
    }

    private void LateUpdate()
    {
        HandleLook();
        TickCameraSprings();

        ApplyHeadbob();
        ApplyCameraTilt();
    }

    // Movement

    private void HandleMovement()
    {
        if (!allowMovement || !cc.enabled) return;

        UpdateGroundedState();
        UpdateMovementState();

        Vector2 moveInput = InputManager.Instance.MoveInput;

        if (isGrounded)
        {
            isRunningState = InputManager.Instance.IsRunning && moveInput.y >= 0f;
            wasRunningOnGround = isRunningState;
        }
        else
        {
            isRunningState = wasRunningOnGround;
        }

        ApplyFov();
        ApplyGravity();

        HandleJump();
        HandleDashInput();

        Vector3 horizontalMove;

        if (isDashing)
        {
            horizontalMove = dashDirection * dashSpeed;
        }
        else
        {
            float targetSpeed = isRunningState ? runSpeed : walkSpeed;
            horizontalMove = (transform.right * moveInput.x + transform.forward * moveInput.y) * targetSpeed;
        }

        Vector3 finalMovement = horizontalMove * Time.deltaTime;
        finalMovement.y = velocity.y * Time.deltaTime;
        cc.Move(finalMovement);
    }


    private void ApplyGravity()
    {
        float g = velocity.y < 0f ? gravity * fallGravityMultiplier : gravity;
        velocity.y += g * Time.deltaTime;
        velocity.y = Mathf.Max(velocity.y, -maxFallSpeed);
    }

    private void ApplyFov()
    {
        float targetFov = isRunningState ? runFov : baseFov;
        smoothedBaseFov = Mathf.Lerp(smoothedBaseFov, targetFov, Time.deltaTime * fovLerpSpeed);

        float dashFovTarget = isDashing ? dashFovAmount : 0f;
        dashFovOffset = Mathf.Lerp(dashFovOffset, dashFovTarget,
            Time.deltaTime * (isDashing ? dashFovInSpeed : dashFovOutSpeed));


        float baseValue = CameraZoomOverride ?? smoothedBaseFov;
        playerCamera.fieldOfView = baseValue + fovPunchOffset + dashFovOffset;
    }

    private void UpdateGroundedState()
    {
        wasGrounded = isGrounded;
        isGrounded = Physics.CheckSphere(groundCheck.position, groundCheckRadius, groundMask, QueryTriggerInteraction.Ignore);

        if (isGrounded)
            coyoteTimer = coyoteTime;
        else
            coyoteTimer -= Time.deltaTime;

        if (!wasGrounded && isGrounded)
            OnLanded();

        airTime = isGrounded ? 0f : airTime + Time.deltaTime;

        if (isGrounded && velocity.y < 0f)
            velocity.y = -groundStickForce;
    }

    private void UpdateMovementState()
    {
        if (isDashing)
            CurrentState = MovementState.Dashing;
        else if (isGrounded)
            CurrentState = MovementState.Grounded;
        else
            CurrentState = MovementState.Airborne;
    }

    private void HandleJump()
    {
        bool canJumpNow = isGrounded || coyoteTimer > 0f;

        if (canJumpNow && InputManager.Instance.IsJumping)
        {
            velocity.y = jumpForce;
            coyoteTimer = 0f;
        }
    }

    private void OnLanded()
    {
        if (airTime < minAirTimeForLanding) return;

        float impactSpeed = Mathf.Abs(velocity.y);
        float t = Mathf.InverseLerp(landingImpactMinSpeed, landingImpactMaxSpeed, impactSpeed);
        if (t <= 0f) return;

        dipVelocity -= landingDipImpulse * t;
        fovPunchVelocity += fovPunchImpulse * t;
    }

    // ─── Dash ───────────────────────────────────────────────────────────────

    private void HandleDashInput()
    {
        if (isDashing || dashCooldownTimer > 0f) return;
        if (!isGrounded && !allowAirDash) return;
        if (!InputManager.Instance.IsDashing) return;

        Vector2 moveInput = InputManager.Instance.MoveInput;
        Vector3 inputDir = transform.right * moveInput.x + transform.forward * moveInput.y;

        dashDirection = inputDir.sqrMagnitude > 0.01f ? inputDir.normalized : transform.forward;

        StartCoroutine(DashRoutine());
    }

    private IEnumerator DashRoutine()
    {
        isDashing = true;
        yield return new WaitForSeconds(dashDuration);
        isDashing = false;
        dashCooldownTimer = dashCooldown;
    }

    private void TickDashCooldown()
    {
        if (dashCooldownTimer > 0f)
            dashCooldownTimer -= Time.deltaTime;
    }

    public float DashCooldownNormalized =>
        dashCooldown <= 0f ? 1f : 1f - Mathf.Clamp01(dashCooldownTimer / dashCooldown);

    // Look, Tilt, Dip, Bob:

    private void HandleLook()
    {
        if (!allowLook) return;

        if (Cursor.lockState != CursorLockMode.Locked)
        {
            Cursor.lockState = CursorLockMode.Locked;
            Cursor.visible = false;
        }

        Vector2 look = InputManager.Instance.LookInput * lookSensitivity;
        transform.Rotate(Vector3.up * look.x);

        pitch -= look.y;
        pitch = Mathf.Clamp(pitch, -pitchClamp, pitchClamp);

        float rawLookTilt = Mathf.Clamp(-look.x * lookTiltAmount, -maxLookTiltAngle, maxLookTiltAngle);
        lookRoll = Mathf.LerpAngle(lookRoll, rawLookTilt, Time.deltaTime * tiltLerpSpeed);
    }

    private void TickCameraSprings()
    {
        float dt = Mathf.Min(Time.deltaTime, 0.033f);

        Spring(ref dipOffset, ref dipVelocity, dipStiffness, dipDamping, dt);
        Spring(ref fovPunchOffset, ref fovPunchVelocity, fovPunchStiffness, fovPunchDamping, dt);
    }

    private static void Spring(ref float value, ref float velocity, float stiffness, float damping, float dt)
    {
        velocity += -value * stiffness * dt;
        velocity -= velocity * damping * dt;
        value += velocity * dt;
    }

    private void ApplyHeadbob()
    {
        Vector3 targetBob = Vector3.zero;
        float targetBobRoll = 0f;

        if (hasHeadbob && isGrounded && !isDashing)
        {
            Vector2 moveInput = InputManager.Instance.MoveInput;

            if (moveInput.magnitude > 0.1f)
            {
                float freq = isRunningState ? runBobFrequency : walkBobFrequency;
                float amp = isRunningState ? runBobAmplitude : walkBobAmplitude;

                bobTimer += Time.deltaTime * freq;

                float lateralAmount = Mathf.Abs(moveInput.normalized.x);
                float swayScale = Mathf.Lerp(forwardSwayScale, 1f, lateralAmount);
                float verticalScale = Mathf.Lerp(forwardBobBoost, 1f, lateralAmount);

                float sway = Mathf.Cos(bobTimer * 0.5f);
                targetBob.y = Mathf.Sin(bobTimer) * amp * verticalScale;
                targetBob.x = sway * amp * bobHorizontalRatio * swayScale;

                targetBobRoll = sway * bobRollAngle * swayScale * (amp / Mathf.Max(walkBobAmplitude, 0.0001f));
            }
        }

        bobOffset = Vector3.Lerp(bobOffset, targetBob, Time.deltaTime * bobSmoothing);
        bobRoll = Mathf.LerpAngle(bobRoll, targetBobRoll, Time.deltaTime * bobSmoothing);

        playerCamera.transform.localPosition = cameraBaseLocalPos + bobOffset + Vector3.up * dipOffset;
    }

    private void ApplyCameraTilt()
    {
        Vector2 moveInput = InputManager.Instance.MoveInput;
        float targetRoll = -moveInput.x * maxTiltAngle;
        currentRoll = Mathf.LerpAngle(currentRoll, targetRoll, Time.deltaTime * tiltLerpSpeed);

        float dashRollTarget = isDashing ? -Vector3.Dot(dashDirection, transform.right) * dashTiltAngle : 0f;
        dashRoll = Mathf.LerpAngle(dashRoll, dashRollTarget,
            Time.deltaTime * (isDashing ? dashTiltInSpeed : dashTiltOutSpeed));

        playerCamera.transform.localRotation = Quaternion.Euler(pitch, 0f, currentRoll + dashRoll + lookRoll + bobRoll);
    }

    // Public State Controls

    public void HandleMovementState(bool canMove)
    {
        allowMovement = canMove;
        if (!canMove) velocity = Vector3.zero;
    }

    public void HandleLookState(bool canLook)
    {
        allowLook = canLook;
        if (!canLook)
        {
            Cursor.lockState = CursorLockMode.None;
            Cursor.visible = true;
        }
    }

    public Camera GetPlayerCamera() => playerCamera;

    private void OnDrawGizmosSelected()
    {
        if (groundCheck != null)
        {
            Gizmos.color = Color.yellow;
            Gizmos.DrawWireSphere(groundCheck.position, groundCheckRadius);
        }
    }
}