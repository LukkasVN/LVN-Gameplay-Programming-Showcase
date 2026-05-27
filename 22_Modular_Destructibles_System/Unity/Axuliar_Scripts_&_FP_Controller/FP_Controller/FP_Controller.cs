using System.Collections;
using UnityEngine;

/*
    Extension of the LVN Showcase FP_Controller (14_FirstPersonController).
    All original movement, look, bobbing, interaction and flashlight code is preserved unchanged.
    Added: HitWeaponSO-driven hit system, destructible focus tracking, and per-hit camera shake.
    Weapon switching is utility-only — call SetActiveWeapon(), NextWeapon(), or PreviousWeapon()
    from any external script. The hit system itself is fully data-driven via HitWeaponSO and DestructibleSO, with no hardcoded logic.

    - Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
*/

[RequireComponent(typeof(CharacterController))]
public class FP_Controller : MonoBehaviour
{
    [Header("References")]
    [SerializeField] private Camera playerCamera;
    private CharacterController cc;

    [Header("Movement")]
    [SerializeField] private float walkSpeed = 4.5f;
    [SerializeField] private float runSpeed = 7f;
    [SerializeField] private float gravity = -35f;
    [SerializeField] private float jumpForce = 6f;
    [SerializeField] private bool allowDoubleJump = false;
    [Range(0f, 1f)][SerializeField] private float coyoteTime = 0.12f;
    [SerializeField] private bool allowAirSprint = false;
    private float coyoteTimer = 0f;
    private bool isRunningState = false;
    private bool wasRunningOnGround = false;
    private bool allowMovement = true;
    private bool wasGrounded = false;

    [Header("[Movement] Ground Check")]
    [SerializeField] private Transform groundCheck;
    [Range(0.05f, 0.5f)][SerializeField] private float groundCheckRadius = 0.35f;
    [SerializeField] private LayerMask groundMask;
    private bool canJump = true;
    private bool isGrounded;

    [Header("[Movement] Steep Slope Push")]
    [SerializeField] private float maxSlopeAngle = 35f;
    [SerializeField] private float slopePushForce = 6f;

    [Header("Camera")]
    [Range(0.1f, 5f)][SerializeField] private float lookSensitivity = 0.3f;
    [Range(40f, 90f)][SerializeField] private float pitchClamp = 70f;
    [Range(50f, 120f)][SerializeField] private float cameraFov = 70f;
    private bool allowLook = true;
    private Vector3 cameraBaseLocalPos;
    private Quaternion cameraBaseLocalRot;

    [Header("[Camera] Camera Effects")]
    [Range(50f, 120f)][SerializeField] private float runFov = 80f;
    [SerializeField] private float fovLerpSpeed = 12f;

    [Header("[Bobbing] Camera Feedback Toggles")]
    [SerializeField] private bool hasBobbing = true;
    [SerializeField] private bool hasBreathEffect = true;
    [SerializeField] private bool hasLandingEffect = true;
    [SerializeField] private bool hasWalkingBob = true;

    [Header("[Bobbing] Camera Feedback Settings")]
    [SerializeField] private float bobSmoothing = 12f;
    [SerializeField] private float walkBobFrequency = 8f;
    [SerializeField] private float walkBobAmplitude = 0.03f;
    [SerializeField] private float runBobFrequency = 12f;
    [SerializeField] private float runBobAmplitude = 0.05f;
    [SerializeField] private float idleBreathFrequency = 1.2f;
    [SerializeField] private float idleBreathAmplitude = 0.015f;
    [SerializeField] private float landingDipAmount = -0.1f;
    [SerializeField] private float landingDipSpeed = 8f;
    private bool allowBobbing = true;
    private float landingLerp = 0f;

    [Header("Interaction")]
    [SerializeField] private float interactDistance = 5f;
    [SerializeField] private LayerMask interactMask;
    private IFP_Interactable currentInteractable;
    private bool allowInteraction = true;

    [Header("Flashlight")]
    [SerializeField] private FP_FlashlightSystem flashlight;
    [HideInInspector] public bool canUseFlashlight = true;

    // ── Hit System ────────────────────────────────────────────────────────────

    [Header("Hit System")]
    [Tooltip("Assign one HitWeaponSO per weapon the player can use. " +
             "Switch between them via SetActiveWeapon(), NextWeapon(), or PreviousWeapon().")]
    [SerializeField] private HitWeaponSO[] weapons;
    [SerializeField] private int activeWeaponIndex = 0;
    [SerializeField] private LayerMask hitMask;
    private bool allowHit = true;
    private bool isHitOnCooldown;
    private Destructible currentFocusedDestructible;

