# 19. Radio Station Manager System

Welcome to the **Radio Station Manager System**, a core gameplay mechanic in the LVN Gameplay Programming Showcase series.

This system implements a persistent, synchronized in-game radio broadcast. All radios tuned to the same station play identical content, and if a radio is turned off and back on within a time window, the broadcast continues from where it left off. Simulating a real radio always broadcasting in the background.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/c3616459-177f-4120-b056-a7794413bcc9" width="800px" />
</p>

---

## Important Clarification

**This system does NOT support or include any functionality for connecting to actual radio stations.** There is no code dedicated to streaming real-world radio broadcasts, APIs for station connectivity, or internet-based audio feeds. This is purely a **local, in-game radio simulation** that plays pre-loaded audio files from your project.

⚠️ **Testing Notice**: This system has been developed and tested in the editor only. It has **not been tested in built games**. Compatibility and behavior in packaged builds may differ and should be verified before production use.

---

## Core Features

- **Synchronized Playback**
  - All radios on the same station broadcast identically in real time.
  - Central manager tracks station state and playback position.
  - Every radio audiosource playing a station receives the same song at the same playback position.

- **Persistent Broadcast State**
  - Radios maintain broadcast time even when turned off.
  - Rejoining a station resumes at the correct position, never restarting.

- **FP_interactable Integration**
  - Radio menu triggered via `FP_interactable` using the FP_Controller for seamless interaction workflow.

---

## Engine Differences

### Unity (C#)
- **Direct local audio file loading** via AudioClip system (may experience lag spikes on load; optimize with async methods).
- **Real-time synchronization** with instant updates across all active radio sources.
- Uses AudioSource components for flexible playback control.
- **Purpose**: Create immersive, persistent radio broadcasts integrated into the game world.

### Unreal Engine (C++)
- **⚠️ Local Audio File Loading Challenge ⚠️** without middleware:
  - Pre-loads audio files in the RadioStationManager subsystem.
  - Uses native USoundWave and UAudioComponent for synchronization.
  - Demonstrates an over-engineered but necessary engine-native workaround within Unreal's constraints (supports .wav format only via this approach).
  - Synchronizes multiple audio components to a shared broadcast timeline.

- **Purpose**: Show how to deliver cohesive radio experiences despite Unreal's audio file loading limitations.

---

## Quick Summary

The **Radio Station Manager System** brings persistent, synchronized radio broadcasts with your own local songs to life by maintaining a shared playback timeline across all radios in the scene. Whether turning radios on, off, or switching stations, the system maintains the illusion of a constantly-running broadcast.

- **Synchronized**: All radios on the same station play identically.
- **Persistent**: Broadcast state continues when radios are off.
- **Interaction-Driven**: Uses `FP_interactable` for seamless menu access.
- **Engine-Aware**: Tailored solutions for each engine's audio capabilities and limitations.
