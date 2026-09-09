using System;
using UnityEngine;

public struct WeaponFireInfo
{
    public Vector3 origin;
    public Vector3 direction;
    public bool didHit;
    public RaycastHit hit;
}

// Contract WeaponHolder talks to. It never references a concrete weapon type, so a projectile,
// beam or melee implementation drops in with zero holder changes.
public interface IWeapon
{
    WeaponDataSO Data { get; }
    int CurrentMagAmmo { get; }
    int CurrentReserveAmmo { get; }
    FireMode CurrentFireMode { get; }
    bool IsReloading { get; }

    event Action OnAmmoChanged;
    event Action<WeaponFireInfo> OnFired;
    event Action OnFireModeChanged;

    // Pass -1 for either ammo value to fall back to the data asset defaults, which is how a world
    // pickup differs from a tossed weapon carrying a live snapshot.
    void Initialize(WeaponDataSO weaponData, Camera fireCamera, LayerMask hitMask, int magAmmo, int reserveAmmo);

    // Called every frame the fire input is held. The implementation derives press versus hold from
    // that, which keeps the Semi/Burst/Auto decision inside the weapon where its data lives.
    void Fire();

    void Reload();
    void CancelActions();
    void AimIn();
    void AimOut();
    void CycleFireMode();
    int AddReserveAmmo(int amount);
}
