
/*
    This script works as a starter and modular template for creating interactable objects in Unity. 
    It supports both click-based and trigger-based interactions, along with hover events.
    It includes customizable cooldowns for interactions and hover events to prevent rapid triggering.
    The script is designed to be flexible and easily extendable for various interaction scenarios.
    
    - Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
*/

using System.Collections;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.InputSystem;

// Must have a Trigger attached to the GameObject for trigger-based interactions.
// Must have a Collider attached to the GameObject for click-based interactions.
public class Interactable : MonoBehaviour
{
    [Header("Interactable State")]
    [SerializeField] private bool isInteractable = true;

    [Header("Click-Based Interaction Settings")]
    [SerializeField] private bool isClickBased = false;
    [SerializeField] private bool hasClickDistance = false;
    [SerializeField] private float clickDistance = 50f;
    [Tooltip("Assign a specific camera for interaction raycasting. If left empty, the main camera will be used.")]
    [SerializeField] private Camera interactionCamera;

    [Header("Trigger-Based Interaction Settings")]
    [SerializeField] private bool isTriggerBased = true;
    [SerializeField] private string interactorTag = "Player";

    [Header("Interact Events")]
    [Space(10)]
    [SerializeField] private UnityEvent onInteract;
    [SerializeField] private bool hasInteractCooldown = false;
    [SerializeField] private float interactCooldownDuration = 1f;

    [Space(10)]

    [Header("Hover Events")]
    [Space(10)]
    [SerializeField] private bool hasHoverEvents = false;
    [SerializeField] private UnityEvent onHoverEnter;
    [SerializeField] private bool hasHoverEnterCooldown = false;
    [SerializeField] private float hoverEnterCooldownDuration = 1f;

    [Space(20)]

    [SerializeField] private UnityEvent onHoverStay;
    [SerializeField] private bool hasHoverStayCooldown = false;
    [SerializeField] private float hoverStayCooldownDuration = 1f;

    [Space(20)]

    [SerializeField] private UnityEvent onHoverExit;
    [SerializeField] private bool hasHoverExitCooldown = false;
    [SerializeField] private float hoverExitCooldownDuration = 1f;
    [HideInInspector] public bool IsInteractable => isInteractable;

    protected bool _isHovered = false;
    protected bool _canInteract = false;
    private bool _isInsideTrigger = false;

    // Cooldown management
    protected Coroutine _interactCooldownCoroutine;
    protected Coroutine _hoverEnterCooldownCoroutine;
    protected Coroutine _hoverStayCooldownCoroutine;
    protected Coroutine _hoverExitCooldownCoroutine;
    protected bool _isOnInteractCooldown = false;
    protected bool _isOnHoverEnterCooldown = false;
    protected bool _isOnHoverStayCooldown = false;
    protected bool _isOnHoverExitCooldown = false;

    void Start()
    {
        if (isTriggerBased){
            isClickBased = false;
        }
        else if (isClickBased){
            _canInteract = true;
            isTriggerBased = false;
        }
    }

    void Update()
    {
        if (!isInteractable) return;

        if (hasHoverEvents)
        {
            bool hovering = false;
            if (isClickBased && Cursor.visible)
            {
                hovering = IsMouseHovering(); // One raycast per frame
            }
            else if (isTriggerBased)
            {
                hovering = _canInteract; // Trigger-based hover
            }

            if (isTriggerBased && _isOnInteractCooldown) { hovering = false; } // Prevent hover during interact cooldown [Remove if undesired]
            HandleHoverState(hovering);
        }

        if (_canInteract)
        {
            // Trigger-based interaction
            if (isTriggerBased && InputManager.Instance.isInteracting)
            {
                Interact();
                return;
            }

            // Click-based interaction
            if (isClickBased && Cursor.visible && IsClicking())
            {
                Interact();
            }
        }
    }

    #region Interaction and Hover Event Methods
    private void Interact() { 
        onInteract?.Invoke(); 
        if (hasInteractCooldown && _interactCooldownCoroutine == null)
        {
            _interactCooldownCoroutine = StartCoroutine(InteractCooldown());
            _canInteract = false;
        }
        Debug.Log("Interacted with " + gameObject.name);
    }

    private void OnHoverEnter()
    {
        if(hasHoverEnterCooldown && _isOnHoverEnterCooldown) return;
        if (hasHoverEvents && !_isHovered)
        {
            Debug.Log("Hovering (Enter) over " + gameObject.name);
            onHoverEnter?.Invoke();
            _isHovered = true;
            if (hasHoverEnterCooldown)
            {
                _isOnHoverEnterCooldown = true;
                _hoverEnterCooldownCoroutine = StartCoroutine(HoverEnterCooldown());
            }
        }
    }

    private void OnHoverStay()
    {
        if(hasHoverStayCooldown && _isOnHoverStayCooldown) return;
        if (hasHoverEvents && _isHovered)
        {
            //Debug.Log("Hovering over " + gameObject.name); // Uncomment for continuous hover logging
            onHoverStay?.Invoke();
            _isHovered = true;
            if (hasHoverStayCooldown)
            {
                _isOnHoverStayCooldown = true;
                _hoverStayCooldownCoroutine = StartCoroutine(HoverStayCooldown());
            }
        }
    }

