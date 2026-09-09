# 23. FPS Self-Animated Weapon System

Welcome to the **FPS Self-Animated Weapon System**, a core gameplay mechanic in the LVN Gameplay Programming Showcase series.

A first person weapon system where every piece of feel is procedural. No animation clips, no Animator or Animation Blueprint, no timeline assets. Recoil, reload pose, aim transitions, fire mode switches, landing impact and hit reactions are all derived in code from spring integrators, phase lerps and state. Ships with an arcade first person controller in the same entry, so movement and weapons are demonstrated interacting with each other rather than sitting side by side.

<h2 align="center">Overview</h2>

<p align="center">
  <img width="800" alt="Unity and Unreal Cpp + Blueprints FPS Self-Animated Weapon System by LucasVN" src="https://github.com/user-attachments/assets/864d5b5b-aafa-41fa-b924-ee0ef5eb4a26"/>
</p>

---

## Important Clarification

**The core of this system is `IWeapon` and `WeaponDataSO` (Unity) / `IFPS_Weapon` and `UFPS_WeaponData` (Unreal).** Everything a weapon does, from ammo type to fire mode to pose, is defined by data. Adding a new weapon never touches code and never adds an animation set. It is one data asset, one viewmodel prefab or Blueprint, one pickup.

Everything else in this entry is example scaffolding built to demonstrate the weapon system doing something:

- **The arcade first person controller** (`FPS_Controller` / `AFPS_Character`) is included to show movement and weapons interacting, particularly the FOV composition and dash. It has zero awareness that weapons exist. The single contact point is a raw FOV override value, never a weapon reference.
- **`Target` / `AFPS_Target`** is a shootable range prop that counts hits and ignores damage. It exists to give the weapons something to hit, not to demonstrate a health system.
- **`WeaponHUD` / `UFPS_WeaponHUD`** is example UI. The extension point for your own HUD is the holder's events (`OnAmmoChanged`, `OnActiveWeaponChanged`, `OnAmmoRejected`, `OnFireModeChanged`) in Unity, or the equivalent `BlueprintAssignable` delegates in Unreal.
- **Pickups** (`WeaponPickup`, `AmmoPickup` / `AFPS_WeaponPickup`, `AFPS_AmmoPickup`) show equip and ammo routing working end to end but are not required to use the weapon system in your own project.

**Unity Note**: Requires a `WeaponDataSO` assigned per weapon and an `InputManager` singleton extended with the five weapon properties. Two layers are required, one for shootables and one for interactables, and neither should include the layer the player occupies.

**Unreal Note**: Requires a `UFPS_WeaponData` Data Asset assigned per weapon. Two trace channels must be created in order, `Weapon` then `Interact`, both defaulting to Ignore, matching the C++ defaults of `GameTraceChannel1` and `GameTraceChannel2`. The HUD must be spawned from an `AHUD` Blueprint's `BeginPlay`, not the Player Controller's, since the pawn is not possessed yet.

**BOTH ENGINES**: The weapon trace and the interaction trace are fully separate, with no shared state, range or gate. Keep them on different layers or channels.

---

## Core Features

- **Fully Procedural Feel**
  - Recoil, camera dip, FOV punch, reload pose and fire mode switching all run through spring integrators (Unity: hand rolled per script, Unreal: a shared `FFPS_Spring` / `FFPS_VectorSpring` struct).
  - Impulses stack on a live spring rather than restarting, so rapid fire compounds instead of resetting.
  - `Target` is the deliberate exception. It uses a three phase scale lerp (up, down, return) instead of a spring, since a hit pop is a keyframed effect with no continuous input. A hit mid pop restarts from the current scale, and the return phase always snaps to exactly base scale so repeated hits cannot drift.

- **Data-Driven Weapons**
  - Each weapon is a `WeaponDataSO` (Unity) / `UFPS_WeaponData` (Unreal) covering ammo type, fire mode, spread, poses and prefab or Blueprint references.
  - `PelletsPerShot` and `AmmoPerShot` are separate fields, so a shotgun is the same component as a pistol, just spending one shell to fire eight traces.
  - Fire mode is a bitflag set (Semi, Burst, Auto) so a weapon can support any combination and cycle only through what it has.

- **Absolute Weapon Poses**
  - Hip and aim poses are transforms relative to the attach point, which is always zeroed. Not offsets from a socket transform.
  - This is what lets an aim position of `0` genuinely center a weapon, and what keeps fixing one weapon's pose from affecting every other weapon on the same socket.
  - Aim blend timing lives on the holder and is identical across the arsenal. Only placement is per weapon.

- **Parameterless, Frame-Driven Fire**
  - `Fire()` takes no arguments and is called every frame the input is held. The weapon itself derives press versus hold by comparing against the previous frame.
  - Keeps fire mode logic entirely inside the data asset. The holder never needs to know which weapons are semi automatic.

- **Weapon-Owned Damage, Interface-Only Contract**
  - Damage lives on the weapon data and is passed through `IHittable` / `IFPS_Hittable`. `Target` receives it and ignores it, since it only counts hits.
  - Each implementer decides whether damage means anything. The weapon system never assumes a health model on the receiving end.

- **Pickup and Toss Behaviour**
  - One carried weapon. Picking up a second automatically tosses the first, removing any inventory concept the entry does not need.
  - Toss direction comes from camera forward, spawn position from a camera space offset, and velocity is set directly rather than applied as an impulse, so mass never distorts the arc.
  - Ammo pickups can be rejected on caliber mismatch or a capped reserve. A rejected pickup stays in the world and fires `OnAmmoRejected` rather than being silently consumed.

