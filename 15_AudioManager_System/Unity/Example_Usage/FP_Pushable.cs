using UnityEngine;

[RequireComponent(typeof(Rigidbody))]
public class FP_Pushable : MonoBehaviour, IFP_Interactable
{
    [Header("Interaction Cooldown")]
    [SerializeField] private float interactCooldown = 0.5f;
    private float lastInteractTime = -999f;

    [Header("Pushable Settings")]
    [SerializeField] private float pushForce = 8f;
    [SerializeField] private float upwardBoost = 1f;

    [Header("Push Indicator (Optional)")]
    [SerializeField] private GameObject pushIndicatorPrefab;

    [Header("Randomization Settings")]
    [SerializeField] private float horizontalRandomPercent = 0.1f;
    [SerializeField] private float verticalRandomPercent = 0.05f;
    [SerializeField] private float torqueRandomPercent = 0.5f;

    [Header("Audio")]
    [SerializeField] private AudioClip pushSound;


    private Rigidbody rb;

    private void Awake()
    {
        rb = GetComponent<Rigidbody>();
    }

    public void OnInteract(Vector3 interactionDirection)
    {
        if (rb == null) return;

        // Cooldown
        if (Time.time < lastInteractTime + interactCooldown)
            return;

        lastInteractTime = Time.time;

        // Prevent stacking forces by resetting velocity
        rb.linearVelocity = Vector3.zero;
        rb.angularVelocity = Vector3.zero;

        Vector3 dir = interactionDirection.normalized;

        dir.y += upwardBoost;

        // Randomness scaled by upwardBoost
        float horizRand = upwardBoost * horizontalRandomPercent;
        float vertRand = upwardBoost * verticalRandomPercent;
        float torqueRand = upwardBoost * torqueRandomPercent;

        Vector3 randomHorizontal = new Vector3(Random.Range(-horizRand, horizRand),0f,Random.Range(-horizRand, horizRand));

        float randomVertical = Random.Range(-vertRand, vertRand);

        dir += randomHorizontal;
        dir.y += randomVertical;

        dir = dir.normalized;

        // Push force
        rb.AddForce(dir * pushForce, ForceMode.Impulse);

        // Torque
        Vector3 randomTorque = Random.insideUnitSphere * torqueRand;
        rb.AddTorque(randomTorque, ForceMode.Impulse);

        if (pushSound != null)
            AudioManager.Instance.PlayAtPosition(pushSound, transform.position);
    }


    public void OnFocusEnter()
    {
        if(pushIndicatorPrefab != null)
            pushIndicatorPrefab.SetActive(true);
    }

    public void OnFocusExit()
    {
        if(pushIndicatorPrefab != null)
        pushIndicatorPrefab.SetActive(false);
    }
}
