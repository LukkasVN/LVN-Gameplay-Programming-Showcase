using UnityEngine;
// This ScriptableObject defines the data for each type of placeable item in the game.
// Each placeable prefab must have a corresponding PlaceableItemSO asset that contains its prefab reference and unique prefabID string used for saving/loading.
[CreateAssetMenu(fileName = "PlaceableItemSO", menuName = "Scriptable Objects/PlaceableItemSO")]
public class PlaceableItemSO : ScriptableObject
{
    [Header("Identity")]
    [Tooltip("Unique string ID — must match across save files. Never change after shipping saves.")]
    public string prefabID;

    [Header("Display")]
    public string displayName;
    public Sprite menuIcon;

    [Header("Prefab")]
    public GameObject prefab;

    [Header("Placement Rules")]
    [Tooltip("Which surface types this item can be placed on. Supports any combination.")]
    public PlacementSurfaceType allowedSurfaces = PlacementSurfaceType.Everything;

    [Header("Rotation")]
    [Range(1f, 90f)]
    [Tooltip("Degrees added per rotate input press.")]
    public float rotationStep = 45f;

    [Tooltip(
        "When enabled, the object's base rotation snaps to the surface normal on every frame " +
        "(floor → upright, wall → flush against it, ceiling → inverted). " +
        "The rotate input then adds an offset ON TOP of that snapped base, so the player " +
        "can still spin the object around the surface normal axis. " +
        "When disabled, rotation is purely world-Y and the surface normal is only used " +
        "to position the pivot point.")]
    public bool snapRotationToSurface = true;
}
