# 09 Modular Interactable System

Welcome to the Modular Interactable System section of the LVN Gameplay Programming Showcase series.

This module presents a unified interaction architecture implemented across **Unity (C#)**, **Unreal C++**, and **Unreal Blueprints**.  
While each engine approaches the problem differently, the core philosophy remains the same:  
a clean, modular, inspector‑friendly system where interactables define their own behaviour with minimal coupling to player logic.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/dc48cecd-a9d1-4e77-85b8-1cbeaf8754da" width="600px" />
</p>

> IMPORTANT NOTES:
>> 1. All versions share the same conceptual structure: a **Base Interactable** defines the interaction flow, and custom behaviour is plugged into the four core events — Hover Enter, Hover Exit, Hover Stay, and Interact.  
>>
>> 2. The Unity version is strongly **event‑driven**, using UnityEvents to keep the system highly modular and designer‑friendly. Inheritance is supported, but the emphasis is on flexible event binding rather than subclassing.  
>>
>> 3. The Unreal versions (C++ and Blueprints) follow a more traditional **inheritance‑driven** approach, where child classes override the base interaction methods directly.  
>>
>> 4. While a single player‑driven raycast is objectively more efficient for click‑hover and click‑interaction, this system is not built around that model.  
Across Unity, Unreal C++, and Unreal Blueprints, the architecture is intentionally **interactable‑driven**: each interactable manages its own detection and state.  
>>
>> 5. The example behaviours (physics impulse, spinning text, look‑at‑camera) simply demonstrate extensibility — the architecture itself is the focus.

Across all three engines, the system revolves around four core interaction events:

- **Hover Enter**  
- **Hover Exit**  
- **Hover Stay**  
- **Interact**

Each implementation handles these events differently, but the workflow remains consistent:  
the base system manages detection and state, while user‑defined logic is attached or overridden depending on the engine.

## Key features include:

- Unified interaction lifecycle across Unity, Unreal C++, and Unreal Blueprints  
- **Event‑driven design in Unity**, enabling maximum modularity and inspector‑based configuration  
- **Inheritance‑driven design in Unreal**, allowing clean overrides in both C++ and Blueprints  
- Minimal dependency on player scripts so the interactables manage their own logic  
- Automatic trigger/collider handling across all versions  
- Architecture‑focused design, independent of specific behaviours  
- Easy to extend with custom interaction responses in both engines
