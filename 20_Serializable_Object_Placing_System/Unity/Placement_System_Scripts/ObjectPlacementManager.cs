using System.Collections.Generic;
using UnityEngine;

// Central manager for placing, editing, and removing objects in the housing system.
// Handles raycasting, preview spawning, placement validation, and interaction with PlacedObject components.
// Also responsible for loading and saving placed objects via SaveManager hooks, and for maintaining the registry of known PlaceableItemSO definitions.
// The manager operates in three distinct modes (Placing, Editing, Removing) with separate logic for each mode's Update tick.

/* Important Note: In order to have saved data instantiate the correct prefab on load, the manager must be able to find a PlaceableItemSO definition by prefabID at runtime. 
    This is achieved by auto-loading all PlaceableItemSO assets from Resources/PlaceableItems/ at Start and caching them in the knownItems list. 
    If you add new PlaceableItemSO assets, make sure they are in that folder or added to the knownItems list manually in the Inspector.
*/
public class ObjectPlacementManager : MonoBehaviour
{
    private enum PlacementMode { None, Placing, Editing, Removing }
    private enum EditPhase { SelectingObject, RepositioningObject }

    [Header("References")]
    private Camera playerCamera;
    [SerializeField] private FP_Controller fpController;

    [Header("Placement Settings")]
    [SerializeField] private float placementDistance = 8f;
    [Tooltip("Layers valid as placement surfaces. Exclude player collider and restricted zones.")]
    [SerializeField] private LayerMask placementLayerMask;
    [Tooltip("Dedicated layer for placed objects only. Manager assigns objects to this layer after placing.")]
    [SerializeField] private LayerMask placedObjectLayerMask;

    [Header("Hover Materials")]
    [SerializeField] private Material hoverValidMaterial;
    [SerializeField] private Material hoverEditMaterial;
    [SerializeField] private Material hoverInvalidMaterial;

    [Header("Items Registry")]
    [Tooltip("Auto-populated from Resources/PlaceableItems/ at Start. Manual fallback only.")]
    [SerializeField] private List<PlaceableItemSO> knownItems = new();

    private PlacementMode _currentMode = PlacementMode.None;
    private EditPhase _editPhase = EditPhase.SelectingObject;
    private PlaceableItemSO _selectedItem;

    private GameObject _previewInstance;
    private Renderer[] _previewRenderers;
    private float _rotationOffset;

    private PlacedObject _targetedObject;
    private readonly List<PlacedObject> _placedObjects = new();
    private Material _lastAppliedMaterial;

    private void Start()
    {
        if (fpController != null)
            playerCamera = fpController.GetPlayerCamera();
        else
            Debug.LogError("[ObjectPlacer] FP_Controller reference is missing!");

        PlaceableItemSO[] found = Resources.LoadAll<PlaceableItemSO>("PlaceableItems");
        if (found != null && found.Length > 0)
        {
            foreach (PlaceableItemSO item in found)
                if (!knownItems.Contains(item)) knownItems.Add(item);

            Debug.Log($"[ObjectPlacer] Auto-loaded {found.Length} PlaceableItemSO assets from Resources.");
        }
        else
        {
            Debug.LogWarning("[ObjectPlacer] No PlaceableItemSO assets found in Resources/PlaceableItems/. " +
                             "Falling back to manually assigned knownItems list.");
        }

        ValidateReferences();
        // LoadPlacedObjects() is called by SaveManager.LoadGameSave() or manually if needed.
    }

    private void Update()
    {
        if (_currentMode == PlacementMode.None) return;
        switch (_currentMode)
        {
            case PlacementMode.Placing: TickPlacing(); break;
            case PlacementMode.Editing: TickEditing(); break;
            case PlacementMode.Removing: TickRemoving(); break;
        }
    }

    public void EnterPlacingMode(PlaceableItemSO item)
    {
        if (item == null) return;
        ExitCurrentMode();
        _selectedItem = item;
        _currentMode = PlacementMode.Placing;
        _rotationOffset = 0f;
        SpawnPreview(item.prefab);
        Debug.Log($"[ObjectPlacer] Placing: {item.displayName}");
        fpController.HandleBobbingState(false);
    }

