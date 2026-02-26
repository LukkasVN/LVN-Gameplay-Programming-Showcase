# 16 First‑Person Flashlight System

Welcome to the **First‑Person Flashlight System** section of the LVN Gameplay Programming Showcase series.

This module implements a fully modular flashlight mechanic for first‑person games, featuring battery simulation, flicker behavior, inertia‑based lag, and designer‑friendly configuration.  
The system is implemented in both Unity and Unreal Engine using the same architectural principles: explicit control, clean separation of concerns, and predictable designer workflows.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/ce0a5d25-9b4b-47bc-98e0-692fe8eff964" width="600px" />
</p>

---

## Core Features

- **Battery Simulation**
  - Configurable max lifetime
  - Instant and gradual recharge
  - Low‑battery threshold logic

- **Light Behavior**
  - Adjustable intensity, cone angles, and attenuation
  - Low‑battery flicker with tunable randomness
  - Toggle on/off with safety checks

- **Inertia‑Based Lag**
  - Smooth rotation lag for a grounded, physical feel
  - Independent pivot rotation (Unreal) or local offset (Unity)
  - Designer‑controlled lag speed

- **Debug & Designer Tools**
  - On‑screen battery percentage
  - Clear, explicit parameters for tuning
  - Modular component‑based architecture

---

## Engine Differences

### Unity
Unity’s transform system makes it straightforward to implement flashlight inertia and battery logic directly in code:

- Flashlight pivot is a standalone `Transform`
- Lag is applied via `Quaternion.Lerp` or `SmoothDamp`
- Flicker and battery drain run in Update
- Light properties controlled through `Light` component
- Recharge and flicker curves can be added easily

Unity’s flexibility allows the entire system to be self‑contained and portable.

---

### Unreal Engine
Unreal requires more attention to component hierarchy due to transform inheritance:

- Flashlight Pivot must **not** inherit camera rotation  
  (`SetUsingAbsoluteRotation(true)` recommended)
- Lag is applied via `FMath::RInterpTo` on the Pivot
- Battery, flicker, and recharge handled inside a custom `UActorComponent`
- Light behavior controlled through `USpotLightComponent`
- Debug UI uses `GEngine->AddOnScreenDebugMessage`

In short:  
**Unity gives full transform freedom, while Unreal requires careful hierarchy setup to enable independent lag.**

---

## Quick Summary

The **First‑Person Flashlight System** provides a realistic, modular flashlight mechanic with battery simulation, flicker behavior, and physical lag.  
Unity focuses on script‑driven control, while Unreal integrates deeply with its component hierarchy and transform system.

Despite these differences, both implementations share the same design goals:

- Explicit, predictable behavior  
- Designer‑friendly parameters  
- Modular, reusable architecture  
- Grounded, physical feel through inertia‑based lag  
