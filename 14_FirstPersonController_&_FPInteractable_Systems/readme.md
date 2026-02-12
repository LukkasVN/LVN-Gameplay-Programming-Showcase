# 14 First Person Controller & FP Interactable Systems

Welcome to the First Person Controller & FP Interactable Systems section of the LVN Gameplay Programming Showcase series.

This module introduces a unified first‑person controller and interaction workflow implemented in both Unity and Unreal Engine.  
The design goal is consistency across engines while respecting their differences: Unity uses hybrid C# scripts and events, while Unreal uses C++ foundations with Blueprint graphs for interaction behavior.

The core idea is straightforward:  
**The character exposes explicit ability toggles, and interactables implement a minimal interface that the character detects and triggers.**

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/3141966f-0698-484f-9789-e9c6835a5718" width="600px" />
</p>

---

### The system is composed of two main parts:

- **First Person Character Controller**  
- **FP Interactable Interface + Implementations**

The controller handles movement, look, sprinting, jumping, and interaction tracing.  
Interactables define their behavior through a simple interface with three functions: `Interact`, `OnFocus`, and `OnUnfocus`.

---

## Engine Differences

### Unity
- Uses hybrid C# scripts for both controller and interactables  
- Interaction logic is event‑driven  
- Includes coyote time 
- Pushables use Rigidbody forces directly in script  

### Unreal
- Uses C++ for the controller and a `UInterface` for interactables  
- Interaction behavior is implemented in Blueprint graphs  
- No timer‑based coyote it instead uses a timeless double‑jump fallback  
- Pushables use physics impulses and torque entirely made in the Blueprint's Graph  

---

## Creating a New Interactable

### Unity
1. Create a script implementing the interactable interface  
2. Override the interaction methods  
3. Attach it to any GameObject  

### Unreal
1. Create a Blueprint Actor  
2. Add the `FP_Interactable` interface  
3. Implement the three Blueprint events:
   - `Event Interact`
   - `Event OnFocus`
   - `Event OnUnfocus`

The character automatically detects and triggers these events.

---

## Pushable Objects

Pushables demonstrate the flexibility of the interaction system.

- Unity: implemented in C# using Rigidbody.AddForce and AddTorque  
- Unreal: implemented in Blueprint using AddImpulse and AddTorqueInRadians  
- Both versions support randomized direction, force, and torque  

---

## Quick Summary

This module provides a consistent first‑person controller and interaction workflow across Unity and Unreal.  
Unity focuses on script‑driven behavior with event hooks.  
Unreal focuses on C++ structure with Blueprint‑driven interaction logic.  
Both engines share the same conceptual design while adapting to their native workflows.
