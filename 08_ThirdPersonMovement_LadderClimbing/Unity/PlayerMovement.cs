using System.Collections;
using UnityEngine;

[RequireComponent(typeof(CharacterController))]
public class PlayerMovement : MonoBehaviour
{
    public Transform playerMesh;
    private ThirdPersonCamera thirdPersonCamera;
    [Header("Movement Settings")]
    [SerializeField] private float walkSpeed = 2f;
    [SerializeField] private float runSpeed = 5f;
    [SerializeField] private float rotationSpeed = 4f;
    [SerializeField] private float gravity = -20f;
    [SerializeField] private float jumpForce = 10f;
    [SerializeField] private float flipForce = 8f;
    private Vector3 _velocity;

    [Header("Falling Settings")]
    [SerializeField] private float fallingVelocityThreshold = -1f;
    [SerializeField] private float fallingRayLength = 0.1f;
    [SerializeField] private float fallGraceTime = 0.05f;
    private bool _isFalling;
    private float _fallTimer;

    [Header("Jump Settings")]
    [SerializeField] private bool allowDoubleJump = true;
    [SerializeField] private float jumpInputBufferTime = 0.01f;
    private bool _jumpInputQueued;
    private float _jumpInputTimer;
    private bool _jumpPending;
    private int _jumpCount;
    private bool _isFlipping;

    [Header("Crouch Settings")]
    [SerializeField] private float ceilingCheckOffset = 0.15f;
    [SerializeField] private float crouchHeight = 1.0f;
    [SerializeField] private float crouchCenterY = 0.57f;
    [SerializeField] private float standHeight = 1.67f;
    [SerializeField] private float standCenterY = 0.9f;
    [SerializeField] private float crouchSpeed = 1.67f;
    [SerializeField] private float crouchBackwardsSpeed = 1.5f;
    private bool _isCrouching;

    [Header("Prone Settings")]
    [SerializeField] private float proneHeight = 0.4f;
    [SerializeField] private float proneCenterY = 0.2f;
    [SerializeField] private float proneSpeed = 1.25f;
    [SerializeField] private float proneBackwardsSpeed = 1.0f;
    private bool _isProning;

    [Header("Slide Settings")]
    [SerializeField] private float slideHeight = 0.2f;
    [SerializeField] private float slideCenterY = 0.4f;
    private bool _isSliding;
    [SerializeField] private float slideFallGraceTime = 1f;
    [SerializeField] private float slideSlopeBoost = 15f;
    [SerializeField] private float slideFriction = 1f;
    [SerializeField] private float minSlideSpeed = 3.5f;
    [SerializeField] private float flatSlideBoost = 2.5f;
    private float _slideFallTimer;
    private Vector3 _slideVelocity;

    [Header("Roll Settings")]
    [SerializeField] private float defaultRollSpeed = 7f;
    [SerializeField] private float rollRunSpeedMultiplier = 1.35f;
    [SerializeField] private float rollGravityDivider = 3f;
    private float rollSpeed = 7f;
    [SerializeField] private float rollDuration = 0.8f;
    private bool _isRolling;
    private float _rollTimer;
    private Vector3 _rollDirection;

    [Header("Ledge Settings")]
    [SerializeField] private float ledgeAlignSpeed = 5f;
    [SerializeField] private float ledgeSpeed = 2f;
    [SerializeField] private float ledgeWallOffset = 0.3f;
    private bool _lastIdleLeft;
    private bool _isOnLedge;
    private bool _ledgeExitCooldown = false;
    private Transform _currentLedge;
    private Vector3 _ledgeForward;
    private Vector3 _ledgeInward;
    private float _ledgeT;
    
    [Header("Glide Settings")]
    [SerializeField] private float glideGravity = -1f;
    [SerializeField] private float glideSpeed = 4f;
    [SerializeField] private float glideRotationSpeed = 3f;
    [SerializeField] private float minGlideActivationHeight = 5f;
    [SerializeField] private float glideFallVelocityThreshold = -2f;
    [SerializeField] private GameObject glider;
    private bool _isGliding;

    [Header("Ladder Climbing Settings")]
    [SerializeField] private float ladderClimbSpeed = 2.5f;
    [SerializeField] private float ladderOffset = 0.3f;
    [SerializeField] private float ladderRotationOffset = 180f;
    [SerializeField] private float climbAnimationTime = 1.0f;
    private bool _isLadderClimbing;
    private bool _isExitingLadder;
    private Transform _currentLadder;
    private Vector3 _ladderFaceDir;
    private Transform _ladderCandidate;
    private bool _isAtTopOfLadder;

