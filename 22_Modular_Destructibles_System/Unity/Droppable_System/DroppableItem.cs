using UnityEngine;
using System.Collections;
using UnityEngine.Events;

/// <summary>
/// Represents a single dropped item in the world.
/// Handles physics, collision, and pickup mechanics.
/// Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
/// </summary>
public class DroppableItem : MonoBehaviour, IDroppable
{
    [SerializeField] private DroppableItemData itemData;

    [Header("Scatter Settings (Optional)")]
    [SerializeField] private DroppableScatterSettings scatterSettings;

    [Header("Pop Settings (Optional)")]
    [SerializeField] private DroppablePopSettings popSettings;

    [Header("Float Settings (Optional)")]
    [SerializeField] private DroppableFloatSettings floatSettings;

    [Header("Event Hooks (Optional)")]
    [SerializeField] private UnityEvent onCollectedEventHook;

    [Header("Settle Settings")]
    [SerializeField] private float settledVelocityThreshold = 0.1f;
    [SerializeField] private float settleCheckTimeout = 5f;
    private Vector3 settlePosition;

    [Header("Example Type for UI and Logic")]
    public DropableType exampleType;
    public enum DropableType
    {
        GenericDrop,
        Coin,
        Gem,
        Key
    }

    private bool isManagedByPool = false;
    private bool hasLanded = false;
    private bool isTransitioningToFloat = false;
    private float floatTransitionProgress = 0f;
    private Vector3 spawnPosition;
    private Rigidbody rb;
    private Transform playerTransform;
    private Vector3 initialScale;
    private Coroutine collectibleCoroutine;
    private GameObject sourcePrefab;


    public void SetManagedByPool(bool value) { isManagedByPool = value; }
    public GameObject SourcePrefab => sourcePrefab;
    public void SetSourcePrefab(GameObject prefab){ sourcePrefab = prefab; }
    private int quantity;
    public int Quantity => quantity;
    public void SetQuantity(int amount) { quantity = amount; }
    public bool IsCollectible { get; private set; }
    public bool IsBeingCollected { get; private set; }
    public DroppableItemData ItemData => itemData;

    private void Awake()
    {
        rb = GetComponent<Rigidbody>();
        initialScale = transform.localScale;

        IsCollectible = false;
        IsBeingCollected = false;

        if (rb != null)
        {
            rb.useGravity = true;
            rb.isKinematic = false;
            rb.linearDamping = 1f;
            rb.angularDamping = 0.7f;
        }
    }

    private void Start()
    {
        if (DropManager.Instance == null)
            Debug.LogWarning("[DroppableItem] No DropManager in scene — pool spawning unavailable.");

        // Pre-placed path — ItemData assigned in Inspector, skip scatter/settle
        if (itemData != null && !isManagedByPool)
        {
            initialScale = transform.localScale;
            settlePosition = transform.position;
            //quantity = itemData.baseQuantity; // Base quantity
            quantity = Random.Range(itemData.minDropQuantity, itemData.maxDropQuantity + 1); // Randomized quantity within range
            MakeCollectible();
        }
    }

    private void Update()
    {
        if (IsBeingCollected && playerTransform != null)
        {
            MoveTowardsPlayer(playerTransform);
        }
        else if (IsCollectible && !IsBeingCollected)
        {
            UpdateFloatRotation();
            UpdateFloatPosition();
        }
    }

    /// <summary>
    /// Called by DropManager when item is spawned from pool.
    /// Handles initialization specific to each spawn.
    /// </summary>
    public void OnSpawn()
    {
        StopAllCoroutines(); // Ensure no leftover coroutines are running when respawning from pool
        transform.SetParent(null);

        hasLanded = false;
        IsCollectible = false;
        IsBeingCollected = false;
        isTransitioningToFloat = false;
        floatTransitionProgress = 0f;
        playerTransform = null;
        settlePosition = Vector3.zero;

        transform.localScale = initialScale;
        spawnPosition = transform.position;

        if (rb != null)
        {
            rb.isKinematic = false;
            rb.linearVelocity = Vector3.zero;
            rb.angularVelocity = Vector3.zero;
        }

        Debug.Log($"[DroppableItem] Spawned: {itemData.itemName} x{itemData.baseQuantity} at {transform.position}");

        if (collectibleCoroutine != null)
            StopCoroutine(collectibleCoroutine);

        ApplyScatterEffect();

        collectibleCoroutine = StartCoroutine(CollectibleDelayRoutine());
    }

