using UnityEngine;

[CreateAssetMenu(fileName = "ItemSO", menuName = "Scriptable Objects/ItemSO")]
public class ItemSO : ScriptableObject
{
    [Header("Prefab Reference [Optional but Mandatory for Droppable Items]")]
    public GameObject itemPrefab;

    [Header("Item Identity")]
    public string itemID;
    public string itemName;

    [Header("Display")]
    public Sprite itemIcon;
    public Color tierColor = Color.white;
    [TextArea(2, 4)]
    public string description;

    public virtual void OnValidate()
    {
        if (string.IsNullOrEmpty(itemID))
            itemID = name;
    }
}