    #if UNITY_EDITOR
    [Header("Unity Editor Settings")]
    [SerializeField] private Color standUpCheckColor = Color.blue;
    [SerializeField] private Color ladderGizmoColor = Color.yellow;
    #endif
    
    private CharacterController _controller;
    private Animator _animator;
    private Transform _mainCamera;

    private void Start()
    {
        _controller = GetComponent<CharacterController>();
        _animator = GetComponentInChildren<Animator>();
        _mainCamera = Camera.main.transform;
        thirdPersonCamera = Camera.main.GetComponent<ThirdPersonCamera>();
    }

    private void Update()
    {
        if (_isOnLedge)
        {
            HandleLedgeMovement();
            return;
        }

        // If exiting ladder, freeze and wait for animation
        if (_isExitingLadder)
        {
            return;
        }

        if (_isLadderClimbing)
        {
            HandleLadderClimbing();
            return;
        }

        // Press to start climbing ladder
        if (_ladderCandidate != null && InputManager.Instance.IsClimbing && !_isLadderClimbing)
        {
            TryStartLadderClimb(_ladderCandidate);
            return;
        }

        QueueJumpInput();

        if (_isRolling)
        {
            UpdateRoll();
            return;
        }

        HandleMovement();
        UpdateFalling();
        HandleGliding();

        if (InputManager.Instance.IsRolling && !_isRolling)
        {
            StartRoll();
        }
    }

    private bool IsGrounded()
    {
        return (_controller.collisionFlags & CollisionFlags.Below) != 0
               || Physics.Raycast(transform.position, Vector3.down, fallingRayLength);
    }

    private void QueueJumpInput()
    {
        if (InputManager.Instance.IsJumping)
        {
            _jumpInputQueued = true;
            _jumpInputTimer = jumpInputBufferTime;
        }

        if (_jumpInputQueued)
        {
            _jumpInputTimer -= Time.deltaTime;
            if (_jumpInputTimer <= 0f)
            {
                _jumpInputQueued = false;
            }
        }
    }

    #region Ladder Climbing

    private void TryStartLadderClimb(Transform ladderRoot)
    {
        if (!InputManager.Instance.IsClimbing) return;
        if (_isRolling || _isSliding || _isCrouching || _isProning || _isFlipping)
            return;

        _isLadderClimbing = true;
        _currentLadder = ladderRoot;
        _isAtTopOfLadder = false;

        Collider ladderCollider = ladderRoot.GetComponent<Collider>();
        Vector3 ladderCenter = ladderCollider != null ? ladderCollider.bounds.center : ladderRoot.position;

        Vector3 toPlayer = (transform.position - ladderCenter).normalized;
        float dot = Vector3.Dot(toPlayer, ladderRoot.forward);
        _ladderFaceDir = dot >= 0 ? ladderRoot.forward : -ladderRoot.forward;

        Vector3 targetPos = ladderCenter;
        targetPos.y = transform.position.y;
        targetPos += _ladderFaceDir * ladderOffset;
        transform.position = targetPos;

        _controller.Move(Vector3.up * 0.1f);

        Vector3 facingDirection = Quaternion.Euler(0f, ladderRotationOffset, 0f) * (-_ladderFaceDir);
        transform.rotation = Quaternion.LookRotation(facingDirection);

        _animator.SetBool("IsLadderClimbing", true);
        _animator.SetBool("IsLadderClimbingDown", false);
        _animator.speed = 1f;
        _velocity = Vector3.zero;

        // Set initial climbing animation frame to mimic an "Idle" pose on ladder (Programmer art workaround hehe)
        _animator.Play("ClimbingLadder", 0, 0.1f);
        _animator.Update(0f);  
    }
    

