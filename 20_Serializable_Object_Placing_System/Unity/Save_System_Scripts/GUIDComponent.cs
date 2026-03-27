using UnityEngine;

public class GUIDComponent : MonoBehaviour
{
    [SerializeField] private string id;
    public string ID => id;

#if UNITY_EDITOR
    private void OnValidate()
    {
        if (string.IsNullOrEmpty(id))
        {
            id = System.Guid.NewGuid().ToString();
        }
    }
#endif
}