    // Camera shake state — kept separate from bobbing to avoid fighting localPosition
    private Coroutine shakeCoroutine;
    private float perlinOffsetX;
    private float perlinOffsetY;
    // ─────────────────────────────────────────────────────────────────────────

    /// <summary>The currently active weapon SO. Null if no weapons are assigned.</summary>
    public HitWeaponSO ActiveWeapon => (weapons != null && weapons.Length > 0)
        ? weapons[activeWeaponIndex]
        : null;

    private Vector3 velocity;
    private float pitch;
    private int jumpCount;

    // ─── Lifecycle ────────────────────────────────────────────────────────────

    private void Awake()
    {
        cc = GetComponent<CharacterController>();
    }

    private void Start()
    {
        if (playerCamera == null)
        {
            Debug.LogError("Player Camera reference is missing!");
            enabled = false;
            return;
        }

        playerCamera.fieldOfView = cameraFov;
        cameraBaseLocalPos = playerCamera.transform.localPosition;
        cameraBaseLocalRot = playerCamera.transform.localRotation;

        if (weapons == null || weapons.Length == 0)
            Debug.LogWarning("[FP_Controller] No weapons assigned. Hit system will be inactive.");
    }

    private void Update()
    {
        HandleMovement();
        HandleInteraction();
        HandleFocus();
        HandleHit();
    }

    void LateUpdate()
    {
        HandleLook();
        ApplyBobbing();
        HandleFlashlight();
    }

    #region Hit/Focus Destructible Handling Logic
    private void HandleFocus()
    {
        if (ActiveWeapon == null) return;

        Ray ray = new Ray(playerCamera.transform.position, playerCamera.transform.forward);

        if (Physics.Raycast(ray, out RaycastHit hit, ActiveWeapon.hitRange, hitMask))
        {
            if (hit.collider.TryGetComponent(out Destructible destructible))
            {
                if (destructible != currentFocusedDestructible)
                {
                    currentFocusedDestructible?.OnFocusExit();
                    currentFocusedDestructible = destructible;
                    currentFocusedDestructible.OnFocusEnter();
                }
                return;
            }
        }

        if (currentFocusedDestructible != null)
        {
            currentFocusedDestructible.OnFocusExit();
            currentFocusedDestructible = null;
        }
    }

    private void HandleHit()
    {
        if (!allowHit || isHitOnCooldown || ActiveWeapon == null) return;
        if (!InputManager.Instance.isHitInteracting) return;

        // Audio Hookup example (uncomment swingSFX in HitWeaponSO and wire it to your audio system to play a sound on hit attempt):
        // Audio: AudioManager.Instance.PlaySFX(ActiveWeapon.swingSFX);

        Ray ray = new Ray(playerCamera.transform.position, playerCamera.transform.forward);

        if (Physics.Raycast(ray, out RaycastHit hit, ActiveWeapon.hitRange, hitMask))
        {
            if (hit.collider.TryGetComponent(out Destructible destructible))
            {
                Vector3 hitDir = (hit.point - playerCamera.transform.position).normalized;
                destructible.TakeDamage(ActiveWeapon.damage, ActiveWeapon.tierLevel, hit.point, hitDir);

                if (ActiveWeapon.useHitShake) // Optional shake on hit that can be used on non-destructible objects too since it's purely visual feedback
                {
                    if (shakeCoroutine != null) StopCoroutine(shakeCoroutine);
                    shakeCoroutine = StartCoroutine(CameraShakeRoutine(ActiveWeapon.shakeDuration, ActiveWeapon.shakeIntensity));
                }
            }
        }

        StartCoroutine(HitCooldownRoutine(ActiveWeapon.hitCooldown));
        UIManager.Instance.ShowHitCooldown(ActiveWeapon.hitCooldown);
    }

    private IEnumerator HitCooldownRoutine(float cooldown)
    {
        isHitOnCooldown = true;
        yield return new WaitForSeconds(cooldown);
        isHitOnCooldown = false;
    }

    private IEnumerator CameraShakeRoutine(float duration, float intensity)
    {
        Vector3 originalPos = cameraBaseLocalPos;
        float elapsed = 0f;
        perlinOffsetX = Random.Range(0f, 100f);
        perlinOffsetY = Random.Range(0f, 100f);

        while (elapsed < duration)
        {
            elapsed += Time.deltaTime;
            float decay = 1f - (elapsed / duration);

            float x = Mathf.PerlinNoise(elapsed * 10f + perlinOffsetX, 0f) * 2f - 1f;
            float y = Mathf.PerlinNoise(0f, elapsed * 10f + perlinOffsetY) * 2f - 1f;

            playerCamera.transform.localPosition = originalPos + new Vector3(x, y, 0f) * intensity * decay;
            yield return null;
        }

        playerCamera.transform.localPosition = originalPos;
        shakeCoroutine = null;
    }

