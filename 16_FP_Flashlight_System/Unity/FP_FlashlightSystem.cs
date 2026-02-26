using System.Collections;
using TMPro;
using UnityEngine;

public class FP_FlashlightSystem : MonoBehaviour
{
    [Header("References")]
    [SerializeField] private Transform flashlightPivot;
    [SerializeField] private Light flashlightLight;
    [SerializeField] private Transform cameraTransform;

    [Header("Battery")]
    [SerializeField] private float maxBattery = 100f;
    [SerializeField] private float batteryLifeTimeSeconds = 30f;
    [SerializeField] private float chargeTime = 1f;
    private float currentFlashlightBattery;

    [Header("Flicker Settings")]
    [SerializeField] private float lowBatteryThreshold = 25f;
    [SerializeField] private float flickerIntervalMin = 0.5f;
    [SerializeField] private float flickerIntervalMax = 1f;
    [SerializeField] private float flickerOffDuration = 0.5f;
    private Coroutine flickerCoroutine;

    [Header("Inertia Settings")]
    [SerializeField] private float inertiaStrength = 25f;
    [SerializeField] private float inertiaDamping = 10f;
    private Quaternion inertiaRotation;
    private Vector3 angularVelocity;

    [Header("Debug")]
    [SerializeField] private TextMeshProUGUI debugBatteryText;
    private Coroutine batteryChargeCoroutine;

    private bool flashlightState = false;

    private void Start()
    {
        inertiaRotation = flashlightPivot.rotation;
        currentFlashlightBattery = maxBattery;

        if (flashlightLight != null)
            flashlightLight.enabled = flashlightState;
    }

    private void LateUpdate()
    {
        if (flashlightState)
        {
            UpdateFlashlight();
            HandleDebugText(); // Optional Debug Update
        }
    }

    public void ToggleFlashlight()
    {
        if (flashlightLight == null || debugBatteryText == null || cameraTransform == null || flashlightPivot == null)
        {
            Debug.Log("Flashlight references not set up properly!");
            return;
        }

        if (currentFlashlightBattery <= 0f)
        {
            currentFlashlightBattery = 0f;
            flashlightState = false;
            flashlightLight.enabled = false;
            Debug.Log("Flashlight battery depleted!");
            return;
        }

        flashlightState = !flashlightState;
        flashlightLight.enabled = flashlightState;

        // Snap instantly when turning ON
        if (flashlightState)
        {
            inertiaRotation = cameraTransform.rotation;
            flashlightPivot.rotation = inertiaRotation;
            angularVelocity = Vector3.zero;
        }
        else
        {
            // Stop flicker when turning OFF
            if (flickerCoroutine != null)
            {
                StopCoroutine(flickerCoroutine);
                flickerCoroutine = null;
            }
        }

        Debug.Log("Flashlight toggled: " + flashlightState);
    }

    private void UpdateFlashlight()
    {
        Quaternion targetRot = cameraTransform.rotation;

        Quaternion delta = targetRot * Quaternion.Inverse(inertiaRotation);
        delta.ToAngleAxis(out float angle, out Vector3 axis);

        if (angle > 180f) angle -= 360f;

        Vector3 torque = axis * angle * inertiaStrength;
        angularVelocity += torque * Time.deltaTime;

        angularVelocity *= Mathf.Exp(-inertiaDamping * Time.deltaTime);

        inertiaRotation = Quaternion.Euler(angularVelocity * Time.deltaTime) * inertiaRotation;

        flashlightPivot.rotation = inertiaRotation;

        HandleBatteryDrain();
    }

    private void HandleBatteryDrain()
    {
        if (currentFlashlightBattery <= 0f)
        {
            currentFlashlightBattery = 0f;
            ToggleFlashlight();
            return;
        }

        currentFlashlightBattery -= (maxBattery / batteryLifeTimeSeconds) * Time.deltaTime;
        currentFlashlightBattery = Mathf.Clamp(currentFlashlightBattery, 0f, maxBattery);

        // Start flicker when low
        if (currentFlashlightBattery <= lowBatteryThreshold && flickerCoroutine == null)
        {
            flickerCoroutine = StartCoroutine(FlickerRoutine());
        }
        // Stop flicker when battery rises above threshold
        else if (currentFlashlightBattery > lowBatteryThreshold && flickerCoroutine != null)
        {
            StopCoroutine(flickerCoroutine);
            flickerCoroutine = null;
            flashlightLight.enabled = true;
        }
    }

    private IEnumerator FlickerRoutine()
    {
        while (flashlightState && currentFlashlightBattery > 0f)
        {
            float wait = Random.Range(flickerIntervalMin, flickerIntervalMax);
            yield return new WaitForSeconds(wait);

            flashlightLight.enabled = false;
            yield return new WaitForSeconds(flickerOffDuration);

            if (flashlightState)
                flashlightLight.enabled = true;
        }

        flickerCoroutine = null;
    }

    private void HandleDebugText() // Optional Debug Update
    {
        if (debugBatteryText != null)
            debugBatteryText.text = $"Flashlight battery: {currentFlashlightBattery:F0}%";
    }

    public void RechargeBattery(float amount)
    {
        currentFlashlightBattery += amount;
        currentFlashlightBattery = Mathf.Clamp(currentFlashlightBattery, 0f, maxBattery);

        HandleDebugText(); // Optional Debug Update

        Debug.Log($"Flashlight recharged: {currentFlashlightBattery:F0}%");
    }

    public void StartBatteryGradualRecharge(float amount)
    {
        if (batteryChargeCoroutine != null)
            StopCoroutine(batteryChargeCoroutine);

        batteryChargeCoroutine = StartCoroutine(ChargeBatteryOverTime(amount, chargeTime));
    }

    private IEnumerator ChargeBatteryOverTime(float amount, float duration)
    {
        float elapsed = 0f;
        while (elapsed < duration)
        {
            RechargeBattery((amount / duration) * Time.deltaTime);
            elapsed += Time.deltaTime;
            yield return null;
        }
    }
}
