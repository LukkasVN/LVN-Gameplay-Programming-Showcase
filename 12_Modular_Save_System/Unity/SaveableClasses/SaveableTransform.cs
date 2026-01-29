using System.Collections;
using UnityEngine;

public class SaveableTransform : MonoBehaviour, ISaveable
{
    [SerializeField] private bool savePosition = true;
    [SerializeField] private bool saveRotation = true;
    [SerializeField] private bool saveScale = true;
    
    public object CaptureState()
    {
        TransformSaveData data = new TransformSaveData();

        if (savePosition) data.position = transform.position;
        if (saveRotation) data.rotation = transform.rotation;
        if (saveScale) data.scale = transform.localScale;

        return data;
    }

    public void RestoreState(object state)
    {
        TransformSaveData data = (TransformSaveData)state;
        StartCoroutine(ApplyAfterPhysics(data));
    }


    // Avoid physics conservation on load/save
    private IEnumerator ApplyAfterPhysics(TransformSaveData data)
    {
        yield return new WaitForFixedUpdate();

        Rigidbody rb = GetComponent<Rigidbody>();
        if (rb != null)
        {
            rb.isKinematic = true;

            if (savePosition) transform.position = data.position;
            if (saveRotation) transform.rotation = data.rotation;
            if (saveScale) transform.localScale = data.scale;

            yield return null;

            rb.isKinematic = false;
        }
        else
        {
            if (savePosition) transform.position = data.position;
            if (saveRotation) transform.rotation = data.rotation;
            if (saveScale) transform.localScale = data.scale;
        }
    }

}
