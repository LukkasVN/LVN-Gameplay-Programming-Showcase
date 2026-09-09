using UnityEngine;

[DisallowMultipleComponent]
public class DebugTracerFade : MonoBehaviour
{
    private LineRenderer line;
    private Color baseColor;
    private float duration;
    private float elapsed;

    public void Begin(LineRenderer target, Color color, float lifetime)
    {
        line = target;
        baseColor = color;
        duration = Mathf.Max(0.01f, lifetime);
    }

    private void Update()
    {
        if (line == null)
        {
            Destroy(gameObject);
            return;
        }

        elapsed += Time.deltaTime;

        float t = 1f - (elapsed / duration);
        float alpha = baseColor.a * t * t * t;

        Color faded = new Color(baseColor.r, baseColor.g, baseColor.b, alpha);
        line.startColor = faded;
        line.endColor = faded;

        if (elapsed >= duration)
            Destroy(gameObject);
    }
}