    public void EnterEditMode()
    {
        ExitCurrentMode();
        _currentMode = PlacementMode.Editing;
        _editPhase = EditPhase.SelectingObject;
        Debug.Log("[ObjectPlacer] Edit mode — aim at a placed object and press Attack.");
        fpController.HandleBobbingState(false);
    }

    public void EnterRemoveMode()
    {
        ExitCurrentMode();
        _currentMode = PlacementMode.Removing;
        Debug.Log("[ObjectPlacer] Remove mode — aim at a placed object and press Attack.");
        fpController.HandleBobbingState(false);
    }

    public void ExitCurrentMode()
    {
        DestroyPreview();
        ClearTargetedObject();
        _editPhase = EditPhase.SelectingObject;
        _rotationOffset = 0f;
        _selectedItem = null;
        _currentMode = PlacementMode.None;
        fpController.HandleBobbingState(true);
    }

    // Prepares the manager for saving by removing any objects staged for deletion and ensuring the placed objects list is clean.
    public void PrepareForSave()
    {
        for (int i = _placedObjects.Count - 1; i >= 0; i--)
        {
            if (_placedObjects[i] == null || _placedObjects[i].MarkedForRemoval)
            {
                if (_placedObjects[i] != null) Destroy(_placedObjects[i].gameObject);
                _placedObjects.RemoveAt(i);
            }
        }

        Debug.Log($"[ObjectPlacer] Prepared for save — {_placedObjects.Count} active placed objects.");
    }

    // Loads placed objects from the save file. Called by SaveManager.LoadGameSave() after the standard scene sweep.
    public void LoadPlacedObjects()
    {
        SaveManager saveManager = FindFirstObjectByType<SaveManager>();
        if (saveManager == null)
        {
            Debug.LogError("[ObjectPlacer] SaveManager not found — cannot load placed objects.");
            return;
        }

        string path = saveManager.GetSavesPath();
        if (!System.IO.File.Exists(path)) return;

        string json = System.IO.File.ReadAllText(path);
        JsonWrapper wrapper = JsonUtility.FromJson<JsonWrapper>(json);
        if (wrapper == null || wrapper.entries == null) return;

        int loaded = 0;
        foreach (SaveEntry entry in wrapper.entries)
        {
            PlacedObjectSaveData data = JsonUtility.FromJson<PlacedObjectSaveData>(entry.jsonData);
            if (data == null || string.IsNullOrEmpty(data.prefabID)) continue;

            PlaceableItemSO definition = FindDefinitionByPrefabID(data.prefabID);
            if (definition == null)
            {
                Debug.LogWarning($"[ObjectPlacer] No PlaceableItemSO found for prefabID '{data.prefabID}'. Entry skipped.");
                continue;
            }

            GameObject go = Instantiate(definition.prefab, data.position, data.rotation);
            go.transform.localScale = data.scale;
            RegisterPlacedObject(go, definition, entry.id);
            loaded++;
        }

        Debug.Log($"[ObjectPlacer] Loaded {loaded} placed objects.");
    }

    // Destroys all placed objects in the scene and clears the registry. Use with caution — this cannot be undone.
    public void ClearAllPlacedObjects()
    {
        ExitCurrentMode();

        for (int i = _placedObjects.Count - 1; i >= 0; i--)
        {
            if (_placedObjects[i] != null)
                Destroy(_placedObjects[i].gameObject);
        }
        _placedObjects.Clear();

        Debug.Log("[ObjectPlacer] All placed objects cleared from scene.");
    }

    private void TickPlacing()
    {
        bool isValid = EvaluatePlacement(_selectedItem, out RaycastHit hit, out _);
        PositionAndRotatePreview(isValid, hit, _selectedItem);

        if (isValid) isValid = !IsPreviewOverlapping(hit);
        UpdatePreviewMaterial(isValid ? hoverValidMaterial : hoverInvalidMaterial);

        if (InputManager.Instance.IsRotatingObject)
        {
            _rotationOffset += _selectedItem.rotationStep;
            PositionAndRotatePreview(isValid, hit, _selectedItem);
        }

        if (InputManager.Instance.IsAttacking && isValid)
            CommitPlacement();

        if (InputManager.Instance.IsSecondary)
            ExitCurrentMode();
    }

