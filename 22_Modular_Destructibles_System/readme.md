# 22. Modular Destructibles System

Welcome to the **Modular Destructibles System**, a core gameplay mechanic in the LVN Gameplay Programming Showcase series.

Data-driven destructible objects with damage, visual feedback, optional debris spawning, and respawn support. Hit by a weapon-driven first-person controller, all behaviour configured through data assets with decoupled hooks for drops, particles, and audio.

<h2 align="center">Overview</h2>

<p align="center">
  <img width="800" alt="Unity and Unreal Cpp + Blueprints Modular Destructibles System by LucasVN" src="https://github.com/user-attachments/assets/790aff33-9188-4763-b6d4-9ea72d947118"/>
</p>

---

## Important Clarification

**The core of this system is `Destructible` and `DestructibleSO` (Unity) / `ADestructibleActor` and `UDestructibleData` (Unreal).** All behaviour is configured through ScriptableObjects (Unity) or DataAssets (Unreal). No code changes are needed to add new destructible types.

Everything else is example scaffolding or optional extensions:

- **`DestructibleDropSpawner` / `UDestructibleDropSpawnerComponent`** bridges this system to the [**Modular Droppables System (Section 21)**](https://github.com/LukkasVN/LVN-Gameplay-Programming-Showcase/tree/main/21_Modular_Droppables_System). It subscribes to the destructible's events and forwards drop calls. Remove it and the destructible system works with no knowledge of drops.
- **The hit cooldown bar and destructible hover panel** are example UI hooks. The extension point for your own HUD is `OnHitCooldownStarted`, `OnDestructibleFocused`, and `OnDestructibleUnfocused` (Unreal) or the `UIManager` calls (Unity).
- **The particle example region** inside `Destructible` (Unity) / `ADestructibleActor::TakeHit_Implementation` (Unreal) shows spatially-aware hit and destroy feedback. Marked `[Recommended Removal]` in Unity since production projects should route this through a pooled VFXManager. In Unreal, `UNiagaraFunctionLibrary::SpawnSystemAtLocation` with `ENCPoolMethod::AutoRelease` is used directly as a lightweight showcase approach.

**Unity Note**: Requires a `DestructibleSO` assigned in the Inspector and a `HitWeaponSO` on `FP_Controller`. Destructibles must be on a layer included in the `hitMask`.

**Unreal Note**: Requires a `UDestructibleData` assigned on `ADestructibleActor` and a `UHitWeaponData` assigned as `ActiveWeapon` on `AFP_Character`. Uses a `ECC_Visibility` line trace gated on `bCanHitFlag` and `ActiveWeapon`. The `HitAction` Input Action must be assigned in the Character Blueprint and mapped in the IMC. Enhanced Input produces no errors if the binding is missing, so this is the first thing to check if hits are silent.

**Unreal Droppables Note**: This section required additions to `ADroppableItem` from Section 21, specifically a physics-based drop mode (`bPhysicsBased`) and a `Settled` state for debris items that should stay where physics leaves them. If you are using the Droppables System from Section 21, check the updated files included here.

**FP_Character / FP_Controller Note**: The attached character file is included to show how the destructible system hooks into the player controller. The recommended approach is to inspect those additions and apply them to your own controller rather than using this file directly. If you do not have an existing controller, you can use it as a base but will need to pull in the dependencies from the previous sections. This applies to future sections as well, as the same controller continues to grow with each new system.

**BOTH ENGINES**: The hit system is a separate trace from the interaction system. They share no state, no range, no gate. Keep them on different layers/channels to avoid cross-triggering.

---

## Core Features

- **Data-Driven Type Definitions**
  - Each type (Rock, Crate, Barrel...) is defined by a `DestructibleSO` (Unity) / `UDestructibleData` (Unreal).
  - Covers identity (name, description, icon), stats (HP, tier requirement), hit feedback (pop intensity and duration), and the optional destroyed prefab/actor class.
  - New types require zero code.

- **Weapon-Driven Hit System**
  - Hits originate from the active weapon data asset (`HitWeaponSO` / `UHitWeaponData`), which defines damage, tier, range, cooldown, and optional camera shake.
  - Tier gating means low-tier weapons bounce off high-tier objects, supporting tool progression.
  - Multiple weapons can be assigned and switched at runtime.
  - Per-weapon hit cooldown drives the optional cooldown bar UI.

- **Hit Feedback**
  - On every successful hit, the destructible plays a scale pop effect.
  - Unity adds an independent horizontal position shake in the hit direction. Unreal keeps only the pop.
  - Pop intensity and duration are per-type. Camera shake is per-weapon.

- **Two Destruction Paths**
  - **One-Shot**: On HP reaching 0, fires events, optionally spawns a destroyed prefab/actor, then destroys after a small delay so in-flight callbacks finish cleanly.
  - **Respawnable**: Deactivates visuals and collision in place instead of destroying. `RespawnDestructible()` / `RestoreDestructible()` reactivates from any external system. Auto-respawn via a delay timer is optional.
  - The choice is per-instance, not per-type.

- **Destroyed Prefab / Actor Hook**
  - Optional prefab (Unity) or actor class (Unreal) spawned at the destruction point with matching rotation, scale, and a configurable offset.
  - Cleaned up automatically on respawn.

- **Decoupled Event System**
  - Unity: `UnityEvent` (`onHit` / `onDestroyed`) for inspector wiring, `event Action<...>` (`OnHitWithData` / `OnDestroyedWithData`) for typed code subscriptions.
  - Unreal: `BlueprintAssignable` multicast delegates (`OnHit`, `OnDestroy`, `OnRestored`) serving both C++ and Blueprint from a single declaration.
  - Both let downstream systems subscribe without the destructible knowing they exist.

- **Optional Drop Integration**
  - Two independent drop tables per destructible: one rolled on every hit, one rolled on destruction. Leave either empty to skip.
  - Each entry uses a two-dial probability model: `dropChance` gates whether the entry drops at all, `bonusRollChance` controls the binomial distribution of quantity between min and max.
  - Adding the component opts in, removing it opts out.

- **Focus and Hover Feedback**
  - Aiming at a destructible within weapon range shows a hover panel with name and current HP.
  - Unreal exposes this via `OnDestructibleFocused(UDestructibleData*, int32 CurrentHP)` and `OnDestructibleUnfocused` on `AFP_Character`.

- **Hit Cooldown Bar**
  - Optional UI bar draining over the active weapon's cooldown duration.
  - Triggered by `OnHitCooldownStarted(float Duration)` (Unreal) or from `FP_Controller` (Unity).
  - Unreal uses a looping `Set Timer by Function Name` in a UMG Widget Blueprint since Widget Blueprints do not support Timelines.

---

## Engine Differences

### Unity (C#)
- `Destructible` is a `MonoBehaviour` driven by `DestructibleSO`, with per-instance fields for placement-level decisions (respawnable, destroyed prefab toggle, destroy delay).
- `HitWeaponSO` and `DestructibleSO` extend `ScriptableObject` with `[CreateAssetMenu]`.
- Hit detection uses `Physics.Raycast` on a configurable `LayerMask` from the camera forward vector.
- Pop and shake run as independent coroutines restarted on rapid hits so they layer without drifting from origin.
- Inspector hooks use `UnityEvent`; code-side events use `event Action<...>`.
- Cached `Collider[]` and `Renderer[]` from `GetComponentsInChildren` power the respawn path with no runtime lookups.
- Drop integration via `DestructibleDropSpawner`, a `[RequireComponent(typeof(Destructible))]` companion that subscribes in `OnEnable` and unsubscribes in `OnDisable`.

### Unreal Engine (C++)
- `ADestructibleActor` extends `AActor` and implements `IDestructible`, a `UINTERFACE` with `BlueprintNativeEvent` methods (`TakeHit`, `ForceDestroy`, `GetDestructibleData`).
- `UDestructibleData` and `UHitWeaponData` extend `UDataAsset`. Type-level config lives on `UDestructibleData`; placement-level overrides (respawnable, spawn destroyed actor, delays, offset) live on `ADestructibleActor`.
- Hit detection uses a `ECC_Visibility` line trace from `AFP_Character`'s camera, gated on `bCanHitFlag` and `ActiveWeapon`. Fully separate from the interaction trace.
- Pop runs as a tick-based state machine (scale-up phase, scale-down phase) instead of coroutines.
- Events use `DECLARE_DYNAMIC_MULTICAST_DELEGATE` (`BlueprintAssignable`), replacing Unity's two-layer pattern with a single delegate type for both Blueprint and C++.
- Respawn uses `SetActorHiddenInGame` and `SetActorEnableCollision` instead of toggling cached component arrays. `FTimerHandle` drives the auto-respawn delay.
- One-shot destruction uses `SetLifeSpan` (equivalent to Unity's `Destroy(gameObject, delay)`).
- Drop integration via `UDestructibleDropSpawnerComponent`, an opt-in `UActorComponent` that binds to `OnHit` and `OnDestroy` in `BeginPlay` and calls `UDropManagerSubsystem::DropItem`.
- `ADroppableItem` was extended with a physics-based mode (`bPhysicsBased`) and `Settled` state to support debris drops that stay where physics leaves them. The updated files are included in this section.
- Particles via `UNiagaraFunctionLibrary::SpawnSystemAtLocation` with `ENCPoolMethod::AutoRelease`. Direction approximated from actor center to hit location, optionally randomized with `FMath::VRandCone`.
- Destructible focus tracking runs as a separate tick block in `AFP_Character`, broadcasting focus delegates so HUD widgets bind without polling.

---

## Quick Summary

- **Data-Driven**: Per-type config in SOs/DataAssets, per-instance decisions on the component/actor.
- **Decoupled Events**: Two layers in Unity (inspector + typed code), single multicast delegate in Unreal serving both Blueprint and C++.
- **Opt-In Companions**: Drops and VFX attach via components/subscribers. Remove them and the core still works.
- **Dual Destruction Paths**: One-shot or respawnable, per-instance.
- **Hit Feedback**: Pop on every hit. Unity adds directional position shake. Both support per-weapon camera shake.
- **Tier System**: Weapon and destructible tiers gate hits, supporting tool progression.
- **HUD-Ready**: Focus delegates and cooldown events on the character, no HUD coupling.

---

<p align="center">
  <strong>Author:</strong> Lucas Varela Negro<br>
  <a href="https://www.linkedin.com/in/lucas-varela-negro/" target="_blank">
    <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white" alt="LinkedIn Badge">
  </a>
</p>
