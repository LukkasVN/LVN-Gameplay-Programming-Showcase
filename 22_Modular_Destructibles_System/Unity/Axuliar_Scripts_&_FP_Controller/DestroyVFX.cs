using UnityEngine;

public class DestroyVFX : MonoBehaviour
{
    void Start()
    {
        Destroy(gameObject, 1.5f); // Destroy this VFX object after 1.5 seconds to clean up the scene.
    }

}
