# 17 Multi-Floor Elevator System

Welcome to the **Multi-Floor Elevator System** section of the LVN Gameplay Programming Showcase series.

This module implements a robust, state-machine-driven elevator capable of handling multiple floors, automated 2-floor logic, and physical door synchronization. It specifically addresses "platform jitter" and "phasing" by taking direct control of the player's movement states during transit.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/6e439657-7de2-4c2a-8e23-cdb6d98e1787" width="600px" />
</p>

---

## Core Features

- **State-Driven Logic**
  - Explicit states: Idle, Opening, Closing, Moving, and Rotating.
  - Queue-based floor navigation for multi-floor support.
  - Automatic "Return to Close" logic when occupants depart.

- **Dynamic Door Synchronization**
  - Designer-configurable door speeds and slide distances.
  - Interruption safety: Entering during an opening sequence queues movement instead of breaking logic.
  - External call flag: Doors stay open when called via button, but auto-close after player entry/exit.

- **Occupant Stability**
  - Tag-based occupancy tracking for precise state changes.
  - Selective jump disabling via `SetCanJump(false)` to maintain grounding during vertical travel.
  - Physics interaction toggling to prevent capsule-to-object friction and clipping inside the cabin.

- **Two-Floor Auto-Mode**
  - Smart detection: If only two floors are defined, the elevator acts as a toggle.
  - Parametric cooldowns for both departure-closing and entry-moving.

---

## Engine Differences

### Unity
Unity’s Transform hierarchy and Rigidbody system provide a "global" approach to platform physics:

- **Universal Physics Handling**: Unity handles the physics of **ALL** objects (props, crates, and players) inside the elevator naturally. Parenting any object to the elevator transform ensures it inherits the platform's velocity perfectly.
- Simple `OnTriggerEnter` parenting logic.
- Door animation via `Vector3.SmoothDamp` or `LeanTween`.

---

### Unreal Engine
Unreal requires explicit management of character grounding due to the specialized nature of the `CharacterMovementComponent`:

- **Player Fixation**: Unlike Unity, Unreal is highly optimized to fixate the **Player** to movement, while secondary physics objects (props) do not automatically move with the platform without extra implementation.
- **Physics Tuning**: Requires explicit disabling of `bEnablePhysicsInteraction` to prevent the character capsule from clipping through floors when colliding with internal props during movement.
- **Post-Update Synchronization**: Uses `TG_PostUpdateWork` to ensure the elevator moves after the character’s local movement update, eliminating visual jitter.
- **Direct Input Control**: Utilizes specific character methods like `SetCanJump()` to prevent jump-induced phasing during high-velocity floor updates.

---

## Quick Summary

The **Multi-Floor Elevator System** provides a professional-grade vertical transport solution. While **Unity handles physics of all objects globally** through its transform hierarchy, **Unreal provides a more granular approach**, where the player is well-fixated to movements while props and secondary physics are handled independently.

Despite these engine quirks, the system remains:

- State-machine resilient
- Immune to "infinite loop" logic traps
- Resilient to jump-phasing and physical-object clipping