    private void CommitPlacement()
    {
        GameObject placed = Instantiate(
            _selectedItem.prefab,
            _previewInstance.transform.position,
            _previewInstance.transform.rotation);

        RegisterPlacedObject(placed, _selectedItem, null);
        Debug.Log($"[ObjectPlacer] Placed: {_selectedItem.displayName}");

        _rotationOffset = 0f;
        _lastAppliedMaterial = null;
    }

    private void TickEditing()
    {
        switch (_editPhase)
        {
            case EditPhase.SelectingObject: TickEditSelection(); break;
            case EditPhase.RepositioningObject: TickEditRepositioning(); break;
        }
    }

    private void TickEditSelection()
    {
        PlacedObject hovered = RaycastForPlacedObject();

        if (hovered != _targetedObject)
        {
            ClearTargetedObject();
            _targetedObject = hovered;
            if (_targetedObject != null)
                ApplyMaterialToObject(_targetedObject, hoverEditMaterial);
        }

        if (InputManager.Instance.IsAttacking && _targetedObject != null)
            BeginRepositioning(_targetedObject);

        if (InputManager.Instance.IsSecondary)
            ExitCurrentMode();
    }

    private void BeginRepositioning(PlacedObject obj)
    {
        if (obj.Definition == null || obj.Definition.prefab == null)
        {
            Debug.LogError($"[ObjectPlacer] Cannot edit '{obj.gameObject.name}' — Definition or prefab is null.");
            ExitCurrentMode();
            return;
        }

        obj.SnapshotForEdit();

        _rotationOffset = obj.Definition.snapRotationToSurface ? 0f : obj.transform.eulerAngles.y;

        SpawnPreview(obj.Definition.prefab);
        _previewInstance.transform.SetPositionAndRotation(obj.transform.position, obj.transform.rotation);
        SetPreviewMaterial(hoverEditMaterial);
        _lastAppliedMaterial = hoverEditMaterial;

        _targetedObject = obj;
        obj.gameObject.SetActive(false);

        _editPhase = EditPhase.RepositioningObject;
        Debug.Log($"[ObjectPlacer] Repositioning: {obj.Definition.displayName}");
    }

    private void TickEditRepositioning()
    {
        bool isValid = EvaluatePlacement(_targetedObject.Definition, out RaycastHit hit, out _);
        PositionAndRotatePreview(isValid, hit, _targetedObject.Definition);

        if (isValid) isValid = !IsPreviewOverlapping(hit);
        UpdatePreviewMaterial(isValid ? hoverEditMaterial : hoverInvalidMaterial);

        if (InputManager.Instance.IsRotatingObject)
        {
            _rotationOffset += _targetedObject.Definition.rotationStep;
            PositionAndRotatePreview(isValid, hit, _targetedObject.Definition);
        }

        if (InputManager.Instance.IsAttacking && isValid)
            CommitEdit();

        if (InputManager.Instance.IsSecondary)
            CancelEdit();
    }

    private void CommitEdit()
    {
        _targetedObject.gameObject.SetActive(true);
        _targetedObject.transform.SetPositionAndRotation(
            _previewInstance.transform.position,
            _previewInstance.transform.rotation);

        DestroyPreview();
        _rotationOffset = 0f;
        _targetedObject = null;
        _editPhase = EditPhase.SelectingObject;
        Debug.Log("[ObjectPlacer] Edit committed.");
    }

    private void CancelEdit()
    {
        _targetedObject.gameObject.SetActive(true);
        _targetedObject.RevertToSnapshot();
        _targetedObject.RestoreMaterials();

        DestroyPreview();
        _rotationOffset = 0f;
        _targetedObject = null;
        _editPhase = EditPhase.SelectingObject;
        Debug.Log("[ObjectPlacer] Edit cancelled — reverted to snapshot.");
    }

    private void TickRemoving()
    {
        PlacedObject hovered = RaycastForPlacedObject();

        if (hovered != _targetedObject)
        {
            ClearTargetedObject();
            _targetedObject = hovered;
            if (_targetedObject != null)
                ApplyMaterialToObject(_targetedObject, hoverInvalidMaterial);
        }

        if (InputManager.Instance.IsAttacking && _targetedObject != null)
            StageForRemoval(_targetedObject);

        if (InputManager.Instance.IsSecondary)
            ExitCurrentMode();
    }

