using UnityEngine;

[CreateAssetMenu(fileName = "DroppableScatterSettings", menuName = "Scriptable Objects/DroppableScatterSettings")]
public class DroppableScatterSettings : ScriptableObject
{
    [Header("Scatter Effect")]
    public bool enabled = true;
    public float forceMin = 3f;
    public float forceMax = 8f;
    public float upForce = 2f;
    public float spinForceMultiplier = 0.3f;

    // Default instance if no SO is assigned
    private static DroppableScatterSettings defaultSettings;

    public static DroppableScatterSettings GetOrDefault()
    {
        if (defaultSettings == null)
        {
            defaultSettings = CreateInstance<DroppableScatterSettings>();
            defaultSettings.enabled = true;
            defaultSettings.forceMin = 3f;
            defaultSettings.forceMax = 8f;
            defaultSettings.upForce = 2f;
            defaultSettings.spinForceMultiplier = 0.3f;
        }
        return defaultSettings;
    }
}
