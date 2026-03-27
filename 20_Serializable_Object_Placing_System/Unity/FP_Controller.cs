using UnityEngine;

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
    [Range(50f, 120f)] [SerializeField] private float runFov = 80f;
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

    private Vector3 velocity;
    private float pitch;
    private int jumpCount;

    [Header("Flashlight")]
    [SerializeField] private FP_FlashlightSystem flashlight;
    [HideInInspector] public bool canUseFlashlight = true;

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
    }

    private void Update()
    {
        HandleMovement();
        HandleInteraction();
    }

    void LateUpdate()
    {
        HandleLook();
        ApplyBobbing();
        HandleFlashlight();
    }

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

        // Update running state
        if (isGrounded)
        {
            // No sprinting backwards
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
                // Keep sprint if we jumped while sprinting
                isRunningState = wasRunningOnGround;
            }
        }

        float targetSpeed = isRunningState ? runSpeed : walkSpeed;

        float targetFov = isRunningState ? runFov : cameraFov;
        playerCamera.fieldOfView = Mathf.Lerp(playerCamera.fieldOfView, targetFov, Time.deltaTime * fovLerpSpeed);

        Vector3 move = (transform.right * moveInput.x) + (transform.forward * moveInput.y);
        move *= targetSpeed;

        // Jumping Logic
        HandleJumping();

        velocity.y += gravity * Time.deltaTime;

        // Steep slope push logic
        if (OnSteepSlope(out Vector3 slopeNormal, out float slopeAngle))
        {
            // Push direction = down the slope
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
        isGrounded = Physics.CheckSphere(groundCheck.position,groundCheckRadius,groundMask,QueryTriggerInteraction.Ignore);

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
                    // Debug.Log("Interacted with interactable: " + hit.collider.name);

                }
                else
                {
                    interactable.OnFocusEnter();
                    // Debug.Log("Looking at interactable: " + hit.collider.name);
                }
            }
            else
            {
                if (currentInteractable != null){
                    currentInteractable.OnFocusExit();
                    currentInteractable = null;
                    // Debug.Log("Not looking at any interactable");
                }
            }
        }
        else
        {
            if (currentInteractable != null){
                currentInteractable.OnFocusExit();
                currentInteractable = null;
                // Debug.Log("Not looking at any interactable");
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

        // Walking / running bob
        if (hasWalkingBob && isGrounded && isMoving)
        {
            float freq = isRunning ? runBobFrequency : walkBobFrequency;
            float amp = isRunning ? runBobAmplitude : walkBobAmplitude;
            targetLocalPos.y += Mathf.Sin(time * freq) * amp;
        }

        // Idle breathing
        if (hasBreathEffect && !isMoving && isGrounded)
        {
            targetLocalPos.y += Mathf.Sin(time * idleBreathFrequency) * idleBreathAmplitude;
        }

        // Landing dip
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

        // Smoothly interpolate camera local position
        playerCamera.transform.localPosition = Vector3.Lerp(playerCamera.transform.localPosition,targetLocalPos,Time.deltaTime * bobSmoothing);
    }

    private void HandleJumping()
    {
        if (!canJump)
        {
            return;
        }
        // Grounded logic
        if (isGrounded)
        {
            if (velocity.y < 0f)
                velocity.y = -2f;

            jumpCount = 0;

            if (InputManager.Instance.IsJumping)
            {
                velocity.y = jumpForce;
                jumpCount = 1;
                coyoteTimer = 0f; // consume coyote time
            }
        }
        else
        {
            // Coyote jump (only if we haven't jumped yet)
            if (coyoteTimer > 0f && InputManager.Instance.IsJumping && jumpCount == 0)
            {
                velocity.y = jumpForce;
                jumpCount = 1;
                coyoteTimer = 0f;
                return;
            }

            // Double jump (Coyote jump takes priority and replaces the double jump, if you want both, you can adjust the logic to allow a double jump after a coyote jump)
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


    // This can be called by interactables or other scripts to enable/disable movement
    public void HandleMovementState(bool canMove)
    {
        allowMovement = canMove;
        if (!canMove)
            velocity = Vector3.zero;
    }

    public void HandleJumpState(bool canJump)
    {
        this.canJump = canJump;
        if (!canJump)
            velocity.y = 0f;
    }

    // This can be called by interactables or other scripts to enable/disable looking around
    public void HandleLookState(bool canLook)
    {
        allowLook = canLook;
        if (!canLook)
        {
            Cursor.lockState = CursorLockMode.None;
            Cursor.visible = true;
        }
    }

    // This can be called by interactables or other scripts to enable/disable interaction
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

    public void setFullFreeze(bool freeze)
    {
        HandleBobbingState(!freeze);
        HandleMovementState(!freeze);
        HandleLookState(!freeze);
        HandleInteractionState(!freeze);
        HandleFlashlightState(!freeze);
        HandleJumpState(!freeze);
    }

    public Camera GetPlayerCamera()
    {
        return playerCamera;
    }

    // Debugging: visualize ground check sphere in editor (only when selected) [Make sure the sphere is slightly inside the ground to avoid false negatives due to precision issues]
    private void OnDrawGizmosSelected()
    {
        if (groundCheck == null) return;

        Gizmos.color = Color.yellow;
        Gizmos.DrawWireSphere(groundCheck.position, groundCheckRadius);
    }

}
