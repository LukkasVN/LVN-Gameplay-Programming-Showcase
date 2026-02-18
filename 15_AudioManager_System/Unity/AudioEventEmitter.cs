using UnityEngine;

public class AudioEventEmitter : MonoBehaviour
{
    [SerializeField] private float customPitch = 1f;
    public void PlayAtOwnPosition(AudioClip clip)
    {
        AudioManager.Instance.PlayAtPosition(clip, transform.position);
    }

    public void PlayAtOwnPositionCustomPitch(AudioClip clip)
    {
        AudioManager.Instance.PlayAtPosition(clip, transform.position, customPitch);
    }
}