    private void ApplyScatterEffect()
    {
        DroppableScatterSettings settings = scatterSettings != null ? scatterSettings : DroppableScatterSettings.GetOrDefault();

        if (!settings.enabled || rb == null)
            return;

        Vector3 randomDirection = Random.insideUnitSphere.normalized;
        float randomForce = Random.Range(settings.forceMin, settings.forceMax);

        Vector3 scatterForce = randomDirection * randomForce;
        scatterForce.y = Mathf.Abs(scatterForce.y) + settings.upForce;

        rb.linearVelocity = scatterForce;
        rb.angularVelocity = Random.insideUnitSphere * (itemData.floatRotationSpeed * Mathf.Deg2Rad * settings.spinForceMultiplier);

        Debug.Log($"[DroppableItem] Applied scatter force: {scatterForce}");

    }

    private IEnumerator CollectibleDelayRoutine()
    {
        yield return new WaitForSeconds(itemData.collectDelay);

        // Wait until the rigidbody has settled AND item has landed
        if (rb != null && !rb.isKinematic)
        {
            float elapsed = 0f;
            while (elapsed < settleCheckTimeout)
            {
                if (rb.linearVelocity.magnitude <= settledVelocityThreshold && hasLanded)
                    break;

                elapsed += Time.deltaTime;
                yield return null;
            }
        }

        MakeCollectible();
        collectibleCoroutine = null;
    }

    private void OnCollisionEnter(Collision collision)
    {
        if (hasLanded || IsCollectible) return;

        foreach (ContactPoint contact in collision.contacts)
        {
            if (contact.normal.y > 0.5f)
            {
                hasLanded = true;

                settlePosition = transform.position;

                break;
            }
        }
    }

    private void OnTriggerEnter(Collider collision)
    {
        if (!IsCollectible || IsBeingCollected)
            return;

        if (collision.CompareTag("Player"))
        {
            Debug.Log($"[DroppableItem] Player entered trigger for {itemData.itemName}");
            StartCollecting(collision.transform);
        }
    }

    public void MakeCollectible()
    {
        DroppableFloatSettings settings = floatSettings != null? floatSettings : DroppableFloatSettings.GetOrDefault();

        IsCollectible = true;

        if (rb != null)
        {
            rb.linearVelocity = Vector3.zero;
            rb.angularVelocity = Vector3.zero;
            rb.Sleep();
            rb.isKinematic = true;
        }

        if (settings.enabled)
            StartCoroutine(FloatTransitionRoutine());

        Debug.Log($"[DroppableItem] {itemData.itemName} is now collectible!");
    }

    private IEnumerator FloatTransitionRoutine()
    {
        DroppableFloatSettings settings = floatSettings != null ? floatSettings : DroppableFloatSettings.GetOrDefault();

        isTransitioningToFloat = true;
        floatTransitionProgress = 0f;

        yield return new WaitForFixedUpdate();

        float startY = transform.position.y;
        float targetY = settlePosition.y + settings.groundOffset;

        while (floatTransitionProgress < 1f)
        {
            floatTransitionProgress += Time.deltaTime / settings.transitionDuration;
            float eased = Mathf.SmoothStep(0f, 1f, floatTransitionProgress);

            float newY = Mathf.Lerp(startY, targetY, eased);

            transform.position = new Vector3(
                transform.position.x,
                newY,
                transform.position.z
            );

            yield return null;
        }

        spawnPosition = new Vector3(transform.position.x, targetY, transform.position.z);
        isTransitioningToFloat = false;
    }