    private void HandleLadderClimbing()
    {
        if (!_isLadderClimbing) return;

        Vector2 input = InputManager.Instance.MoveInput;
        float verticalInput = input.y;

        // Exit at bottom while moving down
        if (verticalInput < -0.1f && IsGrounded())
        {
            ExitLadderAtBottom();
            return;
        }

        if (IsGrounded() && Mathf.Abs(verticalInput) < 0.1f)
        {
            ExitLadderAtBottom();
            return;
        }

        if (Mathf.Abs(verticalInput) > 0.1f)
        {
            Vector3 climbVelocity = Vector3.up * verticalInput * ladderClimbSpeed * Time.deltaTime;
            _controller.Move(climbVelocity);
        }

        // Keep centered on ladder
        Collider ladderCollider = _currentLadder.GetComponent<Collider>();
        Vector3 ladderCenter = ladderCollider != null ? ladderCollider.bounds.center : _currentLadder.position;

        Vector3 attachPos = ladderCenter;
        attachPos.y = transform.position.y;
        attachPos += _ladderFaceDir * ladderOffset;

        Vector3 correction = attachPos - transform.position;
        correction.y = 0;
        _controller.Move(correction);

        _animator.SetBool("IsLadderClimbing", true);

        if (verticalInput > 0.1f)
        {
            _animator.SetBool("IsLadderClimbingDown", false);
            _animator.speed = 1f;
        }
        else if (verticalInput < -0.1f)
        {
            _animator.SetBool("IsLadderClimbingDown", true);
            _animator.speed = 1f;
        }
        else
        {
            _animator.SetBool("IsLadderClimbingDown", false);
            _animator.speed = 0f;
        }
    }

    private void ExitLadderAtBottom()
    {
        _isLadderClimbing = false;
        _currentLadder = null;
        _isAtTopOfLadder = false;

        _animator.SetBool("IsLadderClimbing", false);
        _animator.SetBool("IsLadderClimbingDown", false);
        _animator.speed = 1f;
    }

    private void StartLadderTopExit()
    {
        Debug.Log("Starting ladder top exit");
        _isExitingLadder = true;
        _isLadderClimbing = false;

        _animator.SetBool("IsExitingLadder", true);
        _animator.SetBool("IsLadderClimbing", false);
        _animator.SetBool("IsLadderClimbingDown", false);
        _animator.speed = 1f;

        _velocity = Vector3.zero;
        Invoke(nameof(OnLadderExitAnimationComplete), climbAnimationTime);
        thirdPersonCamera.ChangeTarget(playerMesh);
    }

    public void OnLadderExitAnimationComplete()
    {
        CancelInvoke(nameof(OnLadderExitAnimationComplete));

        if (_currentLadder == null)
        {
            FinishLadderExitCleanup();
            return;
        }

        Transform exitPoint = _currentLadder.Find("ExitPoint");

        _velocity = Vector3.zero;
        _animator.SetBool("IsFalling", false);
        _animator.SetBool("IsJumping", false);

        if (_controller != null) _controller.enabled = false;

        Vector3 targetPos = exitPoint != null ? exitPoint.position : _currentLadder.position;

        transform.position = targetPos;

        if (_controller != null)
        {
            _controller.enabled = true;
        }

        FinishLadderExitCleanup();
    }

    private void FinishLadderExitCleanup()
    {
        _isExitingLadder = false;
        _animator.SetBool("IsExitingLadder", false);

        _currentLadder = null;
        _isAtTopOfLadder = false;
        _velocity = Vector3.zero;
        thirdPersonCamera.ChangeTarget(transform);
    }

    #endregion

    private void HandleGliding()
    {
        bool grounded = IsGrounded();
        bool glideInput = InputManager.Instance.IsGliding;
        
        bool canActivateGlide = !grounded && 
                                _velocity.y < glideFallVelocityThreshold && 
                                !_isFlipping &&
                                !Physics.Raycast(transform.position, Vector3.down, minGlideActivationHeight);

        if (glideInput && canActivateGlide && !_isGliding)
        {
            glider.SetActive(true);
            _isGliding = true;
            _animator.SetBool("IsGliding", true);
            _animator.SetBool("IsFalling", false);
        }

        if (_isGliding && (grounded || !glideInput || _isFlipping))
        {
            glider.SetActive(false);
            _isGliding = false;
            _animator.SetBool("IsGliding", false);
            if (!grounded)
            {
                _animator.SetBool("IsFalling", true);
            }
        }

        if (_isGliding)
        {
            Vector2 input = InputManager.Instance.MoveInput;
            Vector3 camForward = _mainCamera.forward;
            Vector3 camRight = _mainCamera.right;
            camForward.y = 0f;
            camRight.y = 0f;
            Vector3 moveDir = (camForward * input.y + camRight * input.x).normalized;

            _velocity.y = Mathf.Max(_velocity.y + glideGravity * Time.deltaTime, glideGravity * 2f);

            Vector3 horizontalVelocity = moveDir * glideSpeed;
            Vector3 glideVelocity = (horizontalVelocity + Vector3.up * _velocity.y) * Time.deltaTime;
            _controller.Move(glideVelocity);

            if (moveDir.magnitude > 0.1f)
            {
                Quaternion targetRot = Quaternion.LookRotation(moveDir);
                transform.rotation = Quaternion.Slerp(transform.rotation, targetRot, glideRotationSpeed * Time.deltaTime);
            }
        }
    }

