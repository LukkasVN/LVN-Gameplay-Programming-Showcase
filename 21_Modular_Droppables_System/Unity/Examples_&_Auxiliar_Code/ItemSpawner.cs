using Unity.VisualScripting;
using UnityEngine;

/// <summary>
/// Small utility class to demonstrate how to spawn droppable items from a MonoBehaviour in the scene.
/// </summary>
public class ItemSpawner : MonoBehaviour
{
    [SerializeField] private DroppableItemData itemData;
    [SerializeField] private int spawnCount = 3;
    [SerializeField] private float timeBetweenSpawns = 0.5f;
    [SerializeField] private float spawnHeight = 0.4f;
    private Vector3 spawnPoint;
    private bool finishedSpawning = false;

    void Start()
    {
        spawnPoint = transform.position;
        spawnPoint.y += spawnHeight;
    }

    public void Spawn()
    {
        if (finishedSpawning)
        {
            Debug.LogWarning("[ItemSpawner] Already finished spawning.");
            return;
        }
        
        GameObject prefab = itemData.itemPrefab;

        if (DropManager.Instance == null)
        {
            Debug.LogError("[DropSpawnerExample] No DropManager in scene!");
            return;
        }

        if (itemData == null || prefab == null)
        {
            Debug.LogError("[DropSpawnerExample] Missing itemData or prefab!");
            return;
        }

        DropManager.Instance.DropItem(itemData, spawnPoint);
        spawnCount--;

        if (spawnCount > 0)
        {
            Invoke(nameof(Spawn), timeBetweenSpawns);
        }
        else
        {
            finishedSpawning = true; // Was never set, warning could never fire
            Debug.Log("[DropSpawnerExample] Finished spawning items.");
        }
    }
}
