using System;
using UnityEngine;

[DisallowMultipleComponent]
public class PlayerInteractor : MonoBehaviour
{
    [Header("References")]
    [SerializeField] private Camera interactCamera;

    [Header("Interaction")]
    [SerializeField] private float interactDistance = 3.5f;
    [SerializeField] private LayerMask interactMask = ~0;

    private IFP_Interactable currentInteractable;
    private bool allowInteraction = true;

    public IFP_Interactable CurrentInteractable => currentInteractable;

    public event Action<string> OnPromptChanged;

    public string CurrentPrompt { get; private set; } = string.Empty;

    private void Awake()
    {
        if (interactCamera == null && TryGetComponent(out FPS_Controller controller))
            interactCamera = controller.GetPlayerCamera();
    }

    private void Start()
    {
        if (interactCamera == null)
        {
            Debug.LogError("[PlayerInteractor] No camera reference. Disabling.");
            enabled = false;
        }
    }

    private void Update()
    {
        if (!allowInteraction) return;

        Ray ray = new Ray(interactCamera.transform.position, interactCamera.transform.forward);

        if (Physics.Raycast(ray, out RaycastHit hit, interactDistance, interactMask)
            && hit.collider.TryGetComponent(out IFP_Interactable interactable))
        {
            SetFocus(interactable);

            if (InputManager.Instance.isInteracting)
                interactable.OnInteract(interactCamera.transform.forward);

            return;
        }

        SetFocus(null);
    }

    private void SetFocus(IFP_Interactable next)
    {
        if (next == currentInteractable) return;

        currentInteractable?.OnFocusExit();
        currentInteractable = next;
        currentInteractable?.OnFocusEnter();

        CurrentPrompt = currentInteractable is IInteractPrompt prompt ? prompt.InteractPrompt : string.Empty;
        OnPromptChanged?.Invoke(CurrentPrompt);
    }

    public void HandleInteractionState(bool canInteract)
    {
        allowInteraction = canInteract;
        if (!canInteract) SetFocus(null);
    }

    private void OnDrawGizmosSelected()
    {
        if (interactCamera == null) return;
        Gizmos.color = Color.cyan;
        Gizmos.DrawRay(interactCamera.transform.position, interactCamera.transform.forward * interactDistance);
    }
}
