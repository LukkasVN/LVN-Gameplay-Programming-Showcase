
// Enumerator for the types of surfaces that placeable objects can be placed on. 
// Used by PlaceableItemSO to restrict placement and by ObjectPlacementManager to determine valid placement targets.
[System.Flags]
public enum PlacementSurfaceType
{
    Floor = 1 << 0,
    Wall = 1 << 1,
    Ceiling = 1 << 2,
    Everything = Floor | Wall | Ceiling
}