    private void HandleMovement()
    {
        if (_isGliding) return;

        Vector2 input = InputManager.Instance.MoveInput;
        bool runPressed = InputManager.Instance.IsRunning;
        bool dancePressed = InputManager.Instance.IsDancing;
        bool crouchTogglePressed = InputManager.Instance.IsCrouching;
        bool crouchButtonPressed = InputManager.Instance.CrouchButtonPressed;
        bool proneTogglePressed = InputManager.Instance.IsProning;
        bool isIdle = input.magnitude < 0.1f;

        Vector3 camForward = _mainCamera.forward;
        Vector3 camRight = _mainCamera.right;
        camForward.y = 0f;
        camRight.y = 0f;
        Vector3 moveDir = camForward * input.y + camRight * input.x;
        moveDir.Normalize();

        bool grounded = IsGrounded();

        if (grounded && _velocity.y < 0)
        {
            _velocity.y = Mathf.Max(_velocity.y, -2f);
            _jumpCount = 0;
            _isFlipping = false;
            _animator.SetBool("IsJumping", false);
            _animator.SetBool("IsFalling", false);

            if (_jumpInputQueued && !_jumpPending && !_isCrouching && !_isProning)
            {
                _animator.SetTrigger("JumpTrigger");
                _jumpPending = true;
                _jumpInputQueued = false;
                _jumpCount++;
            }
        }
        else
        {
            _velocity.y += gravity * Time.deltaTime;

            bool jumpPressed = _jumpInputQueued || InputManager.Instance.IsJumping;

            if (allowDoubleJump
                && jumpPressed
                && !_jumpPending
                && !_isFlipping
                && !grounded
                && _jumpCount == 0
                && !_isCrouching
                && !_isProning)
            {
                _animator.SetTrigger("AirJumpTrigger");
                _jumpPending = true;
                _jumpInputQueued = false;
                _jumpCount++;
            }
        }

        bool CancelSlideCondition() =>
            !crouchButtonPressed || _slideVelocity.magnitude < minSlideSpeed || (!grounded && _slideFallTimer > slideFallGraceTime);

        void CancelSlide()
        {
            _isSliding = false;
            _animator.SetBool("IsSliding", false);
            _animator.ResetTrigger("Slide_Trigger");

            bool standAllowed = CanStandUp();
            bool crouchAllowed = CanCrouchUp();

            if (standAllowed && crouchAllowed)
            {
                _controller.height = standHeight;
                _controller.center = new Vector3(0f, standCenterY, 0f);
                _isCrouching = false;
                _isProning = false;
                _animator.SetBool("IsCrouching", false);
                _animator.SetBool("IsProning", false);
                _animator.ResetTrigger("CrouchTrigger");
            }
            else if (crouchAllowed)
            {
                _controller.height = crouchHeight;
                _controller.center = new Vector3(0f, crouchCenterY, 0f);
                _isCrouching = true;
                _isProning = false;
                _animator.SetBool("IsCrouching", true);
                _animator.SetBool("IsProning", false);
                _animator.SetTrigger("CrouchTrigger");
            }
            else
            {
                _controller.height = proneHeight;
                _controller.center = new Vector3(0f, proneCenterY, 0f);
                _isCrouching = true;
                _isProning = true;
                _animator.SetBool("IsCrouching", true);
                _animator.SetBool("IsProning", true);
                _animator.SetTrigger("ProneTrigger");
            }
        }

        bool airborneSlideAllowed = !grounded && _velocity.y < 0f;
        bool canSlide = runPressed && input.magnitude > 0.1f && !_isSliding && !_isCrouching && !_isProning && (grounded || airborneSlideAllowed);
        if (!_isSliding && crouchButtonPressed && canSlide)
        {
            _isSliding = true;
            _slideVelocity = moveDir * runSpeed;
            _controller.height = slideHeight;
            _controller.center = new Vector3(0f, slideCenterY, 0f);
            _animator.SetBool("IsSliding", true);
            _animator.ResetTrigger("Slide_Trigger");
            _animator.SetTrigger("Slide_Trigger");
            _slideFallTimer = 0f;
        }

        if (_isSliding)
        {
            Vector3 groundNormal = Vector3.up;
            if (Physics.Raycast(transform.position, Vector3.down, out RaycastHit hit, 1.5f))
            {
                groundNormal = hit.normal;
            }

            Vector3 slopeDir = Vector3.ProjectOnPlane(Vector3.down, groundNormal).normalized;
            _slideVelocity += slopeDir * slideSlopeBoost * Time.deltaTime;
            _slideVelocity += moveDir * flatSlideBoost * Time.deltaTime;
            _slideVelocity = Vector3.Lerp(_slideVelocity, Vector3.zero, slideFriction * Time.deltaTime);

            if (!grounded)
            {
                _slideFallTimer += Time.deltaTime;
            }
            else
            {
                _slideFallTimer = 0f;
            }

            if (CancelSlideCondition())
            {
                CancelSlide();
            }
        }

        if (proneTogglePressed && _isCrouching && !_isProning)
        {
            _controller.height = proneHeight;
            _controller.center = new Vector3(0f, proneCenterY, 0f);
            _isProning = true;
            _animator.SetBool("IsProning", true);
            _animator.SetTrigger("ProneTrigger");
        }
        else if (proneTogglePressed && _isProning)
        {
            if (CanCrouchUp())
            {
                _controller.height = crouchHeight;
                _controller.center = new Vector3(0f, crouchCenterY, 0f);
                _isProning = false;
                _isCrouching = true;
                _animator.SetBool("IsProning", false);
                _animator.SetBool("IsCrouching", true);
                _animator.SetTrigger("ProneTrigger");
            }
        }

        if (crouchTogglePressed && !runPressed && !_isProning && !_isSliding && IsGrounded())
        {
            if (_isCrouching)
            {
                if (CanStandUp())
                {
                    _controller.height = standHeight;
                    _controller.center = new Vector3(0f, standCenterY, 0f);
                    _animator.ResetTrigger("CrouchTrigger");
                    _isCrouching = false;
                    _animator.SetBool("IsCrouching", false);
                }
                else
                {
                    return;
                }
            }
            else
            {
                _isCrouching = true;
                _controller.height = crouchHeight;
                _controller.center = new Vector3(0f, crouchCenterY, 0f);
                _animator.SetTrigger("CrouchTrigger");
                _animator.SetBool("IsCrouching", true);
            }
        }

        bool movingBackward = Vector3.Dot(new Vector3(moveDir.x, 0f, moveDir.z), _mainCamera.forward) < -0.1f;

        float targetSpeed;
        if (_isSliding)
        {
            targetSpeed = _slideVelocity.magnitude;
        }
        else if (_isProning)
        {
            targetSpeed = movingBackward ? proneBackwardsSpeed : proneSpeed;
        }
        else if (_isCrouching)
        {
            targetSpeed = movingBackward ? crouchBackwardsSpeed : crouchSpeed;
        }
        else if (runPressed && input.y >= 0f && !movingBackward)
        {
            targetSpeed = runSpeed;
        }
        else
        {
            targetSpeed = movingBackward ? walkSpeed * 0.85f : walkSpeed;
        }

        Vector3 horizontalVelocity = moveDir * targetSpeed;
        Vector3 finalVelocity = _isSliding
            ? (_slideVelocity + Vector3.up * _velocity.y) * Time.deltaTime
            : (horizontalVelocity + Vector3.up * _velocity.y) * Time.deltaTime;

        _controller.Move(finalVelocity);

        if (dancePressed && isIdle && !_animator.GetBool("IsFalling") && !_animator.GetBool("IsDancing"))
        {
            _animator.SetBool("IsDancing", true);
            _animator.SetTrigger("IsDancingTrigger");
        }
        else if (!isIdle || _animator.GetBool("IsFalling"))
        {
            _animator.SetBool("IsDancing", false);
        }

        if (moveDir.magnitude > 0.1f)
        {
            Quaternion targetRot = movingBackward ? Quaternion.LookRotation(-moveDir) : Quaternion.LookRotation(moveDir);
            transform.rotation = Quaternion.Slerp(transform.rotation, targetRot, rotationSpeed * Time.deltaTime);
        }

        bool walkingAnim = input.magnitude > 0.1f && (!runPressed || _isCrouching || _isProning);

        _animator.SetBool("IsWalking", walkingAnim);
        _animator.SetBool("IsRunning", !_isCrouching && !_isProning && runPressed && input.magnitude > 0.1f && !movingBackward);
        _animator.SetBool("IsWalkingBackwards", movingBackward);
    }

