# 12 Modular Save System

Welcome to the Modular Save System section of the LVN Gameplay Programming Showcase series.

This module introduces a clean, component‑driven architecture for saving and loading game state across Unity and Unreal Engine.  
The system focuses on modularity, clarity, and extensibility, allowing any gameplay behaviour to define its own save logic without modifying the core framework.

The core idea is simple: every saveable object has a stable GUID, and every saveable behaviour exposes its own data.  
The SaveManager collects these components, serializes their data as JSON, and restores them by matching GUIDs.

> Note: I use my Modular Interactable system to test the Load,Save,Delete and Reset methods.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/6022b23f-5b0c-469e-8619-f540627cfc97" width="600px" />
</p>

---

## System Overview

The system is composed of two main parts:

- **GUID Component**  
- **Saveable Components**

The GUID component gives each object a permanent unique identifier.  
Saveable components each handle one specific type of data, such as transforms, physics, inventory, puzzle state, or anything else you choose to support.

### The SaveManager:

- Finds all saveable components in the scene  
- Captures their data into JSON  
- Stores the data alongside the object's GUID and data type  
- Restores the data by passing it back to the component that created it  

The SaveManager never needs to know what the data actually is.

---

## Adding a New Saveable System

The workflow is identical in Unity and Unreal, with only syntax differences.

### 1. Create a small data class  
Define a simple data container that represents exactly what you want to save.

### 2. Create a saveable component  
Implement the logic to capture the current state into your data class and restore it later.  
Each behaviour gets its own saveable component, keeping responsibilities isolated.

### 3. Add both components to the object  
Add the GUID component and your saveable component.  
The SaveManager will automatically detect it and include it in the save file.

---

## Why This Works

Each saved entry contains:

- The GUID of the object  
- The JSON string of the saved data  
- The type name of the data  

On load:

- The system finds the object by GUID  
- Reconstructs the correct data type  
- Passes the data back to the component that originally created it  

This keeps the architecture clean, modular, and infinitely expandable.

---

## Mental Model

Whenever you want to save something new:

- Make a data class  
- Make a saveable component  
- Add it to the object  

That’s the entire system.
