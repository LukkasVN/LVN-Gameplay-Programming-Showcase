using TMPro;
using UnityEngine;

[RequireComponent(typeof(TextMeshProUGUI))]
public class TextFacesPlayer : MonoBehaviour
{
    private TextMeshProUGUI textMesh;
    private Transform playerTransform;
    public Camera debugCamera;
    void Start()
    {
        textMesh = GetComponent<TextMeshProUGUI>();
        GameObject player = GameObject.FindGameObjectWithTag("Player");
        if (player != null)
        {
            playerTransform = player.transform;
        }
        else

        {
            Debug.LogWarning("Player object with tag 'Player' not found in the scene, using debug camera instead.");
            FaceDebugCamera();
        }
    }

    void Update()
    {
        if (playerTransform != null && gameObject.activeSelf)
        {
            FacePlayer(playerTransform);
        }
    }

    private void FacePlayer(Transform playerTransform)
    {
        Vector3 directionToPlayer = playerTransform.position - transform.position;
        directionToPlayer.y = 0; // Keep only the horizontal direction
        if (directionToPlayer != Vector3.zero)
        {
            Quaternion targetRotation = Quaternion.LookRotation(-directionToPlayer);
            transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.deltaTime * 5f);
        }
    }

    private void FaceDebugCamera()
    {
        if (debugCamera != null)
        {
            Vector3 directionToCamera = debugCamera.transform.position - transform.position;
            directionToCamera.y = 0; // Keep only the horizontal direction
            if (directionToCamera != Vector3.zero)
            {
                Quaternion targetRotation = Quaternion.LookRotation(-directionToCamera);
                transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.deltaTime * 5f);
            }
        }
        else
        {
            Debug.LogWarning("Debug camera not assigned.");
        }
    }

}
