using UnityEngine;

public interface IFP_Interactable
{
    void OnInteract(Vector3 InteractionDirection);
    void OnFocusEnter();
    void OnFocusExit();
}
