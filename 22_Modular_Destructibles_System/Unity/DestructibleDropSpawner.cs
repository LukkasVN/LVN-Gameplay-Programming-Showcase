using UnityEngine;

/*
    Subscribes to a Destructible's typed events and forwards drop spawning to DropManager. 
    [You can replace the DropManager reference with your own drop system if needed, as long as it has a compatible API.] 

    Two independent drop tables: one rolled on every hit, one rolled on destruction.
    Leave either array empty to skip that trigger.

    - Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
*/

[RequireComponent(typeof(Destructible))]
public class DestructibleDropSpawner : MonoBehaviour
{
    [System.Serializable]
    public struct DropEntry
    {
        public DroppableItemData itemData;

        [Range(0f, 1f)]
        [Tooltip("Probability the entry drops at all. Fails this and nothing spawns. " + "Set to 1 for guaranteed drops with bonus variance.")]
        public float dropChance;

        [Min(0)]
        [Tooltip("Guaranteed pickups when the entry drops.")]
        public int minQuantity;

        [Min(0)]
        [Tooltip("Maximum pickups. (max - min) bonus rolls are made, each at bonusRollChance.")]
        public int maxQuantity;

        [Range(0f, 1f)]
        [Tooltip("Probability of each bonus roll succeeding. Ignored if min == max.")]
        public float bonusRollChance;
    }

    [Header("On Hit Drops")]
    [Tooltip("Rolled every successful hit. Leave empty to skip.")]
    [SerializeField] private DropEntry[] onHitDrops;

    [Header("On Destroyed Drops")]
    [Tooltip("Rolled once when HP reaches 0. Leave empty to skip.")]
    [SerializeField] private DropEntry[] onDestroyedDrops;

    [Header("Spawn")]
    [Tooltip("Vertical offset on the spawn point so items don't clip into the ground or hit surface.")]
    [SerializeField] private float spawnHeightOffset = 0.5f;

    private Destructible destructible;

    private void Awake() => destructible = GetComponent<Destructible>();

    private void OnEnable()
    {
        destructible.OnHitWithData += HandleHit;
        destructible.OnDestroyedWithData += HandleDestroyed;
    }

    private void OnDisable()
    {
        destructible.OnHitWithData -= HandleHit;
        destructible.OnDestroyedWithData -= HandleDestroyed;
    }

    private void HandleHit(Vector3 hitPosition, Vector3 hitDirection) => RollDrops(onHitDrops, hitPosition);
    private void HandleDestroyed(Vector3 position) => RollDrops(onDestroyedDrops, position);

    private void RollDrops(DropEntry[] table, Vector3 origin)
    {
        if (DropManager.Instance == null || table == null || table.Length == 0) return;

        Vector3 spawnPos = origin + Vector3.up * spawnHeightOffset;

        foreach (var entry in table)
        {
            if (entry.itemData == null) continue;
            if (Random.value > entry.dropChance) continue;

            // Minimum is guaranteed once the entry's chance passes.
            int count = entry.minQuantity;

            // Each bonus slot is an independent roll, giving a binomial distribution peaked around the expected value rather than uniform across the range.
            int bonusSlots = Mathf.Max(0, entry.maxQuantity - entry.minQuantity);
            for (int i = 0; i < bonusSlots; i++)
            {
                if (Random.value <= entry.bonusRollChance)
                    count++;
            }

            for (int i = 0; i < count; i++)
                DropManager.Instance.DropItem(entry.itemData, spawnPos);
        }
    }
}