using UnityEngine;

// Everything that makes one weapon differ from another. A second weapon is a new asset, never a
// new script. 
// Both poses are absolute local values under the player camera, so the ViewmodelSocket
// transform is ignored entirely and should be left at zero.
[CreateAssetMenu(fileName = "NewWeaponData", menuName = "LVN/Weapons/Weapon Data")]
public class WeaponDataSO : ScriptableObject
{
    [Header("Identity")]
    public string weaponName = "New Weapon";
    public Sprite weaponIcon;

    [Header("Ammo")]
    public AmmoType ammoType = AmmoType.Pistol;
    public int magSize = 12;
    public int reserveCap = 96;
    public int startingReserve = 36;
    public bool autoReloadOnEmpty = true;

    [Header("Fire")]
    public float fireRate = 8f;
    public float reloadDuration = 1.4f;
    public float damage = 10f;
    public float range = 120f;
    public int pelletsPerShot = 1;
    public int ammoPerShot = 1;

    [Header("[Fire] Modes")]
    public FireMode supportedFireModes = FireMode.Semi;
    public FireMode defaultFireMode = FireMode.Semi;
    public int burstCount = 3;
    public float burstDelay = 0.06f;

    [Header("[Fire] Spread")]
    public float hipSpread = 3f;
    public float aimSpread = 0.35f;

    [Header("Hip Pose")]
    public Vector3 hipPosition = new Vector3(0.18f, -0.14f, 0.32f);
    public Vector3 hipRotation = Vector3.zero;

    [Header("Aim Pose")]
    public Vector3 aimPosition = new Vector3(0f, -0.05f, 0.3f);
    public Vector3 aimRotation = Vector3.zero;
    public float aimFov = 45f;

    [Header("Prefabs")]
    public GameObject viewmodelPrefab;
    public GameObject pickupPrefab;

    public float TimeBetweenShots => fireRate <= 0f ? 0f : 1f / fireRate;

    public bool Supports(FireMode mode) => (supportedFireModes & mode) != 0;

    private void OnValidate()
    {
        if (supportedFireModes == FireMode.None)
            supportedFireModes = FireMode.Semi;

        if (!Supports(defaultFireMode))
            defaultFireMode = Supports(FireMode.Semi) ? FireMode.Semi
                            : Supports(FireMode.Burst) ? FireMode.Burst
                            : FireMode.Auto;

        magSize = Mathf.Max(1, magSize);
        pelletsPerShot = Mathf.Max(1, pelletsPerShot);
        ammoPerShot = Mathf.Clamp(ammoPerShot, 1, magSize);
        burstCount = Mathf.Max(1, burstCount);
        reserveCap = Mathf.Max(0, reserveCap);
        startingReserve = Mathf.Clamp(startingReserve, 0, reserveCap);
        aimFov = Mathf.Clamp(aimFov, 10f, 120f);
    }
}