    private void UpdateFalling()
    {
        bool grounded = IsGrounded();

        if (grounded)
        {
            _isFalling = false;
            _animator.SetBool("IsFalling", false);
            _fallTimer = 0f;
            _jumpInputTimer = 0f;
            _jumpInputQueued = false;
            _jumpPending = false;
            return;
        }

        _fallTimer += Time.deltaTime;

        bool currentlyFalling = _fallTimer > fallGraceTime && _velocity.y <= fallingVelocityThreshold;

        if (currentlyFalling && !_isFalling && !_isFlipping && !_isGliding)
        {
            _isFalling = true;
            _animator.SetBool("IsFalling", true);
            _isCrouching = false;
            _animator.SetBool("IsCrouching", false);
        }
    }

    public void Jump()
    {
        if (_isCrouching) return;

        _velocity.y = 0;
        _velocity.y = jumpForce;
        _animator.SetBool("IsJumping", true);
        _jumpPending = false;
        _jumpInputQueued = false;
    }

    public void Jump(float customJumpForce)
    {
        if (_isCrouching) return;

        _velocity.y = 0;
        _velocity.y = customJumpForce;
        _animator.SetBool("IsJumping", true);
        _jumpPending = false;
        _jumpInputQueued = false;
    }

    public void BeginFlip()
    {
        if (_isCrouching) return;

        _isFlipping = true;
        _animator.SetBool("IsFlipping", true);
        _animator.SetBool("IsFalling", true);
        _jumpInputQueued = false;
        Jump(flipForce);
    }