    private void OnHoverExit()
    {
        if(hasHoverExitCooldown && _isOnHoverExitCooldown) return;
        if (hasHoverEvents && _isHovered)
        {
            Debug.Log("Stopped hovering over " + gameObject.name);
            onHoverExit?.Invoke();
            _isHovered = false;
            if (hasHoverExitCooldown)
            {
                _isOnHoverExitCooldown = true;
                _hoverExitCooldownCoroutine = StartCoroutine(HoverExitCooldown());
            }
        }
    }
    #endregion

    #region Interaction Handlers
    // Methods to handle trigger-based interaction
    void OnTriggerEnter(Collider other)
    {
        if (!isTriggerBased || !isInteractable)
            return;

        if (other.CompareTag(interactorTag))
        {
            _isInsideTrigger = true;

            if (!_isOnInteractCooldown) {_canInteract = true;}
        }
    }

    void OnTriggerExit(Collider other)
    {
        if (!isTriggerBased || !isInteractable)
            return;

        if (other.CompareTag(interactorTag))
        {
            _isInsideTrigger = false;
            _canInteract = false;
        }
    }

    // Method to check for click-based interaction
    private bool IsClicking()
    {
        if (!isClickBased || _isOnInteractCooldown || !isInteractable)
            return false;

        // Modern Input System click check
        if (!Mouse.current.leftButton.wasPressedThisFrame)
            return false;

        Camera cam = interactionCamera != null ? interactionCamera : Camera.main;
        if (cam == null)
            return false;

        Ray ray = cam.ScreenPointToRay(Mouse.current.position.ReadValue());

        float maxDistance = hasClickDistance ? clickDistance : Mathf.Infinity;

        if (Physics.Raycast(ray, out RaycastHit hit, maxDistance, ~0, QueryTriggerInteraction.Ignore))
        {
            return hit.collider != null && hit.collider.gameObject == gameObject;
        }

        return false;
    }

    private void HandleHoverState(bool hovering)
    {
        // Hover Enter
        if (hovering && !_isHovered && !_isOnHoverEnterCooldown)
        {
            OnHoverEnter();
            return;
        }

        // Hover Stay
        if (hovering && _isHovered && !_isOnHoverStayCooldown)
        {
            OnHoverStay();
            return;
        }

        // Hover Exit
        if (!hovering && _isHovered && !_isOnHoverExitCooldown)
        {
            OnHoverExit();
            return;
        }
    }
 
    private bool IsMouseHovering()
    {
        if (!isClickBased || !isInteractable)
            return false;

        Camera cam = interactionCamera != null ? interactionCamera : Camera.main;
        if (cam == null)
            return false;

        Ray ray = cam.ScreenPointToRay(Mouse.current.position.ReadValue());

        float maxDistance = hasClickDistance ? clickDistance : Mathf.Infinity;

        if (Physics.Raycast(ray, out RaycastHit hit, maxDistance, ~0, QueryTriggerInteraction.Ignore))
        {
            return hit.collider != null && hit.collider.gameObject == gameObject;
        }

        return false;
    }
    #endregion

    //Method to enable or disable interaction externally
    public void SetInteractable(bool state)
    {
        isInteractable = state;

        if (!isInteractable){
            _canInteract = false;
            _isOnInteractCooldown = false;
            _isOnHoverEnterCooldown = false;
            _isOnHoverStayCooldown = false;
            _isOnHoverExitCooldown = false;
            _isHovered = false;
            StopAllCoroutines();
        }
    }

    #region Cooldown Coroutines
    // Coroutine to handle interaction cooldown
    private IEnumerator InteractCooldown()
    {
        _isOnInteractCooldown = true;
        yield return new WaitForSecondsRealtime(interactCooldownDuration);
        _isOnInteractCooldown = false;

        if (isTriggerBased && _isInsideTrigger)
        {
            _canInteract = true;
        }
        _interactCooldownCoroutine = null;
    }

    private IEnumerator HoverEnterCooldown()
    {
        _isOnHoverEnterCooldown = true;
        yield return new WaitForSecondsRealtime(hoverEnterCooldownDuration);
        _isOnHoverEnterCooldown = false;
        _hoverEnterCooldownCoroutine = null;
    }

    private IEnumerator HoverStayCooldown()
    {
        _isOnHoverStayCooldown = true;
        yield return new WaitForSecondsRealtime(hoverStayCooldownDuration);
        _isOnHoverStayCooldown = false;
        _hoverStayCooldownCoroutine = null;
    }

    private IEnumerator HoverExitCooldown()
    {
        _isOnHoverExitCooldown = true;
        yield return new WaitForSecondsRealtime(hoverExitCooldownDuration);
        _isOnHoverExitCooldown = false;
        _hoverExitCooldownCoroutine = null;
    }

    #endregion

    #region Safety Methods
    void OnDestroy()
    {
        StopAllCoroutines();
    }

    void OnDisable()
    {
        SetInteractable(false);
    }
    #endregion
}
