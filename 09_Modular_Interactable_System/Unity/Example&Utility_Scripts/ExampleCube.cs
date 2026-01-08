using System.Collections;
using TMPro;
using UnityEngine;

[RequireComponent(typeof(Rigidbody))]
public class ExampleCube : MonoBehaviour
{
    private Rigidbody rb;
    public TextMeshProUGUI hoverStayTextUI;
    void Start()
    {
        rb = GetComponent<Rigidbody>();
    }

    public void RandomExplosion(float force)
    {
        Vector3 randomDirection = new Vector3(
            Random.Range(-1f, 1f),
            Random.Range(-1f, 1f),
            Random.Range(-1f, 1f)
        ).normalized;
    
        rb.AddForce(randomDirection * force, ForceMode.Impulse);
        rb.AddTorque(Random.insideUnitSphere * force, ForceMode.Impulse);
    }

    public void OnHoverStayTextRotator()
    {
        if (hoverStayTextUI != null && hoverStayTextUI.gameObject.activeSelf)
        {
            Debug.Log("Rotating hover stay text UI on every frame.");
            hoverStayTextUI.transform.Rotate(Vector3.up, 150f * Time.deltaTime);
        }
    }

}
