using System.Collections;
using System.Collections.Generic;
using UnityEngine;

// This component is added to each placed object instance by ObjectPlacementManager immediately after instantiation. [ISaveable utility]
[RequireComponent(typeof(GUIDComponent))]

// It holds a reference to the PlaceableItemSO definition for that object, which contains important data like the prefabID used for saving/loading

// IMPORTANT NOTE: PlacedObject must be in every prefab that you want to be placeable, and that prefab must be assigned to a PlaceableItemSO asset. 
// The manager relies on this for saving/loading to work correctly. 
// If a placed object is missing its PlacedObject component or reference data, it will log errors and fail to save/load properly.
public class PlacedObject : MonoBehaviour, ISaveable
{
    public PlaceableItemSO Definition { get; private set; }

    private Dictionary<int, Material[]> _originalMaterials = new();

    private Vector3 _snapshotPosition;
    private Quaternion _snapshotRotation;

    public bool MarkedForRemoval { get; private set; }

    // Called by ObjectPlacementManager immediately after instantiating a new placed object.
    public void Initialize(PlaceableItemSO definition)
    {
        Definition = definition;
        CacheMaterials();
    }

    private void CacheMaterials()
    {
        _originalMaterials.Clear();
        foreach (Renderer r in GetComponentsInChildren<Renderer>())
            _originalMaterials[r.GetInstanceID()] = r.sharedMaterials;
    }

    // Restore the original materials cached at spawn time. Used by Edit mode hover feedback.
    public void RestoreMaterials()
    {
        foreach (Renderer r in GetComponentsInChildren<Renderer>())
        {
            if (_originalMaterials.TryGetValue(r.GetInstanceID(), out Material[] mats))
                r.materials = mats;
        }
    }

    public void SnapshotForEdit()
    {
        _snapshotPosition = transform.position;
        _snapshotRotation = transform.rotation;
    }

    public void RevertToSnapshot()
    {
        transform.position = _snapshotPosition;
        transform.rotation = _snapshotRotation;
    }

    public void MarkForRemoval(bool value) => MarkedForRemoval = value;


    // ISaveable implementation

    public object CaptureState()
    {
        return new PlacedObjectSaveData
        {
            prefabID = Definition != null ? Definition.prefabID : string.Empty,
            position = transform.position,
            rotation = transform.rotation,
            scale = transform.localScale
        };
    }

    public void RestoreState(object state)
    {
        PlacedObjectSaveData data = (PlacedObjectSaveData)state;
        StartCoroutine(ApplyAfterPhysics(data));
    }

    private IEnumerator ApplyAfterPhysics(PlacedObjectSaveData data)
    {
        yield return new WaitForFixedUpdate();

        Rigidbody rb = GetComponent<Rigidbody>();
        if (rb != null)
        {
            rb.isKinematic = true;
            transform.position = data.position;
            transform.rotation = data.rotation;
            transform.localScale = data.scale;
            yield return null;
            rb.isKinematic = false;
        }
        else
        {
            transform.position = data.position;
            transform.rotation = data.rotation;
            transform.localScale = data.scale;
        }
    }
}