    private void StageForRemoval(PlacedObject obj)
    {
        _targetedObject = null;
        obj.MarkForRemoval(true);
        obj.gameObject.SetActive(false);
        string name = obj.Definition != null ? obj.Definition.displayName : obj.gameObject.name;
        Debug.Log($"[ObjectPlacer] Staged for removal: {name}. Press Save to commit.");
    }

    private bool EvaluatePlacement(PlaceableItemSO definition, out RaycastHit hit, out PlacementSurfaceType surfaceType)
    {
        surfaceType = PlacementSurfaceType.Floor;
        hit = default;

        Ray ray = new Ray(playerCamera.transform.position, playerCamera.transform.forward);

        if (!Physics.Raycast(ray, out hit, placementDistance, placementLayerMask, QueryTriggerInteraction.Ignore))
            return false;

        surfaceType = ClassifySurface(hit.normal);
        return (definition.allowedSurfaces & surfaceType) != 0;
    }

    private static PlacementSurfaceType ClassifySurface(Vector3 normal)
    {
        float dot = Vector3.Dot(normal, Vector3.up);
        if (dot > 0.7f) return PlacementSurfaceType.Floor;
        if (dot < -0.7f) return PlacementSurfaceType.Ceiling;
        return PlacementSurfaceType.Wall;
    }

    // Checks if the preview object is overlapping any existing placed objects, which would block placement.
    private bool IsPreviewOverlapping(RaycastHit surfaceHit)
    {
        if (_previewInstance == null) return false;

        Renderer[] renderers = _previewInstance.GetComponentsInChildren<Renderer>();
        if (renderers.Length == 0) return false;

        Bounds bounds = renderers[0].bounds;
        for (int i = 1; i < renderers.Length; i++)
            bounds.Encapsulate(renderers[i].bounds);

        int mask = placedObjectLayerMask;

        Collider[] hits = Physics.OverlapBox(
            bounds.center,
            bounds.extents * 0.95f, // Shrink the box slightly to avoid the resting surface counting as an overlap.
            _previewInstance.transform.rotation,
            mask,
            QueryTriggerInteraction.Ignore);

        return hits.Length > 0;
    }

    private void SpawnPreview(GameObject prefab)
    {
        DestroyPreview();
        _previewInstance = Instantiate(prefab);
        _lastAppliedMaterial = null;

        // Disable colliders so the preview doesn't block its own placement raycasts
        foreach (Collider col in _previewInstance.GetComponentsInChildren<Collider>())
            col.enabled = false;

        _previewRenderers = _previewInstance.GetComponentsInChildren<Renderer>();
        SetPreviewMaterial(hoverInvalidMaterial);
    }

    private void DestroyPreview()
    {
        if (_previewInstance == null) return;
        Destroy(_previewInstance);
        _previewInstance = null;
        _previewRenderers = null;
        _lastAppliedMaterial = null;
    }

    // Positions and rotates the preview instance based on the current surface hit and the definition's rotation rules.
    private void PositionAndRotatePreview(bool isValid, RaycastHit hit, PlaceableItemSO definition)
    {
        if (_previewInstance == null) return;

        bool hasHit = hit.normal != Vector3.zero;

        // ── Rotation ──────────────────────────────────────────────────────
        if (definition.snapRotationToSurface && hasHit)
        {
            Vector3 worldRef = (Mathf.Abs(Vector3.Dot(hit.normal, Vector3.forward)) < 0.9f)
                ? Vector3.forward
                : Vector3.right;

            Vector3 stableForward = Vector3.ProjectOnPlane(worldRef, hit.normal).normalized;
            Quaternion baseRotation = Quaternion.LookRotation(stableForward, hit.normal);
            Quaternion offsetSpin = Quaternion.AngleAxis(_rotationOffset, hit.normal);
            _previewInstance.transform.rotation = offsetSpin * baseRotation;
        }
        else
        {
            _previewInstance.transform.rotation = Quaternion.Euler(0f, _rotationOffset, 0f);
        }

        // ── Position ──────────────────────────────────────────────────────
        if (hasHit)
        {
            float offset = GetExtentAlongNormal(_previewInstance, hit.normal);
            _previewInstance.transform.position = hit.point + hit.normal * offset;
        }
        else
        {
            _previewInstance.transform.position =
                playerCamera.transform.position + playerCamera.transform.forward * placementDistance;
        }
    }

