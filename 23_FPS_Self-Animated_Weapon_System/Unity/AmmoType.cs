public enum AmmoType
{
    Pistol,
    Shotgun,
    Rifle
}

[System.Flags]
public enum FireMode
{
    None = 0,
    Semi = 1 << 0,
    Burst = 1 << 1,
    Auto = 1 << 2
}
