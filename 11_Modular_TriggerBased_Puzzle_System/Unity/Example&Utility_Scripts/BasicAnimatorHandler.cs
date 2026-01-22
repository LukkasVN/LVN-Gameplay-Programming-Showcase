using UnityEngine;

public class BasicAnimatorHandler : MonoBehaviour
{
    public string parameterName;
    public Animator animator;
    
    public void SetParameterBool(bool value)
    {
        if (animator != null && !string.IsNullOrEmpty(parameterName))
        {
            animator.SetBool(parameterName, value);
        }
    }
    
}
