using UnityEngine;
using UnityEngine.Events;

// Placed in the editor it carries -1 counts, meaning data asset defaults. Tossed, it carries the live counts instead, so a
// gun thrown down is the same gun when picked back up.
public class WeaponPickup : MonoBehaviour, IFP_Interactable, IInteractPrompt
{
    [Header("Payload")]
    [SerializeField] private WeaponDataSO weaponData;
    [Tooltip("Leave at -1 for the data asset defaults.")]
    [SerializeField] private int magAmmo = -1;
    [SerializeField] private int reserveAmmo = -1;

    [Header("Prompt")]
    [Tooltip("A {weapon} token is replaced with the weapon name.")]
    [SerializeField] private string interactPrompt = "Grab Weapon";

    [Header("Feedback Hooks")]
    [SerializeField] private UnityEvent onFocusEnter;
    [SerializeField] private UnityEvent onFocusExit;
    [SerializeField] private UnityEvent onCollected;
    [Tooltip("Reached only when the currently held weapon could not be tossed to make room.")]
    [SerializeField] private UnityEvent onPickupBlocked;

    public string InteractPrompt => weaponData != null
        ? interactPrompt.Replace("{weapon}", weaponData.weaponName)
        : interactPrompt;

    public WeaponDataSO WeaponData => weaponData;

    public void InitializeFromToss(WeaponDataSO data, int currentMag, int currentReserve)
    {
        weaponData = data;
        magAmmo = currentMag;
        reserveAmmo = currentReserve;
    }

    public void OnFocusEnter() => onFocusEnter?.Invoke();

    public void OnFocusExit() => onFocusExit?.Invoke();

    public void OnInteract(Vector3 interactDirection)
    {
        WeaponHolder holder = WeaponHolder.Local;
        if (holder == null || weaponData == null) return;

        if (!holder.TryPickUpWeapon(weaponData, magAmmo, reserveAmmo))
        {
            onPickupBlocked?.Invoke();
            return;
        }

        onCollected?.Invoke();
        OnFocusExit();
        Destroy(gameObject);
    }
}
