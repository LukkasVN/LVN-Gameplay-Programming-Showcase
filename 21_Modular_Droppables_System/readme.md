# 21. Modular Droppables System

Welcome to the **Modular Droppables System**, a core gameplay mechanic in the LVN Gameplay Programming Showcase series.

This system implements a fully data-driven item drop experience with physics-based scatter, smooth float behaviour, and trigger-based collection. Items can be spawned at runtime through a manager or placed directly in the scene as standalone actors, with both paths fully supported and independent.

<h2 align="center">Overview</h2>

<p align="center">
  <img width="800" alt="Unity and Unreal Cpp + Blueprints Modular Droppables System by LucasVN_Gif" src="https://github.com/user-attachments/assets/dd6073dd-a16b-466b-8086-6d7757445496" />
</p>

---

## Important Clarification

**The core of this system is `DroppableItem` and `DropManager` (Unity) / `UDropManagerSubsystem` (Unreal).** These form a self-contained, data-driven drop system that requires no code modification to create new item types, so all behaviour is configured through ScriptableObjects (Unity) or DataAssets (Unreal).

Everything else in this section is example scaffolding to demonstrate the system in action:

- **`ItemSpawner`** is a small utility showing one way to call into the manager. It is not part of the core system because in a real project this would be replaced by whatever triggers drops in your game (enemy death, chest opening, quest reward, etc). For the in-scene showcase, item spawning is triggered via interactable objects built using the [**Modular Interactable System from Section 09**](https://github.com/LukkasVN/LVN-Gameplay-Programming-Showcase/tree/main/09_Modular_Interactable_System).
- **The UI handling** (`UIManager` calls, `exampleType` enum) is an example showing collection results reflected in a simple UI counter. The correct extension point for your own logic is the `onCollectedEventHook` UnityEvent (Unity) or `OnCollectedEvent` delegate (Unreal). Feel free to bind your own system there and discard the example entirely.

**Unity Note**: Items placed directly in the scene require a `DroppableItemData` ScriptableObject assigned in the Inspector. Pool-spawned items receive their data at runtime from the `DropManager`.

**Unreal Note**: For items spawned via the subsystem, the `UDroppableItemData` asset must have its `ItemClass` field pointing to the correct Blueprint actor class. Items placed directly in the level assign `ItemData` via the Details panel.

**BOTH ENGINES IMPORTANT NOTE**: Make sure to properly configure the collision masks between droppables, spawners, entities, and the player so they ignore each other when necessary. Otherwise, you may encounter strange and unintended behaviors, such as objects and the player flying into space when collecting a coin.

---

## Core Features

- **Two Usage Paths**
  - **Manager-Spawned** → Items are spawned at runtime via `DropManager` / `UDropManagerSubsystem`. Quantity is randomised from the item's defined range. Destroyed on collection.
  - **Scene-Placed** → Items placed directly in the level with `ItemData` assigned in the Inspector or Details panel. Fully independent with no manager required. Destroyed on collection (You can also add it to an existing pool adding new logic to the DropManager instead of destroying it).

- **Physics-Based Scatter**
  - On spawn (If enabled), items receive a randomised directional impulse with an upward bias and random spin, creating a natural scatter effect.
  - Scatter force, up force, and spin multiplier are configurable per item via `DroppableScatterSettings` / `UDroppableScatterSettings`.
  - Scene-placed items skip scatter entirely and boot straight into float.

- **Settle Detection**
  - After a configurable collect delay, the system waits for the rigidbody to come to rest on a valid upward-facing surface before making the item collectible.
  - A timeout failsafe forces collectible state if the item never fully settles, preventing permanently uncollectable items.

- **Smooth Float Behaviour**
  - Once collectible (And if enabled to), items smoothly lerp from their resting position to a configurable hover height above the ground contact point.
  - A sine wave bob and continuous yaw rotation run during the float idle state.
  - Float height, bob amplitude, bob speed, rotation speed, and transition duration are all configurable via `DroppableFloatSettings` / `UDroppableFloatSettings`.

- **Trigger-Based Collection**
  - A dedicated trigger sphere overlaps with the player. The physics collider explicitly ignores pawns so the player never physically pushes the item.
  - Collection only activates during valid float states, preventing premature pickup during scatter or settle.
  - On collection, items smoothly move toward the player with a configurable speed and a slight upward offset.

- **Pop Effect**
  - A brief scale-up and scale-back animation plays at the moment of collection (If enabled).
  - Configurable scale multiplier and duration via `DroppablePopSettings` / `UDroppablePopSettings`.

- **Modular Settings**
  - Scatter, Pop, and Float behaviours are each defined in separate ScriptableObjects (Unity) or DataAssets (Unreal).
  - Any settings object can be shared across multiple item types or overridden per item.
  - If no settings object is assigned, sensible defaults are applied automatically at runtime.
  - New item types require zero code — create a data asset, assign settings, done.

- **Collection Event Hook**
  - A `UnityEvent` (Unity) / `BlueprintAssignable` delegate (Unreal) fires on collection.
  - This is the intended extension point for inventory systems, UI updates, audio, achievements, or any downstream logic — without touching the base class.

---

## Engine Differences

### Unity (C#)
- `DropManager` is a singleton `MonoBehaviour` handling pool-based spawning, reusing instances rather than instantiating and destroying them.
- Item definitions use `ScriptableObject` (`DroppableItemData`) extending a base `ItemSO`.
- Settle detection uses `Rigidbody` velocity magnitude and `OnCollisionEnter` contact normal checks.
- Float and rotation are driven by coroutines transitioning into per-frame `Update` logic.
- The player reference is received directly from the trigger overlap, so this way `DroppableItem` has no singleton dependency of its own.
- Pre-placed items initialise through `Start()`, bootstrapping straight to float without scatter.

### Unreal Engine (C++)
- `UDropManagerSubsystem` is a `UWorldSubsystem` handling runtime spawning via `SpawnActor` directly with no pooling, as Unreal's spawn cost makes it unnecessary for this use case.
- Item definitions use `UDataAsset` (`UDroppableItemData`) extending a base `UItemData`.
- Settle detection uses `OnComponentHit` impact normal checks and physics linear velocity polling each tick.
- Float and rotation are driven by an explicit `EDropState` enum state machine evaluated each `Tick`, with `FTimerHandle` replacing coroutine delays.
- Yaw rotation is driven by an explicit tracked float to prevent pitch and roll drift from physics accumulation.
- Physics constraints lock X and Y rotation at the body instance level, ensuring clean yaw-only spin during scatter.
- Pre-placed items initialise through `BeginPlay()`, bootstrapping straight to float without scatter.

---

## Quick Summary

The **Modular Droppables System** delivers a complete item drop loop: spawn, scatter, settle, float, and collect, everything configured entirely through data assets with no code modification required for new item types.

- **Data-Driven**: All item behaviour is defined in ScriptableObjects or DataAssets so new items need zero code.
- **Modular**: Scatter, Pop, and Float are independently configurable and shareable across item types.
- **Dual-Path**: Items work both as runtime-spawned drops and as manually placed scene actors.
- **Physics-Aware**: Settle detection waits for the item to actually land before transitioning to float, with a timeout failsafe.
- **Non-Invasive Collection**: Trigger-based pickup with explicit pawn ignore on the physics collider.
- **Expandable**: Collection event hook is the clean integration point for any downstream system like inventory, UI, audio, quests...
- **Engine-Aware**: Tailored solutions for each engine's physics, lifecycle, and asset management differences.

---

<p align="center">
  <strong>Author:</strong> Lucas Varela Negro<br>
  <a href="https://www.linkedin.com/in/lucas-varela-negro/" target="_blank">
    <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white" alt="LinkedIn Badge">
  </a>
</p>
