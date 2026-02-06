# 13 Modular Static Text Translation System

Welcome to the Modular Static Text Translation System section of the LVN Gameplay Programming Showcase series.

This module introduces a simple, modular approach for handling static text localization across Unity and Unreal Engine.  
The system focuses on clarity and minimal coupling: each text element stores its own localized strings, and a central Translation Manager broadcasts language changes so UI and in‑world text update automatically.

The core idea is straightforward:  
**Localized text lives with the component that displays it, and the manager only tracks the active language.**

<h2 align="center">Overview</h2>

<p align="center">
  <img src="https://github.com/user-attachments/assets/86176146-a530-4d63-b984-3f4c65b0545a"width="600px" />
</p>

---

### The system is composed of three main parts:

- **Language‑Specific Text Containers**  
- **Translation Manager**  
- **Text Components (UI or in‑world)**

Localized text containers hold the per‑language strings.  
The Translation Manager stores the current language, persists it, and notifies listeners when it changes.  
Text components subscribe to these notifications and update themselves automatically.

### The Translation Manager:

- Stores the active language  
- Loads/saves it (PlayerPrefs in Unity, config file in Unreal)  
- Broadcasts `OnLanguageChanged`  
- (Unity) Provides optional per‑language font overrides and [Multiline] attribute for translation texts  

The manager never touches UI or actors directly, it only broadcasts updates.

---

## Adding a New Translatable Text

The workflow is nearly identical in Unity and Unreal.

### 1. Define your localized text  
Fill in the per‑language fields in the `LocalizedText` (Unity) or `FLocalizedText` (Unreal) struct.

### 2. Add a translation component  
Attach the appropriate component to your UI element or actor:

### 3. Set your translated texts in each language’s field. 
Once the translation component is added, you’ll see a list of your supported languages, each with its own text box. 
Fill these fields to define the text for every language used by the attached text component.

- **Unity:** `TranslatableStaticText`  
- **Unreal:** `UTranslatableStaticTextComponent`

This components subscribe to the Translation Manager and update automatically.

### 4. Change the language at runtime  
Call:

- **Unity:** `TranslationManager.Instance.SetLanguage(lang)`  
  *(Recommended via UI Button OnClick events)*

- **Unreal:** `UTranslationManager::SetLanguage(lang)`  
  *(Recommended via Widget Blueprint bindings)*

All subscribed text elements update instantly.

---

## Quick Summary

Whenever you want a piece of text to be translatable:

- Ensure the Translation Manager is active (in‑scene for Unity, Subsystem for Unreal)  
- Add a translation component to any GameObject/UObject with a text component  
- Fill the per‑language text fields  
- Let the manager handle updates automatically  

That’s the entire system.
