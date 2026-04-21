using UnityEngine;
using System.Collections.Generic;

/// <summary>
/// Manages pooling and spawning of droppable items in the scene.
/// Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
/// </summary>
public class DropManager : MonoBehaviour
{
    public static DropManager Instance { get; private set; }

    [SerializeField] private int poolSize = 50;
    [SerializeField] private List<GameObject> droppableItemPrefabs = new List<GameObject>();

    private Dictionary<GameObject, Queue<DroppableItem>> pooledItemsByPrefab = new();
    private Dictionary<GameObject, Transform> poolParentsByPrefab = new(); 

    private List<DroppableItem> activeDrops = new List<DroppableItem>();

    private Transform poolParent;

    private void Awake()
    {
        if (Instance != null && Instance != this)
        {
            Destroy(gameObject);
            return;
        }

        Instance = this;

        InitializePool();
    }

    private void InitializePool()
    {
        if (droppableItemPrefabs.Count == 0)
        {
            Debug.LogError("[DropManager] No DroppableItem prefabs assigned!");
            return;
        }

        GameObject poolContainerGO = new GameObject("DroppablePool");
        poolContainerGO.transform.SetParent(transform);
        poolParent = poolContainerGO.transform;

        foreach (GameObject prefab in droppableItemPrefabs)
        {
            if (prefab == null) continue;

            // Create a parent per prefab
            GameObject prefabParentGO = new GameObject($"Pool_{prefab.name}");
            prefabParentGO.transform.SetParent(poolParent);
            Transform prefabParent = prefabParentGO.transform;

            poolParentsByPrefab[prefab] = prefabParent;

            Queue<DroppableItem> prefabPool = new Queue<DroppableItem>();

            for (int i = 0; i < poolSize; i++)
            {
                GameObject pooledGO = Instantiate(prefab, prefabParent);
                pooledGO.name = $"PooledDrop_{prefab.name}_{i}";
                pooledGO.SetActive(false);

                DroppableItem pooledItem = pooledGO.GetComponent<DroppableItem>();
                if (pooledItem != null)
                {
                    pooledItem.SetSourcePrefab(prefab); // REQUIRED
                    pooledItem.SetManagedByPool(true); // OPTIONAL - Only needed if you want the item to know it's pool-managed
                    prefabPool.Enqueue(pooledItem);
                }
            }

            pooledItemsByPrefab[prefab] = prefabPool;

            Debug.Log($"[DropManager] Initialized pool for {prefab.name} with {poolSize} items");
        }
    }

    private DroppableItem GetFromPool(GameObject prefab)
    {
        if (!pooledItemsByPrefab.ContainsKey(prefab))
        {
            Debug.LogWarning($"[DropManager] Prefab {prefab.name} not in pool dictionary!");
            return null;
        }

        Queue<DroppableItem> prefabPool = pooledItemsByPrefab[prefab];

        if (prefabPool.Count > 0)
        {
            return prefabPool.Dequeue();
        }

        // Expand pool if needed
        Transform parent = poolParentsByPrefab[prefab];

        GameObject newGO = Instantiate(prefab, parent);
        DroppableItem item = newGO.GetComponent<DroppableItem>();

        if (item != null)
        {
            item.SetSourcePrefab(prefab);
        }

        return item;
    }

    private void ReturnToPool(DroppableItem item)
    {
        item.gameObject.SetActive(false);
        item.SetItemData(null);
        item.SetManagedByPool(false);

        GameObject prefab = item.SourcePrefab;

        if (prefab != null && pooledItemsByPrefab.ContainsKey(prefab))
        {
            item.transform.SetParent(poolParentsByPrefab[prefab]);
            pooledItemsByPrefab[prefab].Enqueue(item);
        }
        else
        {
            Debug.LogWarning("[DropManager] Returned item has no valid prefab reference!");
        }
    }

    public DroppableItem DropItem(DroppableItemData itemData, Vector3 position)
    {
        if (itemData == null)
        {
            Debug.LogError("[DropManager] Cannot drop item with null ItemData!");
            return null;
        }

        GameObject prefab = itemData.itemPrefab;

        if (prefab == null)
        {
            Debug.LogError($"[DropManager] ItemData {itemData.itemName} has no prefab assigned!");
            return null;
        }

        if (!pooledItemsByPrefab.ContainsKey(prefab))
        {
            Debug.LogError($"[DropManager] Prefab {prefab.name} is not registered in the pool!");
            return null;
        }

        DroppableItem dropItem = GetFromPool(prefab);

        if (dropItem == null)
        {
            Debug.LogError("[DropManager] Failed to get item from pool!");
            return null;
        }

        dropItem.gameObject.SetActive(true);
        dropItem.transform.position = position;
        dropItem.transform.SetParent(null);
        dropItem.SetManagedByPool(true); // OPTIONAL - Only needed if you want the item to know it's pool-managed

        dropItem.SetItemData(itemData);

        int finalQuantity = Random.Range(itemData.minDropQuantity,itemData.maxDropQuantity + 1);

        dropItem.SetQuantity(finalQuantity);

        activeDrops.Add(dropItem);

        dropItem.OnSpawn();

        Debug.Log($"[DropManager] Dropped {itemData.itemName} x{finalQuantity}. Active drops: {activeDrops.Count}");

        

        return dropItem;
    }

    public void OnDropCollected(DroppableItem drop)
    {
        if (activeDrops.Contains(drop))
        {
            activeDrops.Remove(drop);
            ReturnToPool(drop);
            Debug.Log($"[DropManager] Item returned to pool. Active drops: {activeDrops.Count}");
        }
    }

    public int GetActiveDropCount() => activeDrops.Count;

}