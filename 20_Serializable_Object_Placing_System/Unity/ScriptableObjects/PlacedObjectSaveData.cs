using UnityEngine;

// Data class used for saving/loading PlacedObject state. 
// Captured by PlacedObject.CaptureState() and used by ObjectPlacementManager to restore objects on load.
[System.Serializable]
public class PlacedObjectSaveData
{
    public string prefabID;
    public Vector3 position;
    public Quaternion rotation;
    public Vector3 scale;
}