    public void StartCollecting(Transform player)
    {
        IsBeingCollected = true;
        playerTransform = player;

        if (rb != null)
            rb.isKinematic = true;

        StartCoroutine(PopEffectRoutine());

        Debug.Log($"[DroppableItem] Starting collection of {itemData.itemName}");
    }

    private IEnumerator PopEffectRoutine()
    {
        DroppablePopSettings settings = popSettings != null ? popSettings : DroppablePopSettings.GetOrDefault();

        if (!settings.enabled)
            yield break;

        float elapsed = 0f;
        Vector3 targetScale = initialScale * settings.scale;

        while (elapsed < settings.duration)
        {
            elapsed += Time.deltaTime;
            transform.localScale = Vector3.Lerp(initialScale, targetScale, elapsed / settings.duration);
            yield return null;
        }

        elapsed = 0f;
        while (elapsed < settings.duration)
        {
            elapsed += Time.deltaTime;
            transform.localScale = Vector3.Lerp(targetScale, initialScale, elapsed / settings.duration);
            yield return null;
        }

        transform.localScale = initialScale;
    }

    private void UpdateFloatPosition()
    {
        if (isTransitioningToFloat) return;

        DroppableFloatSettings settings = floatSettings != null ? floatSettings : DroppableFloatSettings.GetOrDefault();

        if (!settings.enabled) return;

        float bobOffset = Mathf.Sin(Time.time * settings.bobSpeed) * settings.bobHeight / 2f;
        transform.position = new Vector3(
            transform.position.x,
            spawnPosition.y + bobOffset,
            transform.position.z
        );
    }

    private void UpdateFloatRotation()
    {
        DroppableFloatSettings settings = floatSettings != null ? floatSettings : DroppableFloatSettings.GetOrDefault();

        if (!settings.enabled) return;

        transform.RotateAround(transform.position, Vector3.up, settings.rotationSpeed * Time.deltaTime);
    }

    public void MoveTowardsPlayer(Transform playerTransform)
    {
        if (playerTransform == null)
            return;

        // I added a small upward offset to the target position to make it look nicer when items float up towards the player instead of going straight to their feet.
        // I recommend removing or adjusting this offset if your player pivot is at a different height or if you want a different visual effect.
        Vector3 targetPosition = playerTransform.position + Vector3.up * 1.0f;

        Vector3 direction = (targetPosition - transform.position).normalized;

        transform.position += direction * itemData.collectSpeed * Time.deltaTime;

        if (Vector3.Distance(transform.position, targetPosition) < 0.5f)
            OnCollected();
    }

    public void OnCollected()
    {
        Debug.Log($"[DroppableItem] Collected: {itemData.itemName} x{quantity}");

        onCollectedEventHook?.Invoke();

        // EXPANSION POINT: Hook into your own inventory, quest, or progression system here.
        // Example: InventoryManager.Instance.AddItem(itemData, quantity);

        #region Example UI Logic — Remove or replace with your own collection logic
        // This switch exists purely to demonstrate collection working with a simple UI.
        // Delete this entire region and use onCollectedEventHook (If possible) or your own system instead.
        switch (exampleType)
        {
            case DropableType.Coin:
                UIManager.Instance.AddCoin(quantity);
                break;
            case DropableType.Gem:
                UIManager.Instance.AddGem(quantity);
                break;
            case DropableType.Key:
                UIManager.Instance.AddKey(quantity);
                break;
            default:
                Debug.Log($"[DroppableItem] Collected {itemData.itemName} x{quantity}");
                break;
        }
        #endregion

        if (isManagedByPool)
            DropManager.Instance.OnDropCollected(this);
        else
            Destroy(gameObject);
    }

    public void SetItemData(DroppableItemData data)
    {
        itemData = data;
    }
}