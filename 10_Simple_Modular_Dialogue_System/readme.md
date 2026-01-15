# 10 Simple Modular Dialogue System

Welcome to the Simple Modular Dialogue System section of the LVN Gameplay Programming Showcase series.

This module introduces a lightweight, inspector‑friendly dialogue architecture implemented across **Unity (C#)**, **Unreal C++**, and **Unreal Blueprints**.  
The goal is to provide a clean, modular, non‑branching dialogue flow that supports Locked, Base, and Used dialogue types with minimal overhead.

This section is tested using the interaction system from the [**Section 09 Modular Interactable System**](https://github.com/LukkasVN/LVN-Gameplay-Programming-Showcase/tree/main/09_Modular_Interactable_System),
which supplies the trigger and interaction events used to start, advance, and interrupt dialogue.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/dff7f5c1-af2b-404a-953c-f9616029d773" width="600px" />
</p>

## Unreal Implementation Note

Unlike previous sections, this module does **not** provide separate “C++ only” and “Blueprint only” Unreal implementations.  
Instead, both approaches are intentionally combined:

- Dialogue state, sequencing, and flag logic are implemented in **C++**.  
- UI handling and visual responses are implemented in **Blueprints**.

This hybrid structure keeps the system modular while avoiding unnecessary duplication.  
It also simplifies UI development, since Blueprint widgets can react directly to C++‑exposed flags without requiring parallel versions of the system.

> IMPORTANT NOTES:
>> 1. This system is intentionally simple and linear. It focuses on clean sequencing rather than branching dialogue or decision trees.  
>>
>> 2. Unity uses an event‑driven approach (UnityEvents) to keep the workflow modular and designer‑friendly.  
>>
>> 3. Unreal uses a mixed C++/Blueprint workflow: C++ manages dialogue logic, while Blueprints handle UI reactions.  
>>
>> 4. Dialogue type selection (Locked, Base, Used) follows the same rules across all engines.  
>>
>> 5. The system integrates directly with the Modular Interactable System (Section 09), allowing interactables to trigger dialogue consistently.

Across Unity, Unreal C++, and Unreal Blueprints, the system revolves around three dialogue phases:

- Start Dialogue (For each of the three types [Locked, Base and Used])  
- Advance Dialogue  
- End (For each of the three types [Locked, Base and Used]) or Force‑End Dialogue

The dialogue component manages state and sequencing, while UI elements respond to start/end events or flags.

## Key features include:

- Unified dialogue flow across Unity, Unreal C++, and Unreal Blueprints  
- Simple, linear dialogue sequencing with Locked/Base/Used variations  
- Inspector‑driven configuration for designers in all engines  
- Flag‑based event system in Unreal for clean UI reactions  
- UnityEvents‑based system for modular behaviour in Unity  
- Clear separation between dialogue logic and UI logic  
- Fully compatible with the Modular Interactable System (Section 09)  
- Easy to extend with custom UI, animations, or interaction rules
