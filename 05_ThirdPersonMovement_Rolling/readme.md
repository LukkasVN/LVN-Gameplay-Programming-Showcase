# 05 Third Person Movement : Rolling

Welcome to the Rolling section of the LVN Gameplay Programming Showcase series.

Building on the [**04 Sliding Module**](https://github.com/LukkasVN/LVN-Gameplay-Programming-Showcase/tree/main/04_ThirdPersonMovement_Sliding), this entry introduces a responsive and directionally-aware rolling mechanic that blends animation, movement input, and dynamic rotation.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/1017b88c-189a-4797-bca4-b853fa8fb4d8" width="600px" />
</p>

The rolling system activates when the player presses the roll key while grounded and not already performing another movement override (e.g., vault or slide).  
Once triggered, the character rolls in the direction of current movement input, with rotation smoothly interpolated to match the roll vector.  
The system supports backward and diagonal rolls, ensuring consistent game-feel and visual alignment regardless of input direction.

Capsule and mesh adjustments are synchronized with animation to allow clean transitions and prevent clipping.  
The roll duration is curve-driven, allowing for tunable momentum and animation blending.  
If the roll ends in a low-clearance area, the system can optionally transition into crouch or prone, depending on environment constraints.

<h3>Key features include:</h3>

- Roll triggered by pressing the roll key while grounded  
- Directional roll based on movement input, including backward and diagonal support  
- Smooth actor rotation blending (stable in C++ and C# implementations)  
- Capsule resizing and mesh offset synced to roll animation  
- Optional gravity override during roll for slope handling  
- Curve-driven roll speed and duration for tunable momentum  
- Automatic stance transition if roll ends in low-clearance space  
- Fully modular and non-invasive to core movement pipeline  
- Animated using Free Mixamo animations  

---

> Note: The **C++ (Unreal)** and **C# (Unity)** implementations handle rotation cleanly and predictably.  
> Extra Note: The **Blueprint prototype** became messy with rotation interpolation and yaw correction, but all other behaviour (movement, capsule resizing, timers, gravity override) works without issues.

