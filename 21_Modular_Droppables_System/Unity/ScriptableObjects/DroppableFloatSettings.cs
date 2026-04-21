using UnityEngine;

[CreateAssetMenu(fileName = "DroppableFloatSettings", menuName = "Scriptable Objects/DroppableFloatSettings")]
public class DroppableFloatSettings : ScriptableObject
{
    [Header("Float Effect")]
    public bool enabled = true;
    public float groundOffset = 0.5f;
    public float bobHeight = 0.25f;
    public float bobSpeed = 1.5f;
    public float rotationSpeed = 90f;
    public float transitionDuration = 0.4f;

    private static DroppableFloatSettings defaultSettings;

    public static DroppableFloatSettings GetOrDefault()
    {
        if (defaultSettings == null)
        {
            defaultSettings = CreateInstance<DroppableFloatSettings>();
            defaultSettings.enabled = true;
            defaultSettings.groundOffset = 0.5f;
            defaultSettings.bobHeight = 0.25f;
            defaultSettings.bobSpeed = 1.5f;
            defaultSettings.rotationSpeed = 90f;
            defaultSettings.transitionDuration = 0.4f;
        }
        return defaultSettings;
    }
}