- **Reloads Are Cancelled, Never Banked**
  - Swapping or tossing mid reload cancels it outright. A reload that completed while the weapon was stowed would show up as ammo appearing from nowhere, which reads as a bug rather than a feature.

- **Movement and Camera Integration**
  - Gravity is asymmetric: base scale while rising, multiplied while falling, plus terminal velocity. A symmetric arc reads as floaty regardless of the numbers behind it.
  - FOV has one composition point. The walk and run blend is smoothed, but landing punch and dash kick are added raw on top, since smoothing flattens an impulse before it reads on screen.
  - No takeoff dip on jump. It fought the jump's own upward motion and read as a hitch. Only the landing dip remains.

- **Decoupled Event System**
  - Unity: `UnityEvent` for inspector wiring (`OnFired`, hit point events) alongside `event Action<...>` for typed code subscriptions (`OnAmmoChanged`, `OnActiveWeaponChanged`, `OnAmmoRejected`, `OnFireModeChanged`).
  - Unreal: `BlueprintAssignable` multicast delegates serving both C++ and Blueprint from a single declaration. Delegate accessors on `IFPS_Weapon` are native pure virtual rather than `UFUNCTION`, since a C++ interface cannot hold `UPROPERTY` delegates.
  - Both let the HUD, pickups and feedback layer subscribe without the weapon system knowing they exist.

---

## Engine Differences

### Unity (C#)
- `HitscanWeapon` is a `MonoBehaviour` evaluated in `LateUpdate`, after `WeaponHolder` forwards input in `Update`. Reversing the order loses a frame of trigger input.
- `WeaponDataSO` is the only `ScriptableObject`. Everything else, including the arcade controller, is a `MonoBehaviour`.
- `WeaponViewmodelFeedback` must sit on the weapon prefab root, never the ViewModelSocket. On the socket it silently overwrites the holder's aim blend every `LateUpdate`, and the symptom is every pose value on every data asset appearing to do nothing while the transform itself logs correctly. `WeaponHolder.Start` now logs an error if it finds one on the socket.
- The spring integrator is duplicated by hand across three scripts. Functional, but the Unreal build's shared struct is the better structure and worth backporting.
- `WeaponHolder.Local` is a static, since `IFP_Interactable.OnInteract(Vector3)` carries no reference back to the interactor. The Unreal version passes the interactor directly and needs no static.
- Debug traces are real `LineRenderer` instances with a dedicated `DebugTracerFade` component, not `Debug.DrawLine`, since the latter only renders in the Scene view.
- `AmmoType` and `IWeapon` are interface or data only files. Removing an entry from the `AmmoType` enum shifts serialized indices on every existing data asset, so every `WeaponDataSO` needs a re-check after any enum edit.

### Unreal Engine (C++)
- `AFPS_HitscanWeapon` implements `IFPS_Weapon`, stored on the holder as `TScriptInterface<IFPS_Weapon>`. A Blueprint-only implementer gets a null native delegate pointer, which makes subscription self guarding.
- `UFPS_ViewmodelFeedbackComponent` must sit at weapon actor level. On the attach point or the character it overwrites the aim blend the same way the Unity equivalent does on the socket. It now errors and disables itself if its owner does not implement `IFPS_Weapon`.
- The spring integrator is a shared header-only struct (`FFPS_Spring`, `FFPS_VectorSpring`) used by every system that needs one, rather than duplicated per script.
- HUD is a Widget Blueprint spawned from an `AHUD` Blueprint's `BeginPlay`. Spawning from the Player Controller fails, since the pawn is not possessed yet and `GetOwningPlayerPawn` returns null. `NativeConstruct` pushes weapon, ammo and fire mode once, since the holder equips before the widget exists.
- `OnDashUpdated` only fires while recharging plus the frame it flips ready, never at startup. The dash bar's default state has to be set by hand in the Designer tab.
- Widget Blueprints have no Timelines. Cooldown and dash bars use `Set Timer by Function Name` with a stored `FTimerHandle`.
- Live Coding cannot patch reflected types. Any `UPROPERTY` or `UENUM` change needs the editor closed and a full rebuild.
- Two trace channels, `Weapon` then `Interact`, both defaulting to Ignore, matching `GameTraceChannel1` and `GameTraceChannel2`. Enhanced Input bindings live on components rather than the character.

---

## Quick Summary

- **Fully Procedural**: No animation clips or Animator/Animation Blueprint anywhere in the system. Springs and phase lerps drive every piece of feel.
- **Data-Driven**: One data asset per weapon. Adding a weapon is never a code change.
- **Absolute Poses**: Hip and aim poses are zeroed against the attach point, not offsets from a socket.
- **Interface-Driven Damage**: Damage lives on the weapon, travels through a hit interface, and the sample target ignores it on purpose.
- **Single Slot Pickup**: One carried weapon, automatic toss on pickup, ammo pickups that can be refused without being consumed.
- **Cancelled Reloads**: Swap or toss cancels a reload outright rather than letting it complete while stowed.
- **Movement-Aware, Weapon-Agnostic Controller**: The arcade controller exposes a raw FOV override and nothing else, and has no knowledge that weapons exist.
- **Decoupled Events**: Two layers in Unity (inspector plus typed code), single multicast delegate in Unreal serving both Blueprint and C++.

---

<p align="center">
  <strong>Author:</strong> Lucas Varela Negro<br>
  <a href="https://www.linkedin.com/in/lucas-varela-negro/" target="_blank">
    <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white" alt="LinkedIn Badge">
  </a>
</p>
