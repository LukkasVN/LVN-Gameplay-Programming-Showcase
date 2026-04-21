using UnityEngine;

[CreateAssetMenu(fileName = "DroppableItemData", menuName = "Scriptable Objects/DroppableItemData")]
public class DroppableItemData : ItemSO
{
    [Header("Drop Settings")]
    public int baseQuantity = 1;
    public int minDropQuantity = 1;
    public int maxDropQuantity = 3;

    [Header("Collection Settings")]
    public float collectDelay = 1f;
    public float collectSpeed = 15f;

    [Header("Float/Rotation Settings")]
    public float floatRotationSpeed = 90f;
}
