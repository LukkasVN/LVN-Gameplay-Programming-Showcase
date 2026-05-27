using System;
using System.Collections;
using UnityEngine;
using UnityEngine.Events;

/*
    Attach to any GameObject to make it destructible via FP_Controller's hit system.
    It is build so you can attach or call TakeDamage() from any script or event system.
    Driven by a DestructibleSO. Only external dependency is UIManager for hover feedback.

    - Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
*/

public class Destructible : MonoBehaviour
{
    [Header("Data")]
    [SerializeField] private DestructibleSO data;

    [Header("Events")]
    [Tooltip("Inspector hook. Fires on each successful hit.")]
    public UnityEvent onHit;
    [Tooltip("Inspector hook. Fires just before destruction.")]
    public UnityEvent onDestroyed;

    [Header("Destroy Settings")]
    [SerializeField] private bool hasDestroyedPrefab = true;
    [SerializeField] private Vector3 destroyedPrefabOffset;
    [Range(0f, 5f)]
    [SerializeField] private float destroyDelay = 0.1f;

    [Header("Respawn")]
    [Tooltip("If true, destruction deactivates this GameObject's visuals/colliders " +
         "instead of destroying it, so RespawnDestructible() can bring it back. " +
         "If false, the GameObject is destroyed normally.")]
    [SerializeField] private bool isRespawnable = false;
    [SerializeField] private float respawnDelay = 5f; // Set to auto-respawn after this many seconds. Replace or remove as needed. Example purpose only.
    private GameObject destroyedInstance; // reference to the spawned destroyed prefab instance, if any, so we can manage it on respawn.

    [Header("Example Particles [Recommended Removal]")]
    [SerializeField] private GameObject hitParticlesPrefab;
    [SerializeField] private GameObject destroyParticlesPrefab;

    // Code-side events with spatial data
    public event Action<Vector3, Vector3> OnHitWithData;
    public event Action<Vector3> OnDestroyedWithData;

    public int CurrentHP => currentHP;
    public int MaxHP => data != null ? data.hitPoints : 0;
    public string DestructibleName => data != null ? data.destructibleName : string.Empty;

    private int currentHP;
    private Vector3 originalLocalPosition;
    private Vector3 originalScale;

    // Pop and shake run independently so they layer cleanly on rapid hits.
    private Coroutine popCoroutine;
    private Coroutine shakeCoroutine;
    private Collider[] cachedColliders;
    private Renderer[] cachedRenderers;

    // Examples of subscribing to the typed events for spatial feedback. Replace with your own VFX/audio system.
    // Although you can also achieve this logic by adding the logic inside TakeDamage(), HandleDestruction(), or via inspector UnityEvents, using typed events with data allows for better separation of concerns and reusability.
    #region Particle Example Methods [Recommended Removal]
    void OnEnable()
        {
            OnHitWithData += ExampleHitParticles; // Example subscription to the hit event with spatial data. Replace or remove as needed.
            OnDestroyedWithData += ExampleDestroyParticles; // Example subscription to the destroy event with spatial data. Replace or remove as needed.
        }

        public void ExampleHitParticles(Vector3 hitPosition, Vector3 hitDirection)
        {
            if (hitParticlesPrefab != null)
                Instantiate(hitParticlesPrefab, hitPosition, Quaternion.LookRotation(-hitDirection));
        }
        
        public void ExampleDestroyParticles(Vector3 position)
        {
            if (destroyParticlesPrefab != null)
                Instantiate(destroyParticlesPrefab, position, destroyParticlesPrefab.transform.rotation);
        }
    #endregion

    private void Start()
    {
        if (data == null)
        {
            Debug.LogError($"[Destructible] No DestructibleSO assigned on {gameObject.name}!", gameObject);
            enabled = false;
            return;
        }

        currentHP = data.hitPoints;
        originalLocalPosition = transform.localPosition;
        originalScale = transform.localScale;

        if (isRespawnable)
        {
            cachedColliders = GetComponentsInChildren<Collider>(includeInactive: true);
            cachedRenderers = GetComponentsInChildren<Renderer>(includeInactive: true);
        }
    }

    public void TakeDamage(int damage, int tierLevel, Vector3 hitPosition, Vector3 hitDirection)
    {
        if (currentHP <= 0) return;

        if (tierLevel < data.tierRequired)
        {
            Debug.Log($"[Destructible] Tool tier too low ({tierLevel} < {data.tierRequired}) for '{data.destructibleName}'.");
            return;
        }

        currentHP = Mathf.Max(currentHP - damage, 0);

        if (data.useHitPopEffect)
            TriggerHitFeedback(hitDirection);

        onHit?.Invoke();
        OnHitWithData?.Invoke(hitPosition, hitDirection);

        UIManager.Instance.UpdateDestructibleHover(currentHP, data.hitPoints);

        if (currentHP <= 0)
            HandleDestruction(hitPosition, hitDirection);
    }