    public void EndFlip()
    {
        _isFlipping = false;
        _animator.SetBool("IsFlipping", false);
        _animator.SetBool("IsJumping", false);
    }

    private bool CanStandUp()
    {
        float checkDistance = (standHeight - crouchHeight) - ceilingCheckOffset;
        Vector3 origin = transform.position + Vector3.up * crouchHeight;
        float radius = _controller.radius * 0.95f;

        return !Physics.SphereCast(origin, radius, Vector3.up, out _, checkDistance);
    }

    private bool CanCrouchUp()
    {
        float checkDistance = (crouchHeight - proneHeight) - ceilingCheckOffset;
        Vector3 origin = transform.position + Vector3.up * proneHeight;
        float radius = _controller.radius * 0.95f;

        return !Physics.SphereCast(origin, radius, Vector3.up, out _, checkDistance);
    }

    private void StartRoll()
    {
        if (!IsGrounded() || _isSliding || _isCrouching || _isProning) return;

        Vector2 input = InputManager.Instance.MoveInput;
        Vector3 camForward = _mainCamera.forward; camForward.y = 0f;
        Vector3 camRight   = _mainCamera.right;   camRight.y = 0f;
        Vector3 moveDir    = (camForward * input.y + camRight * input.x).normalized;
        
        if (moveDir.sqrMagnitude < 0.1f)
            moveDir = transform.forward;

        _rollDirection = moveDir;

        _isRolling = true;
        _rollTimer = 0f;

        if(InputManager.Instance.IsRunning)
        {
            rollSpeed = defaultRollSpeed * rollRunSpeedMultiplier;
        }
        else
        {
            rollSpeed = defaultRollSpeed;
        }
        _animator.SetBool("IsRolling", true);

        _controller.height = crouchHeight;
        _controller.center = new Vector3(0f, crouchCenterY, 0f);

        transform.rotation = Quaternion.LookRotation(_rollDirection);
    }

    private void UpdateRoll()
    {
        if(_animator.GetBool("IsJumping"))
        {
            FinishRoll();
            return;
        }

        _rollTimer += Time.deltaTime;

        Vector3 rollVelocity = _rollDirection * rollSpeed;

        rollVelocity.y = _velocity.y;
        if(rollSpeed > defaultRollSpeed){
            _velocity.y += gravity/ (rollGravityDivider * 1.35f) * Time.deltaTime;
        }
        else{
        _velocity.y += gravity/rollGravityDivider * Time.deltaTime;
        }

        _controller.Move(rollVelocity * Time.deltaTime);

        if (_rollTimer >= rollDuration)
        {
            FinishRoll();
        }
    }

