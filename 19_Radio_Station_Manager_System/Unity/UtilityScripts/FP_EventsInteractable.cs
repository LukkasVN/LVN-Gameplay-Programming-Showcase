using UnityEngine;
using UnityEngine.Events;

public class FP_EventsInteractable : MonoBehaviour, IFP_Interactable
{
    [Header("Events")]
    public UnityEvent onFocusEnter;
    public UnityEvent onFocusExit;
    public UnityEvent onInteract;

    [Header("Interaction Settings")]
    [SerializeField] private float interactCooldown = 0.5f;

    private float lastInteractTime = -999f;

    [HideInInspector]
    public Vector3 lastInteractionDirection;

    void Start()
    {
        if(gameObject.layer != LayerMask.NameToLayer("Interactable"))
        {
            Debug.LogWarning($"GameObject '{gameObject.name}' is using FP_EventsInteractable but is not on the 'Interactable' layer. Please set it to the correct layer for interaction to work properly.");
        }
    }

    public void OnFocusEnter()
    {
        onFocusEnter?.Invoke();
    }

    public void OnFocusExit()
    {
        onFocusExit?.Invoke();
    }

    public void OnInteract(Vector3 interactionDirection)
    {
        if (Time.time < lastInteractTime + interactCooldown)
            return;

        lastInteractTime = Time.time;

        lastInteractionDirection = interactionDirection;
        onInteract?.Invoke();
    }
}
