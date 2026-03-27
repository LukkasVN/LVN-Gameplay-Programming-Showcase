# 20. Serializable Object Placing System

Welcome to the **Serializable Object Placing System**, a core gameplay mechanic in the LVN Gameplay Programming Showcase series.

This system implements a first-person object placement experience with full save integration. Players can place, edit, reposition, and remove objects at runtime across any surface type specified. 
All placed objects are serialized and persist across sessions through an updated version of the existing save system from Section 12.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/00dc00c9-b53d-427d-b82b-699384a31b15" width="800px" />
</p>

---

## Important Clarification

**This system requires an updated version the Modular Save System from Section 12 to function** (It is inside this section's files). The placement system itself owns no file I/O, because it delegates all saving and loading to the existing save infrastructure. The two systems are designed to work together but remain decoupled in responsibility.

⚠️ **Important UNITY Note**: All `PlaceableItemSO` ScriptableObject assets **must be placed inside `Assets/Resources/PlaceableItems/`** for the manager to auto-discover them at runtime. Unity's `Resources.LoadAll` API can only load assets from folders explicitly named `Resources`. Assets placed anywhere else will not be found automatically.

---

## Core Features

- **Four Placement Modes**
  - **None** --> Idle, no placement active.
  - **Placing** --> A ghost preview follows the player's aim. Confirm to place, cancel to exit.
  - **Editing** --> Aim at a placed object to select it, then reposition it freely. Cancel reverts it to its original position.
  - **Removing** --> Aim at a placed object and confirm to stage it for removal. Removal is only committed when the player saves.

- **Surface Validation & Preview Feedback**
  - Each item defines which surfaces it can be placed on: Floor, Wall, Ceiling, or any combination.
  - The preview material changes color to indicate valid or invalid placement in real time.

- **Surface Snapping & Rotation**
  - **Snap to Surface** --> The object aligns its base to the surface normal. Useful for wall shelves, ceiling lights, and similar items. Rotation input spins around that normal axis.
  - **Free Rotation** --> The object keeps world-Y rotation regardless of surface. Rotation input adds degrees on the vertical axis.

- **Save Integration**
  - Placed objects are serialized through `PlacedObjectComponent` / `PlacedObject` which implement the saveable interface.
  - Removal is non-destructive until save, so this way staged objects are hidden but not destroyed, allowing the player to reload and recover them.
  - Clearing all placed objects and then saving correctly writes an empty state to disk, removing previously saved entries.

---

## Engine Differences

### Unity (C#)
- `ObjectPlacementManager` is a `MonoBehaviour` attached to a scene object or the player (Not Recommended).
- Item definitions use `ScriptableObject` (`PlaceableItemSO`) auto-loaded from `Resources/PlaceableItems/`.
- Overlap validation uses `Physics.OverlapBox` filtered to the `placedObjectLayerMask` only, so the placement surface layer is intentionally excluded to avoid false positives.
- `SaveManager` owns all file I/O and calls into the placement manager on save and load.

### Unreal Engine (C++)
- `UObjectPlacementManager` is an `ActorComponent` attached to the player pawn directly by code.
- Item definitions use `UDataAsset` (`UPlaceableItemData`) auto-discovered at runtime via the Asset Registry.
- Overlap validation uses `OverlapMultiByChannel` on a dedicated `PlacedObjectChannel` only.
- `USaveManagerSubsystem` (GameInstance subsystem) owns all file I/O and calls into the placement manager on save and load.

---

## Quick Summary

The **Serializable Object Placing System** delivers a full housing-style placement loop: Place, edit, remove, save, load along with correct surface alignment, overlap validation, and persistent state across sessions.

- **Multi-Mode**: Distinct placing, editing, and removing workflows with clear player feedback.
- **Surface-Aware**: Per-item surface restrictions with snap-to-surface or free rotation.
- **Non-Destructive Removal**: Staged removals are only committed on save, allowing reload-based undo.
- **Save-Integrated**: Plugs directly into the save system with no file I/O ownership of its own.
- **Engine-Aware**: Tailored solutions for each engine's object lifecycle and asset management differences.
