using UnityEngine;

/*
    Data container for a destructible object.
    One SO per destructible type (Rock, Crate, Barrel, Pot...).
    Damage stage materials are fully optional, leave the array empty to skip material progression.

    - Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
*/

[CreateAssetMenu(fileName = "DestructibleSO", menuName = "Scriptable Objects/DestructibleSO")]
public class DestructibleSO : ScriptableObject
{
    [Header("Identity")]
    public string destructibleName;
    [TextArea(2, 5)]
    public string destructibleDescription;
    [Tooltip("Optional — for future UI use such as icons or quest tracking.")]
    public Sprite uiIcon;

    [Header("Stats")]
    [Min(1)] public int hitPoints = 3;
    [Tooltip("Minimum HitWeaponSO tier level required to damage this object. 0 = any weapon.")]
    public int tierRequired = 0;

    [Header("Hit Pop Effect")]
    [Tooltip("Toggle the scale-punch feedback on each hit. Disable for very large objects.")]
    public bool useHitPopEffect = true;
    [Range(0.05f, 0.5f)]
    public float popEffectIntensity = 0.15f;
    [Range(0.05f, 0.3f)]
    public float popEffectDuration = 0.1f;

    [Header("Destroy Settings")]
    public GameObject destroyedPrefab; // Optional prefab to spawn on destruction.

    // Audio Data
    // public AudioClip destroySFX;
    // public AudioClip hitSFX;
    // public AudioClip wrongTierSFX;

}