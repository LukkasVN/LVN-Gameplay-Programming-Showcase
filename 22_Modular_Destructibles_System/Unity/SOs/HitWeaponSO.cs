using UnityEngine;

/*
    Data container for a hit weapon.

    - Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
*/

[CreateAssetMenu(fileName = "HitWeaponSO", menuName = "Scriptable Objects/HitWeaponSO")]
public class HitWeaponSO : ScriptableObject
{
    [Header("Identity")]
    public string weaponName;
    [Tooltip("Optional — for hotbar or inventory UI display.")]
    public Sprite icon;

    [Header("Stats")]
    [Min(1)] public int damage = 1;
    [Tooltip("Must be >= DestructibleSO.tierRequired on the target or the hit is rejected.")]
    [Min(0)] public int tierLevel = 0;
    [Min(0.1f)] public float hitRange = 4f;
    [Tooltip("Minimum time between hits in seconds.")]
    [Min(0.05f)] public float hitCooldown = 0.3f;

    [Header("Camera Shake On Hit")]
    public bool useHitShake = true;
    [Range(0f, 1f)] public float shakeIntensity = 0.05f;
    [Range(0.05f, 0.5f)] public float shakeDuration = 0.1f;

    // Audio Data
    // public AudioClip swingSFX;
    // public AudioClip neutralHitSFX;
}