    private void FinishRoll()
    {
        _isRolling = false;

        if (CanStandUp())
        {
            _controller.height = standHeight;
            _controller.center = new Vector3(0f, standCenterY, 0f);
            _isCrouching = false;
            _animator.SetBool("IsCrouching", false);
        }
        else
        {
            _controller.height = crouchHeight;
            _controller.center = new Vector3(0f, crouchCenterY, 0f);
            _isCrouching = true;
            _animator.SetBool("IsCrouching", true);
        }

        _animator.SetBool("IsRolling", false);
        
        if(!IsGrounded())
        {
            _animator.SetBool("IsFalling", true);
        }
    }

    private void EnterLedge(Transform ledgeRoot)
    {
        if (_isRolling || _isSliding || _isCrouching || _isProning || _isFlipping || _isFalling || _animator.GetBool("IsJumping"))
            return;

        _isOnLedge = true;
        _currentLedge = ledgeRoot;

        _ledgeForward = _currentLedge.forward;
        _ledgeInward  = -_currentLedge.right;

        Vector3 localToLedge = transform.position - _currentLedge.position;
        _ledgeT = Vector3.Dot(localToLedge, _ledgeForward);

        float yRot = _currentLedge.rotation.eulerAngles.y + 90f;
        Quaternion targetRot = Quaternion.Euler(0f, yRot, 0f);

        StartCoroutine(LerpToLedge(transform.position, targetRot));

        bool facingLeft = Vector3.Dot(transform.right, _ledgeForward) < 0f;
        _lastIdleLeft = facingLeft;
        _animator.SetBool("IsLedgeIdleLeft", facingLeft);
        _animator.SetBool("IsLedgeIdleRight", !facingLeft);
    }

    private void ExitLedge()
    {
        _isOnLedge = false;
        _currentLedge = null;

        _animator.SetBool("IsLedgeIdleLeft", false);
        _animator.SetBool("IsLedgeIdleRight", false);
        _animator.SetBool("IsLedgeWalkingLeft", false);
        _animator.SetBool("IsLedgeWalkingRight", false);

        _ledgeExitCooldown = true;
        Invoke(nameof(ResetLedgeCooldown), 0.2f);
    }

    private void HandleLedgeMovement()
    {
        Vector2 input = InputManager.Instance.MoveInput;
        float x = input.x;

        _ledgeT += x * ledgeSpeed * Time.deltaTime;

        Vector3 desiredPos = _currentLedge.position + _ledgeForward * _ledgeT + _ledgeInward * ledgeWallOffset;

        Vector3 delta = desiredPos - transform.position;
        _controller.Move(delta);

        bool isIdle = Mathf.Abs(input.x) < 0.1f;

        if (isIdle)
        {
            _animator.SetBool("IsLedgeIdleLeft", _lastIdleLeft);
            _animator.SetBool("IsLedgeIdleRight", !_lastIdleLeft);

            _animator.SetBool("IsLedgeWalkingLeft", false);
            _animator.SetBool("IsLedgeWalkingRight", false);
        }
        else
        {
            if (input.x < -0.1f)
            {
                _lastIdleLeft = true;
                _animator.SetBool("IsLedgeIdleLeft", false);
                _animator.SetBool("IsLedgeIdleRight", false);
                _animator.SetBool("IsLedgeWalkingLeft", true);
                _animator.SetBool("IsLedgeWalkingRight", false);
            }
            else if (input.x > 0.1f)
            {
                _lastIdleLeft = false;
                _animator.SetBool("IsLedgeIdleLeft", false);
                _animator.SetBool("IsLedgeIdleRight", false);
                _animator.SetBool("IsLedgeWalkingLeft", false);
                _animator.SetBool("IsLedgeWalkingRight", true);
            }
        }
    }

    private IEnumerator LerpToLedge(Vector3 targetPos, Quaternion targetRot)
    {
        float t = 0f;
        while (t < 1f)
        {
            transform.position = Vector3.Lerp(transform.position, targetPos, t);
            transform.rotation = Quaternion.Slerp(transform.rotation, targetRot, t);
            t += Time.deltaTime * ledgeAlignSpeed;
            yield return null;
        }

        transform.position = targetPos;
        transform.rotation = targetRot;
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other == null) return;

