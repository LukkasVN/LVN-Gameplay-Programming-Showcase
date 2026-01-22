# 11 Modular Trigger-Based Puzzle System

Welcome to the Modular Trigger-Based Puzzle System section of the LVN Gameplay Programming Showcase series.

This module introduces a **clean, event-driven puzzle architecture** built around trigger volumes and tag validation, designed to be modular, reusable, and inspector-friendly.  
The system focuses on clarity and composability rather than complexity, allowing puzzle logic to be assembled without hard-wiring dependencies.

The core idea is simple: individual triggers detect objects entering and exiting, optionally validate them by tag, and expose UnityEvents that can be used standalone or combined through a central controller.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/4af59324-b7e6-4005-9fa0-71e75b3265c9" width="600px" />
</p>

## System Overview

The system is composed of two main parts:

- **PuzzleTrigger**  
- **PuzzleController** (optional)

Each `PuzzleTrigger` can work completely on its own, reacting to trigger enter and exit events.  
When multiple triggers need to be combined into a single puzzle condition, they can all reference the same `PuzzleController`, which evaluates the global puzzle state.

No polling, no update loops, and no hard-wired logic chains.

## PuzzleTrigger Behaviour

Each `PuzzleTrigger` exposes four UnityEvents:

- `onAnyEnter`  
- `onAnyExit`  
- `onTagEnter`  
- `onTagExit`  

The behaviour follows these rules:

- **onAnyEnter / onAnyExit**  
  Fired whenever *any* object enters or exits the trigger.

- **onTagEnter / onTagExit**  
  Fired only when the entering or exiting object matches a specified tag.

### Empty-Trigger Mode

A configurable boolean allows the trigger to operate in two modes:

- **Normal mode**  
  Events fire on every enter and exit.

- **Only-When-Empty mode**  
  Events fire only when:
  - The first object enters an empty trigger.
  - The last object exits, leaving the trigger empty.

This mode applies to **all events**, tagged and non-tagged alike, without interfering with tag-based activation logic.

### Activation State

Each trigger maintains a simple `isActivated` state that depends solely on tagged objects.  
If at least one correctly tagged object is inside the trigger, it is considered activated.

This activation state is intentionally decoupled from event gating, keeping behaviour predictable and modular.

## PuzzleController Behaviour

The `PuzzleController` is an optional layer that enables multi-trigger puzzles.

Its responsibilities are minimal:

- Collect references to multiple `PuzzleTrigger` instances.  
- Check whether all registered triggers are currently activated.  
- Fire a single event when the puzzle conditions are met.

Triggers do not need to know about each other.  
They simply report their activation state when it changes.

To use it:

1. Add a `PuzzleController` to the scene.  
2. Reference it from any number of `PuzzleTrigger` components.  
3. Configure the final puzzle event once, in one place.

No additional wiring is required.

## Key Features

- Fully modular trigger-based puzzle architecture  
- Works standalone or as part of a multi-trigger puzzle  
- Event-driven design using UnityEvents in Unity and Blueprint Events in Unreal
- Clear separation between trigger logic and puzzle logic  
- Optional empty-trigger gating without breaking modularity  
- Tag-based activation for precise puzzle conditions  
- No polling, no update loops, no hard dependencies  
- Inspector-friendly and designer-driven  
- Clean, reusable, and easy to extend  

This system is intentionally small in scope, focusing on **clarity, flexibility, and reuse**, making it ideal for fast prototyping and puzzle mechanics.
