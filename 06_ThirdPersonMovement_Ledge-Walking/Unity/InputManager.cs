using UnityEngine;
using UnityEngine.InputSystem;

public class InputManager : MonoBehaviour
{
    public static InputManager Instance { get; private set; }

    private PlayerInput _playerInput;
    private InputAction _move;
    private InputAction _look;
    private InputAction _run;
    private InputAction _dance;
    private InputAction _jump;
    private InputAction _crouch;
    private InputAction _prone;
    private InputAction _roll;

    public Vector2 MoveInput { get; private set; }
    public Vector2 LookInput { get; private set; }
    public bool IsRunning { get; private set; }
    public bool IsDancing { get; private set; }
    public bool IsJumping { get; private set; }
    public bool IsCrouching { get; private set; }
    public bool CrouchButtonPressed { get; private set; }
    public bool IsProning { get; private set; }
    public bool IsRolling { get; private set; }


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
        _look = _playerInput.actions["Look"];
        _run = _playerInput.actions["Run"];
        _dance = _playerInput.actions["Dance"];
        _jump = _playerInput.actions["Jump"];
        _crouch = _playerInput.actions["Crouch"];
        _prone = _playerInput.actions["Prone"];
        _roll = _playerInput.actions["Roll"];
    }

    private void Update()
    {
        MoveInput = _move.ReadValue<Vector2>();
        LookInput = _look.ReadValue<Vector2>();
        IsRunning = _run.IsPressed();
        IsDancing = _dance.WasPressedThisFrame();
        IsJumping = _jump.WasPressedThisFrame();
        IsCrouching = _crouch.WasPressedThisFrame();
        CrouchButtonPressed = _crouch.IsPressed();
        IsProning = _prone.WasPressedThisFrame();
        IsRolling = _roll.WasPressedThisFrame();
    }
}
