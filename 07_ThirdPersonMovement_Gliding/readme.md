# 07 Third Person Movement : Gliding

Welcome to the Gliding section of the LVN Gameplay Programming Showcase series.

Building on the [**06 Ledge-Walking Module**](https://github.com/LukkasVN/LVN-Gameplay-Programming-Showcase/tree/main/06_ThirdPersonMovement_Ledge-Walking), this entry introduces a prototype gliding mechanic that lets the player deploy a glider during freefall and control a smoother descent.  
The system is designed to remain modular, readable and fully inspector driven (with certain needed adjustments for the glider) while integrating cleanly with the existing locomotion setup.

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/6d92a964-ccef-4338-81ca-c0d9f3a6f436" width="600px" />
</p>

This mechanic uses a Glider model by **gamedevsaari** and slightly adjusted Mixamo animations to fit the gameplay context.  
Unity allows more flexible animation adjustments through the Animator in scene, while Unreal uses a more rigid but stable setup.  
Once the player meets the minimum fall conditions, the glider activates and movement transitions into a controlled glide with tweakable behaviour parameters.

<h3>Key features include:</h3>

- Glider activates automatically when minimum fall conditions are met  
- Glide gravity, speed and rotation are exposed as inspector variables  
- Minimum glide height and minimum fall speed fully adjustable  
- Clean integration with the existing locomotion system  
- Animation adjustments supported in both Unity and Unreal setups  
- Needs certain engine adjustements to properly set the glider and the animations to work properly

---

> IMPORTANT NOTE: The glider model used comes from **gamedevsaari**.  
> Model link: https://sketchfab.com/3d-models/glider-54fe8d3e61c34c288044b5876cdf68c8

> Extra Note: Due to a tighter schedule this week, only the Unity C# and Unreal C++ versions are included. The Blueprint version will return next week if everything goes smoothly, along with a new mechanic for the series.