    #endregion

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

        playerCamera.transform.localRotation = Quaternion.Euler(pitch, 0f, 0f);
    }

    private void HandleMovement()
    {
        if (!allowMovement || !cc.enabled)
            return;

        IsGrounded();

        Vector2 moveInput = InputManager.Instance.MoveInput;

        if (isGrounded)
        {
            if (moveInput.y < 0f)
                isRunningState = false;
            else
                isRunningState = InputManager.Instance.IsRunning;

            wasRunningOnGround = isRunningState;
        }
        else
        {
            if (allowAirSprint)
            {
                if (moveInput.y < 0f)
                    isRunningState = false;
                else
                    isRunningState = InputManager.Instance.IsRunning;
            }
            else
            {
                isRunningState = wasRunningOnGround;
            }
        }

        float targetSpeed = isRunningState ? runSpeed : walkSpeed;

        float targetFov = isRunningState ? runFov : cameraFov;
        playerCamera.fieldOfView = Mathf.Lerp(playerCamera.fieldOfView, targetFov, Time.deltaTime * fovLerpSpeed);

        Vector3 move = (transform.right * moveInput.x) + (transform.forward * moveInput.y);
        move *= targetSpeed;

        HandleJumping();

        velocity.y += gravity * Time.deltaTime;

        if (OnSteepSlope(out Vector3 slopeNormal, out float slopeAngle))
        {
            Vector3 pushDir = Vector3.ProjectOnPlane(Vector3.down, slopeNormal).normalized;
            cc.Move(pushDir * slopePushForce * Time.deltaTime);
            isGrounded = false;
        }

        Vector3 finalMovement = move * Time.deltaTime;
        finalMovement.y = velocity.y * Time.deltaTime;

        cc.Move(finalMovement);
    }

    private void IsGrounded()
    {
        isGrounded = Physics.CheckSphere(groundCheck.position, groundCheckRadius, groundMask, QueryTriggerInteraction.Ignore);

        if (isGrounded)
            coyoteTimer = coyoteTime;
        else
            coyoteTimer -= Time.deltaTime;
    }

    private void HandleInteraction()
    {
        if (!allowInteraction) return;

        Ray ray = new Ray(playerCamera.transform.position, playerCamera.transform.forward);

        if (Physics.Raycast(ray, out RaycastHit hit, interactDistance, interactMask))
        {
            if (hit.collider.TryGetComponent(out IFP_Interactable interactable))
            {
                currentInteractable = interactable;
                if (InputManager.Instance.isInteracting)
                {
                    interactable.OnInteract(playerCamera.transform.forward);
                }
                else
                {
                    interactable.OnFocusEnter();
                }
            }
            else
            {
                if (currentInteractable != null)
                {
                    currentInteractable.OnFocusExit();
                    currentInteractable = null;
                }
            }
        }
        else
        {
            if (currentInteractable != null)
            {
                currentInteractable.OnFocusExit();
                currentInteractable = null;
            }
        }
    }

    private void ApplyBobbing()
    {
        if (!hasBobbing || !allowBobbing) return;

        Vector3 targetLocalPos = cameraBaseLocalPos;
        float time = Time.time;

        Vector2 moveInput = InputManager.Instance.MoveInput;
        bool isMoving = moveInput.magnitude > 0.1f;
        bool isRunning = isRunningState;

        if (hasWalkingBob && isGrounded && isMoving)
        {
            float freq = isRunning ? runBobFrequency : walkBobFrequency;
            float amp = isRunning ? runBobAmplitude : walkBobAmplitude;
            targetLocalPos.y += Mathf.Sin(time * freq) * amp;
        }

        if (hasBreathEffect && !isMoving && isGrounded)
            targetLocalPos.y += Mathf.Sin(time * idleBreathFrequency) * idleBreathAmplitude;

        if (hasLandingEffect)
        {
            if (!wasGrounded && isGrounded)
                landingLerp = 1f;

            if (landingLerp > 0f)
            {
                targetLocalPos.y += landingDipAmount * landingLerp;
                landingLerp -= Time.deltaTime * landingDipSpeed;
            }
        }

        wasGrounded = isGrounded;

        // Skip lerp while shake is running to avoid fighting localPosition
        if (shakeCoroutine == null)
            playerCamera.transform.localPosition = Vector3.Lerp(
                playerCamera.transform.localPosition, targetLocalPos, Time.deltaTime * bobSmoothing);
    }

    private void HandleJumping()
    {
        if (!canJump) return;

        if (isGrounded)
        {
            if (velocity.y < 0f)
                velocity.y = -2f;

            jumpCount = 0;

            if (InputManager.Instance.IsJumping)
            {
                velocity.y = jumpForce;
                jumpCount = 1;
                coyoteTimer = 0f;
            }
        }
        else
        {
            if (coyoteTimer > 0f && InputManager.Instance.IsJumping && jumpCount == 0)
            {
                velocity.y = jumpForce;
                jumpCount = 1;
                coyoteTimer = 0f;
                return;
            }

            if (allowDoubleJump && InputManager.Instance.IsJumping && jumpCount < 1)
            {
                float requiredBoost = jumpForce - velocity.y;
                velocity.y += requiredBoost;
                jumpCount++;
            }
        }
    }

    private bool OnSteepSlope(out Vector3 slopeNormal, out float slopeAngle)
    {
        slopeNormal = Vector3.up;
        slopeAngle = 0f;

        Vector3 origin = groundCheck.position + Vector3.up * 0.1f;

        if (Physics.Raycast(origin, Vector3.down, out RaycastHit hit, 0.6f, groundMask))
        {
            slopeNormal = hit.normal;
            slopeAngle = Vector3.Angle(hit.normal, Vector3.up);
            return slopeAngle > maxSlopeAngle;
        }

        return false;
    }

    private void HandleFlashlight()
    {
        if (!canUseFlashlight) return;

        if (InputManager.Instance.IsFlashlightOn)
            flashlight.ToggleFlashlight();
    }

    // ─── Public State Controls ────────────────────────────────────────────────

    public void HandleMovementState(bool canMove)
    {
        allowMovement = canMove;
        if (!canMove)
            velocity = Vector3.zero;
    }

    public void HandleJumpState(bool state)
    {
        this.canJump = state;
        if (!state)
            velocity.y = 0f;
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

    public void HandleInteractionState(bool canInteract)
    {
        allowInteraction = canInteract;
    }

    public void HandleFlashlightState(bool canUse)
    {
        canUseFlashlight = canUse;
    }

    public void HandleBobbingState(bool canBob)
    {
        allowBobbing = canBob;
    }

    public void HandleHitState(bool canHit)
    {
        allowHit = canHit;
        if (!canHit && currentFocusedDestructible != null)
        {
            currentFocusedDestructible.OnFocusExit();
            currentFocusedDestructible = null;
        }
    }

    public void setFullFreeze(bool freeze)
    {
        HandleBobbingState(!freeze);
        HandleMovementState(!freeze);
        HandleLookState(!freeze);
        HandleInteractionState(!freeze);
        HandleFlashlightState(!freeze);
        HandleJumpState(!freeze);
        HandleHitState(!freeze);
    }

    #region Weapon Utility Methods
    /// <summary>
    /// Switch to the weapon at the given index. Resets cooldown and clears focus.
    /// </summary>
    public void SetActiveWeapon(int index)
    {
        if (weapons == null || index < 0 || index >= weapons.Length) return;
        if (index == activeWeaponIndex) return;

        if (currentFocusedDestructible != null)
        {
            currentFocusedDestructible.OnFocusExit();
            currentFocusedDestructible = null;
        }

        activeWeaponIndex = index;
        isHitOnCooldown = false;
        Debug.Log($"[FP_Controller] Active weapon → {ActiveWeapon?.weaponName ?? "None"}");
    }

    /// <summary>
    /// Switch to the next weapon in the array, wrapping around.
    /// </summary>
    public void NextWeapon() =>
        SetActiveWeapon((activeWeaponIndex + 1) % (weapons?.Length ?? 1));

    /// <summary>
    /// Switch to the previous weapon in the array, wrapping around.
    /// </summary>
    public void PreviousWeapon() =>
        SetActiveWeapon((activeWeaponIndex - 1 + (weapons?.Length ?? 1)) % (weapons?.Length ?? 1));

    #endregion

    public Camera GetPlayerCamera()
    {
        return playerCamera;
    }

    // Gizmos

    private void OnDrawGizmosSelected()
    {
        if (groundCheck != null)
        {
            Gizmos.color = Color.yellow;
            Gizmos.DrawWireSphere(groundCheck.position, groundCheckRadius);
        }

        if (playerCamera != null && ActiveWeapon != null)
        {
            Gizmos.color = Color.red;
            Gizmos.DrawRay(playerCamera.transform.position,
                           playerCamera.transform.forward * ActiveWeapon.hitRange);
        }
    }
}