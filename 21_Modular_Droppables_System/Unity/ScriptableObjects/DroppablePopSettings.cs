using UnityEngine;

[CreateAssetMenu(fileName = "DroppablePopSettings", menuName = "Scriptable Objects/DroppablePopSettings")]
public class DroppablePopSettings : ScriptableObject
{
    [Header("Pop Effect")]
    public bool enabled = true;
    public float scale = 1.5f;
    public float duration = 0.15f;

    // Default instance if no SO is assigned
    private static DroppablePopSettings defaultSettings;

    public static DroppablePopSettings GetOrDefault()
    {
        if (defaultSettings == null)
        {
            defaultSettings = CreateInstance<DroppablePopSettings>();
            defaultSettings.enabled = true;
            defaultSettings.scale = 1.5f;
            defaultSettings.duration = 0.15f;
        }
        return defaultSettings;
    }
}
