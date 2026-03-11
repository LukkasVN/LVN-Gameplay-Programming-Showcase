using UnityEngine;
using System.Collections.Generic;

#if UNITY_EDITOR
using UnityEditor;
#endif

[ExecuteInEditMode]
public class AssetGridVisualizer : MonoBehaviour
{
    [SerializeField] private bool enabledInEditorMode = true;
    [SerializeField] private float gridSeparation = 1f;
    [SerializeField] private List<AssetGrid> grids = new List<AssetGrid>();

    private List<GameObject> gridContainers = new List<GameObject>();
    private bool isDirty = true;

    private void OnValidate()
    {
        if (!enabledInEditorMode || !Application.isEditor)
            return;

        // Ensure defaults are set on new assets
        foreach (var grid in grids)
        {
            foreach (var row in grid.rows)
            {
                foreach (var assetData in row.assets)
                {
                    if (assetData.scaleMultiplier <= 0f)
                        assetData.scaleMultiplier = 1f;
                }
            }
        }

        isDirty = true;
    }

    #if UNITY_EDITOR
    private void Update()
    {
        if (!enabledInEditorMode || Application.isPlaying)
            return;

        if (HasGridConfigChanged())
            isDirty = true;

        if (isDirty)
        {
            RecalculateLayout();
            isDirty = false;
        }
    }

    private bool HasGridConfigChanged()
    {
        foreach (var grid in grids)
        {
            if (grid.HasConfigChanged())
                return true;
        }
        return false;
    }

    private void RecalculateLayout()
    {
        CleanupGridContainers();

        if (grids.Count == 0)
            return;

        float currentXOffset = 0f;

        for (int i = 0; i < grids.Count; i++)
        {
            GameObject gridContainer = CreateGridContainer(i);
            grids[i].InstantiateAndLayoutAssets(currentXOffset, gridContainer.transform);
            currentXOffset -= grids[i].GetGridWidth() + gridSeparation;
        }
    }

    private GameObject CreateGridContainer(int index)
    {
        GameObject container = new GameObject($"Grid {index + 1}");
        container.transform.SetParent(transform);
        container.transform.localPosition = Vector3.zero;
        gridContainers.Add(container);
        return container;
    }

    private void CleanupGridContainers()
    {
        foreach (var container in gridContainers)
        {
            if (container != null)
                DestroyImmediate(container);
        }
        gridContainers.Clear();
    }

    private void OnDestroy()
    {
        if (!Application.isEditor)
            return;

        CleanupGridContainers();
    }

    [ContextMenu("Refresh Layout")]
    public void RefreshLayout() => isDirty = true;

    [ContextMenu("RESET SCRIPT [DANGER]")]
    public void ResetScript()
    {
        CleanupGridContainers();
        grids.Clear();
        isDirty = true;
    }
    #endif
}

[System.Serializable]
public class AssetData
{
    public GameObject prefab;
    [Range(0.1f, 10f)] public float scaleMultiplier = 1f;
    public Vector3 rotation = Vector3.zero;
    public Vector3 positionOffset = Vector3.zero;
}

[System.Serializable]
public class AssetGrid
{
    [SerializeField] private float horizontalSeparation = 1f;
    [SerializeField] private float verticalSeparation = 1f;
    public List<AssetRow> rows = new List<AssetRow>();

    private List<GameObject> instantiatedAssets = new List<GameObject>();

    public bool HasConfigChanged()
    {
        int totalAssets = 0;
        foreach (var row in rows)
            totalAssets += row.assets.Count;

        return totalAssets != instantiatedAssets.Count;
    }

    public void InstantiateAndLayoutAssets(float xOffset, Transform parent)
    {
        instantiatedAssets.Clear();
        float currentY = 0f;

        for (int rowIndex = 0; rowIndex < rows.Count; rowIndex++)
        {
            float rowHeight = 0f;
            float currentX = xOffset;

            for (int colIndex = 0; colIndex < rows[rowIndex].assets.Count; colIndex++)
            {
                AssetData assetData = rows[rowIndex].assets[colIndex];
                if (assetData.prefab == null)
                    continue;

                GameObject instantiatedAsset = PrefabUtility.InstantiatePrefab(assetData.prefab) as GameObject;
                instantiatedAsset.name = assetData.prefab.name;
                instantiatedAsset.transform.SetParent(parent);

                Vector3 basePosition = new Vector3(currentX, 0, currentY);
                instantiatedAsset.transform.localPosition = basePosition + assetData.positionOffset;
                instantiatedAsset.transform.localRotation = Quaternion.Euler(assetData.rotation);
                instantiatedAsset.transform.localScale = Vector3.one * assetData.scaleMultiplier;

                instantiatedAssets.Add(instantiatedAsset);

                Bounds bounds = GetAssetBounds(instantiatedAsset);
                rowHeight = Mathf.Max(rowHeight, bounds.size.y);
                currentX += bounds.size.x + horizontalSeparation;
            }

            currentY -= rowHeight + verticalSeparation;
        }
    }

    public float GetGridWidth()
    {
        float maxWidth = 0f;

        foreach (var row in rows)
        {
            float rowWidth = 0f;
            foreach (var assetData in row.assets)
            {
                if (assetData.prefab == null)
                    continue;

                Bounds bounds = GetPrefabBounds(assetData.prefab);
                rowWidth += bounds.size.x * assetData.scaleMultiplier + horizontalSeparation;
            }
            maxWidth = Mathf.Max(maxWidth, rowWidth);
        }

        return maxWidth;
    }

    private Bounds GetAssetBounds(GameObject asset)
    {
        Collider collider = asset.GetComponent<Collider>();
        if (collider != null)
            return collider.bounds;

        Renderer renderer = asset.GetComponent<Renderer>();
        if (renderer != null)
            return renderer.bounds;

        return new Bounds(asset.transform.position, Vector3.one * 0.1f);
    }

    private Bounds GetPrefabBounds(GameObject prefab)
    {
        Collider collider = prefab.GetComponent<Collider>();
        if (collider != null)
            return collider.bounds;

        Renderer renderer = prefab.GetComponent<Renderer>();
        if (renderer != null)
            return renderer.bounds;

        return new Bounds(Vector3.zero, Vector3.one * 0.1f);
    }

}

[System.Serializable]
public class AssetRow
{
    public List<AssetData> assets = new List<AssetData>();
}