    public void OnFocusEnter() => UIManager.Instance.ShowDestructibleHover(data.destructibleName, currentHP, data.hitPoints);
    public void OnFocusExit() => UIManager.Instance.HideDestructibleHover();

    private void TriggerHitFeedback(Vector3 hitDirection)
    {
        if (popCoroutine != null) StopCoroutine(popCoroutine);
        if (shakeCoroutine != null) StopCoroutine(shakeCoroutine);

        popCoroutine = StartCoroutine(PopEffectCoroutine());
        shakeCoroutine = StartCoroutine(ShakeEffectCoroutine(hitDirection));
    }

    private IEnumerator PopEffectCoroutine()
    {
        float half = data.popEffectDuration * 0.5f;
        Vector3 bigScale = originalScale * (1f + data.popEffectIntensity);

        float elapsed = 0f;
        while (elapsed < half)
        {
            elapsed += Time.deltaTime;
            transform.localScale = Vector3.Lerp(originalScale, bigScale, elapsed / half);
            yield return null;
        }

        elapsed = 0f;
        while (elapsed < half)
        {
            elapsed += Time.deltaTime;
            transform.localScale = Vector3.Lerp(bigScale, originalScale, elapsed / half);
            yield return null;
        }

        transform.localScale = originalScale;
        popCoroutine = null;
    }

    private IEnumerator ShakeEffectCoroutine(Vector3 hitDirection)
    {
        float elapsed = 0f;
        float duration = data.popEffectDuration * 2f;
        float magnitude = data.popEffectIntensity * 0.4f;

        // Vertical shake reads as a glitch — flatten to horizontal only.
        Vector3 shakeAxis = new Vector3(hitDirection.x, 0f, hitDirection.z).normalized;
        if (shakeAxis == Vector3.zero)
            shakeAxis = transform.right;

        while (elapsed < duration)
        {
            elapsed += Time.deltaTime;
            float decay = 1f - (elapsed / duration);
            float offset = Mathf.Sin(elapsed * 40f) * magnitude * decay;
            transform.localPosition = originalLocalPosition + shakeAxis * offset;
            yield return null;
        }

        transform.localPosition = originalLocalPosition;
        shakeCoroutine = null;
    }

    /*
        Destruction order:
        1. Hide UI and fire events first so subscribers can still read state.
        2. Spawn the destroyed prefab (if any).
        3. Either destroy the GameObject (with a small delay so coroutines finish)
        or deactivate visuals/colliders for later RespawnDestructible() calls.
    */
    private void HandleDestruction(Vector3 hitPosition, Vector3 hitDirection)
    {
        UIManager.Instance.HideDestructibleHover();

        onDestroyed?.Invoke();
        OnDestroyedWithData?.Invoke(transform.position);

        if (hasDestroyedPrefab && data.destroyedPrefab != null)
        {
            destroyedInstance = Instantiate(data.destroyedPrefab, transform.position + destroyedPrefabOffset, transform.rotation);
            destroyedInstance.transform.localScale = transform.localScale;
        }

        if (isRespawnable){
            DeactivateForRespawn();
            Invoke(nameof(RespawnDestructible), respawnDelay); // Example respawn after 5 seconds. Replace or remove as needed.
        }
        else{
            Destroy(gameObject, destroyDelay);
        }
    }

    private void DeactivateForRespawn()
    {
        foreach (var col in cachedColliders) col.enabled = false;
        foreach (var rend in cachedRenderers) rend.enabled = false;
    }

    /// <summary>
    /// Bring a respawnable destructible back. Restores HP, re-enables visuals and colliders,
    /// and resets the transform.
    /// Ideal usage is to call it from another script or system so respawn logic is decoupled from the destructible itself, but you can also call it from within this script if you want.
    /// </summary>
    public void RespawnDestructible()
    {
        if (!isRespawnable || currentHP > 0) return;

        if (destroyedInstance != null)
        {
            Destroy(destroyedInstance);
            destroyedInstance = null;
        }

        currentHP = data.hitPoints;
        transform.localPosition = originalLocalPosition;
        transform.localScale = originalScale;

        foreach (var col in cachedColliders) col.enabled = true;
        foreach (var rend in cachedRenderers) rend.enabled = true;
    }

    public void RestoreHP()
    {
        currentHP = data.hitPoints;
    }

    public void SetHP(int value)
    {
        currentHP = Mathf.Clamp(value, 0, data.hitPoints);
    }

    public void ForceDestroy()
    {
        if (currentHP <= 0) return;
        currentHP = 0;
        HandleDestruction(transform.position, Vector3.up);
    }

    private void OnDrawGizmosSelected()
    {
        if (data == null) return;
        Gizmos.color = new Color(1f, 0.3f, 0.3f, 0.35f);
        Gizmos.DrawWireCube(transform.position, transform.localScale);
    }
}