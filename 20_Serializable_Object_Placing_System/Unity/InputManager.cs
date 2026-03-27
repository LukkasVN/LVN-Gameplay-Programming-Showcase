using UnityEngine;
using UnityEngine.InputSystem;

public class InputManager : MonoBehaviour
{
    public static InputManager Instance { get; private set; }

    private PlayerInput _playerInput;
    private InputAction _attack;
    private InputAction _secondary;
    private InputAction _move;
    private InputAction _look;
    private InputAction _run;
    private InputAction _dance;
    private InputAction _jump;
    private InputAction _crouch;
    private InputAction _prone;
    private InputAction _roll;
    private InputAction _glide;
    private InputAction _climb;
    private InputAction _interact;
    private InputAction _tab;
    private InputAction _flashlight;
    private InputAction _rotateObject;

    public Vector2 MoveInput { get; private set; }
    public bool IsAttacking { get; private set; }
    public bool IsSecondary { get; private set; }
    public Vector2 LookInput { get; private set; }
    public bool isInteracting { get; private set; }
    public bool IsRunning { get; private set; }
    public bool IsDancing { get; private set; }
    public bool IsJumping { get; private set; }
    public bool IsCrouching { get; private set; }
    public bool CrouchButtonPressed { get; private set; }
    public bool IsProning { get; private set; }
    public bool IsRolling { get; private set; }
    public bool IsGliding { get; private set; }
    public bool IsClimbing { get; private set; }
    public bool IsTabbing { get; private set; }
    public bool IsFlashlightOn { get; private set; }
    public bool IsRotatingObject { get; private set; }
    private void Awake()
    {
        if (Instance != null && Instance != this)
        {
            Destroy(gameObject);
            return;
        }
        Instance = this;

        _playerInput = GetComponent<PlayerInput>();

        _move = _playerInput.actions["Move"];
        _attack = _playerInput.actions["Attack"];
        _secondary = _playerInput.actions["Secondary"];
        _look = _playerInput.actions["Look"];
        _run = _playerInput.actions["Run"];
        _dance = _playerInput.actions["Dance"];
        _jump = _playerInput.actions["Jump"];
        _crouch = _playerInput.actions["Crouch"];
        _prone = _playerInput.actions["Prone"];
        _roll = _playerInput.actions["Roll"];
        _glide = _playerInput.actions["Glide"];
        _climb = _playerInput.actions["Climb"];
        _interact = _playerInput.actions["Interact"];
        _tab = _playerInput.actions["Tab"];
        _flashlight = _playerInput.actions["Flashlight"];
        _rotateObject = _playerInput.actions["Rotate"];
    }

    private void Update()
    {
        MoveInput = _move.ReadValue<Vector2>();
        IsAttacking = _attack.WasPressedThisFrame();
        IsSecondary = _secondary.WasPressedThisFrame();
        LookInput = _look.ReadValue<Vector2>();
        IsRunning = _run.IsPressed();
        IsDancing = _dance.WasPressedThisFrame();
        IsJumping = _jump.WasPressedThisFrame();
        IsCrouching = _crouch.WasPressedThisFrame();
        CrouchButtonPressed = _crouch.IsPressed();
        IsProning = _prone.WasPressedThisFrame();
        IsRolling = _roll.WasPressedThisFrame();
        IsGliding = _glide.IsPressed();
        IsClimbing = _climb.WasPressedThisFrame();
        isInteracting = _interact.WasPressedThisFrame();
        IsTabbing = _tab.WasPressedThisFrame();
        IsFlashlightOn = _flashlight.WasPressedThisFrame();
        IsRotatingObject = _rotateObject.WasPressedThisFrame();
    }
}
