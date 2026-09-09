using System.Collections;
using UnityEngine;
using UnityEngine.Events;

[DisallowMultipleComponent]
public class Target : MonoBehaviour, IHittable
{
    [System.Serializable] public class TargetHitEvent : UnityEvent<int, Vector3> { }

    private enum PopPhase { Idle, Up, Down, Return }

    [Header("Durability")]
    [SerializeField] private int hitsToDestroy = 5;
    [SerializeField] private float destroyDelay = 0.05f;

    [Header("Pop")]
    [SerializeField] private float popUpScale = 1.6f;
    [SerializeField] private float popUpDuration = 0.06f;
    [SerializeField] private float popDownScale = 0.85f;
    [SerializeField] private float popDownDuration = 0.08f;
    [SerializeField] private float popReturnDuration = 0.12f;

    [Header("Events")]
    [SerializeField] private TargetHitEvent onTargetHit;
    [SerializeField] private UnityEvent onTargetDestroyed;

    private Vector3 baseScale = Vector3.one;
    private int hitCount;
    private bool isDestroying;

    private PopPhase phase = PopPhase.Idle;
    private float phaseElapsed;
    private float currentScale = 1f;
    private float phaseStartScale = 1f;

    public int HitCount => hitCount;
    public int HitsRemaining => Mathf.Max(0, hitsToDestroy - hitCount);

    private void Awake()
    {
        baseScale = transform.localScale;
    }

    private void Update()
    {
        if (phase == PopPhase.Idle) return;

        phaseElapsed += Time.deltaTime;

        float duration = Mathf.Max(GetPhaseDuration(), 0.0001f);
        float alpha = Mathf.Clamp01(phaseElapsed / duration);

        currentScale = Mathf.Lerp(phaseStartScale, GetPhaseTargetScale(), alpha);
        transform.localScale = baseScale * currentScale;

        if (alpha < 1f) return;

        switch (phase)
        {
            case PopPhase.Up:
                StartPhase(PopPhase.Down);
                break;

            case PopPhase.Down:
                StartPhase(PopPhase.Return);
                break;

            default:
                phase = PopPhase.Idle;
                currentScale = 1f;
                transform.localScale = baseScale;
                break;
        }
    }

    public void TakeHit(float damage, Vector3 hitPoint, Vector3 hitDirection)
    {
        hitCount++;

        StartPhase(PopPhase.Up);

        onTargetHit?.Invoke(hitCount, hitPoint);

        if (hitCount >= hitsToDestroy && !isDestroying)
        {
            isDestroying = true;
            onTargetDestroyed?.Invoke();
            StartCoroutine(DestroyRoutine());
        }
    }

    private void StartPhase(PopPhase newPhase)
    {
        phase = newPhase;
        phaseElapsed = 0f;
        phaseStartScale = currentScale;
    }

    private float GetPhaseDuration()
    {
        switch (phase)
        {
            case PopPhase.Up: return popUpDuration;
            case PopPhase.Down: return popDownDuration;
            case PopPhase.Return: return popReturnDuration;
            default: return 0f;
        }
    }

    private float GetPhaseTargetScale()
    {
        switch (phase)
        {
            case PopPhase.Up: return popUpScale;
            case PopPhase.Down: return popDownScale;
            case PopPhase.Return: return 1f;
            default: return 1f;
        }
    }

    private IEnumerator DestroyRoutine()
    {
        yield return new WaitForSeconds(Mathf.Max(destroyDelay, 0.01f));
        Destroy(gameObject);
    }
}