using UnityEngine;

public class ThirdPersonCamera : MonoBehaviour
{
    [Header("References")]
    public Transform target;

    [Header("Orbit Settings")]
    public float distance = 5f;
    public float minPitch = -30f;
    public float maxPitch = 60f;
    public float rotationSpeed = 150f;
    public float smoothTime = 0.15f;

    [Header("Vertical Smoothing")]
    public float targetHeightOffset = 1.6f;

    [Header("Sensitivity Settings")]
    [Range(0.1f, 5f)]
    public float sensitivityMultiplier = 0.75f;
    [Range(0.1f, 5f)]
    public float verticalSensitivityMultiplier = 0.75f;

    private float _yaw;
    private float _pitch;

    private Vector3 _currentVelocity;
    private float _currentTargetHeightVelocity;
    private float _targetHeight;
    private Vector3 _smoothedTargetPosition;
    private Vector3 _smoothedTargetVelocity;

    private void Start()
    {
        if (!target) return;

        Vector3 angles = transform.eulerAngles;
        _yaw = angles.y;
        _pitch = angles.x;

        _smoothedTargetPosition = target.position;
        _targetHeight = target.position.y + targetHeightOffset;

        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;
    }

    private void LateUpdate()
    {
        if (!target) return;

        Vector2 look = InputManager.Instance.LookInput;
        _yaw += look.x * rotationSpeed * sensitivityMultiplier * Time.deltaTime;
        _pitch -= look.y * rotationSpeed * verticalSensitivityMultiplier * Time.deltaTime;
        _pitch = Mathf.Clamp(_pitch, minPitch, maxPitch);

        _smoothedTargetPosition = Vector3.SmoothDamp(_smoothedTargetPosition, target.position, ref _smoothedTargetVelocity, smoothTime);

        _targetHeight = Mathf.SmoothDamp(_targetHeight, target.position.y + targetHeightOffset, ref _currentTargetHeightVelocity, smoothTime);

        Vector3 targetPos = new Vector3(_smoothedTargetPosition.x, _targetHeight, _smoothedTargetPosition.z);

        Quaternion rotation = Quaternion.Euler(_pitch, _yaw, 0f);
        Vector3 desiredPosition = targetPos - rotation * Vector3.forward * distance;

        transform.position = Vector3.SmoothDamp(transform.position, desiredPosition, ref _currentVelocity, smoothTime);
        transform.LookAt(targetPos);
    }
}