    // Calculates the distance from the object's pivot to its outer edge along the given normal direction.
    private static float GetExtentAlongNormal(GameObject go, Vector3 normal)
    {
        Renderer[] renderers = go.GetComponentsInChildren<Renderer>();
        if (renderers.Length == 0) return 0f;

        Bounds combined = renderers[0].bounds;
        for (int i = 1; i < renderers.Length; i++)
            combined.Encapsulate(renderers[i].bounds);

        Vector3 absNormal = new Vector3(
            Mathf.Abs(normal.x),
            Mathf.Abs(normal.y),
            Mathf.Abs(normal.z));

        float halfExtent = Vector3.Dot(combined.extents, absNormal);
        float pivotOffset = Vector3.Dot(combined.center - go.transform.position, -normal);

        return halfExtent + pivotOffset;
    }

    private void UpdatePreviewMaterial(Material mat)
    {
        if (mat == _lastAppliedMaterial) return;
        SetPreviewMaterial(mat);
        _lastAppliedMaterial = mat;
    }

    private void SetPreviewMaterial(Material mat)
    {
        if (_previewRenderers == null) return;
        foreach (Renderer r in _previewRenderers)
        {
            Material[] mats = new Material[r.sharedMaterials.Length];
            for (int i = 0; i < mats.Length; i++) mats[i] = mat;
            r.materials = mats;
        }
    }

    private PlacedObject RaycastForPlacedObject()
    {
        Ray ray = new Ray(playerCamera.transform.position, playerCamera.transform.forward);
        if (Physics.Raycast(ray, out RaycastHit hit, placementDistance, placedObjectLayerMask, QueryTriggerInteraction.Ignore))
            return hit.collider.GetComponentInParent<PlacedObject>();
        return null;
    }

    private void RegisterPlacedObject(GameObject go, PlaceableItemSO definition, string existingGUID)
    {
        PlacedObject po = go.GetComponent<PlacedObject>();
        if (po == null) po = go.AddComponent<PlacedObject>();
        po.Initialize(definition);

        GUIDComponent guid = go.GetComponent<GUIDComponent>();
        if (guid == null) guid = go.AddComponent<GUIDComponent>();

        if (!string.IsNullOrEmpty(existingGUID))
            ForceGUID(guid, existingGUID);

        SetLayerRecursive(go, LayerMaskToIndex(placedObjectLayerMask));
        _placedObjects.Add(po);
    }

    private void ApplyMaterialToObject(PlacedObject obj, Material mat)
    {
        foreach (Renderer r in obj.GetComponentsInChildren<Renderer>())
        {
            Material[] mats = new Material[r.sharedMaterials.Length];
            for (int i = 0; i < mats.Length; i++) mats[i] = mat;
            r.materials = mats;
        }
    }

    private void ClearTargetedObject()
    {
        if (_targetedObject != null)
            _targetedObject.RestoreMaterials();
        _targetedObject = null;
    }

    private PlaceableItemSO FindDefinitionByPrefabID(string id) =>
        knownItems.Find(item => item.prefabID == id);

    private static void SetLayerRecursive(GameObject go, int layer)
    {
        if (layer < 0) return;
        go.layer = layer;
        foreach (Transform child in go.transform)
            SetLayerRecursive(child.gameObject, layer);
    }

    private static int LayerMaskToIndex(LayerMask mask)
    {
        int value = mask.value;
        if (value == 0) return -1;
        int index = 0;
        while ((value & 1) == 0) { value >>= 1; index++; }
        return index;
    }

    private static void ForceGUID(GUIDComponent guid, string id)
    {
        var field = typeof(GUIDComponent)
            .GetField("id", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
        field?.SetValue(guid, id);
    }

    private void ValidateReferences()
    {
        if (playerCamera == null) Debug.LogError("[ObjectPlacer] playerCamera is not assigned!");
        if (fpController == null) Debug.LogError("[ObjectPlacer] fpController is not assigned!");
        if (hoverValidMaterial == null) Debug.LogError("[ObjectPlacer] hoverValidMaterial is not assigned!");
        if (hoverEditMaterial == null) Debug.LogError("[ObjectPlacer] hoverEditMaterial is not assigned!");
        if (hoverInvalidMaterial == null) Debug.LogError("[ObjectPlacer] hoverInvalidMaterial is not assigned!");
    }
}