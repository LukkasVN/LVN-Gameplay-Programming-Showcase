using UnityEngine;

/// <summary>
/// Interface for objects that can be dropped and picked up.
/// Made by Lucas Varela Negro and set Open-Source for the LVN Gameplay Programming Showcase.
/// </summary>
public interface IDroppable
{
    DroppableItemData ItemData { get; }
    bool IsCollectible { get; }
    bool IsBeingCollected { get; }

    void MakeCollectible();
    void StartCollecting(Transform player);
    void OnCollected();
    void SetItemData(DroppableItemData data);
}