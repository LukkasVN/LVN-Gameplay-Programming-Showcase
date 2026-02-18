# 15 Audio Manager System

Welcome to the **Audio Manager System** section of the LVN Gameplay Programming Showcase series.

This module presents a unified audio workflow implemented in both Unity and Unreal Engine.  
It handles music, SFX, UI sounds, 3D positional audio, and mixer‑based volume control through a consistent cross‑engine architecture.

<h2 align="center">Overview</h2>

<p align="center">
  <img src= "https://github.com/user-attachments/assets/d50b8cdb-e8a5-4309-a0a9-d72e3e1c4791" width="600px" />
</p>

---

## Core Features

- Centralized audio handling (Master, Music, SFX, UI, 3D)
- Music fade‑in, fade‑out, and crossfades
- 3D SFX pooling system to avoid having multiple audioSources attached to different GameObjects / Actors
- Mixer‑driven volume control with simple save settings

---

## Engine Differences

### Unity
Unity’s audio workflow is **almost entirely script‑driven**, allowing the system to run with minimal engine configuration:

- Audio routing via exposed `AudioMixer` parameters  
- Crossfades and fade‑outs handled in C#  
- 3D SFX pooling created at runtime  
- Volume persistence through `PlayerPrefs` 

Unity’s flexibility makes the Audio Manager System largely self‑contained and reusable.

---

### Unreal Engine
Unreal requires **more in‑engine setup** to support the same features, especially when using custom "audio groups":

- SoundClasses and SoundMixes must be created and configured in the project settings (Or at least I recommend it)  
- Attenuation assets are required for 3D audio (Audio will not be 3D if no attenuation is set)
- Audio routing depends on SoundClass hierarchies
- Music and SFX use `UAudioComponent` instances managed by a subsystem

In short:  
**Unity handles most logic in code, while Unreal relies heavily on engine‑level audio configuration.**

---

## Quick Summary

The **Audio Manager System** provides a unified audio workflow.  
Unity focuses on script‑based control, while Unreal integrates deeply with its built‑in audio pipeline.
Despite these differences, both implementations share the same design goals and runtime behavior.
