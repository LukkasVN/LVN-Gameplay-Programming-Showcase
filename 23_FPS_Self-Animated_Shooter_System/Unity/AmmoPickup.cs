using UnityEngine;
using UnityEngine.Events;

public class AmmoPickup : MonoBehaviour, IFP_Interactable, IInteractPrompt
{
    [Header("Payload")]
    [SerializeField] private AmmoType ammoType = AmmoType.Pistol;
    [SerializeField] private int amount = 24;

    [Header("Prompt")]
    [SerializeField] private string interactPrompt = "Take Ammo";

    [Header("Feedback Hooks")]
    [SerializeField] private UnityEvent onFocusEnter;
    [SerializeField] private UnityEvent onFocusExit;
    [SerializeField] private UnityEvent onCollected;
    [SerializeField] private UnityEvent onPickupRejected;

    public string InteractPrompt => interactPrompt;
    public AmmoType AmmoType => ammoType;
    public int Amount => amount;

    public void OnFocusEnter() => onFocusEnter?.Invoke();

    public void OnFocusExit() => onFocusExit?.Invoke();

    public void OnInteract(Vector3 interactDirection)
    {
        WeaponHolder holder = WeaponHolder.Local;
        if (holder == null) return;

        if (!holder.AddAmmo(ammoType, amount))
        {
            onPickupRejected?.Invoke();
            return;
        }

        onCollected?.Invoke();
        OnFocusExit();
        Destroy(gameObject);
    }
}