        if (other.CompareTag("Ladder"))
        {
            if (!_isLadderClimbing && !_isExitingLadder)
                _ladderCandidate = other.transform;
        }

        if (other.CompareTag("Ledge"))
        {
            if (!_isOnLedge && !_ledgeExitCooldown)
                EnterLedge(other.transform);
        }
    }

    private void OnTriggerStay(Collider other)
    {
        if (other == null) return;

        // Fallback: if OnTriggerEnter was missed, ensure candidate is set while inside trigger (Unreal doesn't have this workaround if needed)
        if (other.CompareTag("Ladder"))
        {
            if (!_isLadderClimbing && !_isExitingLadder)
            {
                if (_ladderCandidate == null)
                {
                    Debug.Log("[Player] OnTriggerStay set ladder candidate: " + other.name);
                    _ladderCandidate = other.transform;
                }
            }
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if (other == null) return;

        if (other.CompareTag("Ladder"))
        {
            if (_ladderCandidate != null && other.transform == _ladderCandidate)
                _ladderCandidate = null;

            if (_isLadderClimbing && _currentLadder != null && other.transform == _currentLadder)
            {
                Collider ladderCol = _currentLadder.GetComponent<Collider>();
                if (ladderCol != null)
                {
                    Vector3 playerCenterWorld = transform.position + (_controller != null ? _controller.center : Vector3.zero);
                    float playerTopY = playerCenterWorld.y + ((_controller != null ? _controller.height : 1f) * 0.5f);

                    // Closest point on ladder collider to the player's center
                    Vector3 closest = ladderCol.ClosestPoint(playerCenterWorld);
                    float ladderTopY = ladderCol.bounds.max.y;

                    const float topExitThreshold = 0.25f; // tune if needed
                    bool leftFromTop = (playerTopY >= ladderTopY - topExitThreshold) || (closest.y >= ladderTopY - topExitThreshold);

                    if (leftFromTop)
                        StartLadderTopExit();
                    else
                        ExitLadderAtBottom();
                }
                else
                {
                    float ladderTopY = _currentLadder.position.y;
                    float playerTopY = transform.position.y + 1.0f;
                    if (playerTopY >= ladderTopY - 0.2f) StartLadderTopExit();
                    else ExitLadderAtBottom();
                }
            }
        }

        if (_currentLedge != null && other.transform == _currentLedge)
        {
            ExitLedge();
        }
    }


    private void ResetLedgeCooldown()
    {
        _ledgeExitCooldown = false;
    }

    // #if UNITY_EDITOR
    // private void OnDrawGizmos()
    // {
    //     Gizmos.color = standUpCheckColor;

    //     float radius = 0.5f;
    //     if (_controller != null) radius = _controller.radius * 0.95f;

    //     Vector3 standOrigin = transform.position + Vector3.up * crouchHeight;
    //     float standCheckDistance = (standHeight - crouchHeight) - ceilingCheckOffset;
    //     Gizmos.DrawWireSphere(standOrigin, radius);
    //     Gizmos.DrawLine(standOrigin, standOrigin + Vector3.up * Mathf.Max(0.01f, standCheckDistance));

    //     Vector3 crouchOrigin = transform.position + Vector3.up * proneHeight;
    //     float crouchCheckDistance = (crouchHeight - proneHeight) - ceilingCheckOffset;
    //     Gizmos.DrawWireSphere(crouchOrigin, radius);
    //     Gizmos.DrawLine(crouchOrigin, crouchOrigin + Vector3.up * Mathf.Max(0.01f, crouchCheckDistance));

    //     // Ledge visualization — leaves your ledge math untouched, just helps debug
    //     if (_isOnLedge && _currentLedge != null)
    //     {
    //         Gizmos.color = Color.cyan;
    //         Gizmos.DrawLine(transform.position, transform.position + _ledgeForward);
    //         Gizmos.DrawLine(transform.position, transform.position + _ledgeInward);
    //         Vector3 desiredPos = _currentLedge.position + _ledgeForward * _ledgeT + _ledgeInward * ledgeWallOffset;
    //         Gizmos.DrawWireSphere(desiredPos, 0.05f);
    //     }
    // }
    // #endif
}