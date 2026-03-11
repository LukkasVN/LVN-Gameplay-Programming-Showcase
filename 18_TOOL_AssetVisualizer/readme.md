# 18 Asset Grid Visualizer [TOOL]

Welcome to the **Asset Grid Visualizer**, a not so "Gameplay" section of the LVN Gameplay Programming Showcase series.

This module implements a flexible, data-driven grid system for organizing and visualizing assets in the editor. It provides an intuitive way to arrange and preview your game assets in a structured layout, making it easier to organize, and check your game's visual elements without cluttering the main scene.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/c5243e3d-717d-4817-bc64-0e7c16f8eacd" width="600px" />
</p>

---

## Core Features

- **Grid Based Asset Organization**
  - Multi grid support with configurable horizontal and vertical spacing.
  - Row based layout system for precise asset placement.
  - Per asset scale, rotation, and position offset control.

- **Pure C++ Implementation**
  - Zero Blueprint dependencies, fully data driven structs.
  - Editor integrated property editing via UPROPERTY and USTRUCT.
  - Lightweight and performant asset organization.

- **Flexible Asset Placement**
  - Configurable separation between assets (HorizontalSeparation: and VerticalSeparation).
  - Grid to grid spacing control (GridSeparation).
  - Per asset customization: scale multiplier, rotation, and positional offset that adapts to the grid.

- **GameObject/Actor Lifecycle Management**
  - Reset Script button for safe grid reset and asset destruction.
  - Refresh Layout button and ContexMenu method for immediate layout recalculation.

---

## Engine Differences

### Unity
Unity provides real time updates for grid organization with seamless asset preview:

- **Real Time Updates**: All property changes to scale, rotation, offset, and spacing reflect immediately in the scene without manual refresh.
- **Prefab Support**: The system accepts both meshes and prefabs for maximum flexibility in asset organization.
- **Instant Inspector Updates**: Changes in the inspector update the grid layout instantly, allowing fast iteration and tweaking.
- **Purpose**: Organize and preview your game assets in a clean, structured grid view for design review and asset management.

---

### Unreal Engine
Unreal requires manual refresh and focuses exclusively on static mesh organization:

- **Manual Refresh Required**: Every property change requires clicking the Refresh Layout button to update the grid.
- **Static Meshes Only**: The system only accepts UStaticMesh assets. Blueprints are not supported, keeping the scope focused and the system lightweight.
- **Editor Mode Toggle**: The bEnabledInEditor boolean controls whether the grid is active in the editor. When disabled, the grid does not render or take up scene space.
- **Purpose**: Organize and preview your game's static mesh assets in a clean, structured grid view for design review, asset management, and quality checking without cluttering your main level.

---

## Quick Summary

The **Asset Grid Visualizer** is a tool designed to organize and check your game assets in a comfortable, structured way. It keeps your work organized while allowing you to preview and tweak asset placement, scale, and rotation all in one view.

While Unity provides real time updates and supports both meshes and prefabs, Unreal requires manual Refresh Layout clicks and accepts only static meshes. Despite these differences, the system remains:

- Lightweight and focused on asset organization
- Easy to use with configurable spacing and per asset controls
- Safe and reliable with automatic cleanup
- Perfect for design review, asset cataloging, and visual quality checking
