# 06 Third Person Movement : Ledge Walking

Welcome to the Ledge Walking section of the LVN Gameplay Programming Showcase series.

Building on the [**05 Rolling Module**](https://github.com/LukkasVN/LVN-Gameplay-Programming-Showcase/tree/main/05_ThirdPersonMovement_Rolling), this entry introduces a prototype mechanic that allows the player to snap onto a ledge and move laterally along it.  
The system focuses on clean state handling for entering, traversing, and exiting ledges, while remaining modular and inspector‑driven for iteration.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/1dd9e0c2-e74f-4a4c-9b39-67c6821354ad" width="600px" />
</p>

Once the player overlaps a ledge trigger, the character snaps into position with inward and vertical offsets exposed in the inspector for easy tuning.  
Movement input drives traversal along the ledge, clamped to its bounds to prevent overshoot or teleport issues.  
The mechanic integrates with the existing locomotion setup and is designed to remain self‑contained without disrupting other modules.

<h3>Key features include:</h3>

- Snap onto a ledge trigger and move left or right along it  
- Movement clamps to ledge bounds for stability and reproducibility 
- Multiple position offsets and speed exposed as inspector variables for quick adjustments  
- Clean state handling for entering, traversing, and exiting ledge without disrupting core movement

---

> IMPORTANT NOTE: In order to work properly the Ledge  needs to be a Blueprint with both the wall mesh and as a child the floor/ledge mesh.

> Extra Note: The **Blueprint version** was skipped this week due to time constraints. On busy weeks I may leave Blueprint out to finish and refine the other two implementations. If specifically requested, I can go back and update the Blueprint version later, but otherwise I will continue forward to the next mechanic.
