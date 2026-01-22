using UnityEngine;
using UnityEngine.Events;
using System.Collections.Generic;
using System.Collections;

[RequireComponent(typeof(Collider))]
public class PuzzleTrigger : MonoBehaviour
{
    [Header("Tag Settings")]
    public string requiredTag = "PuzzleObject";

    [Header("Trigger Settings")]
    public bool triggerOnlyWhenEmpty = false; // Flag to only trigger when no other objects are inside
    private int objectsInside = 0;

    [Header("Events")]
    [Space(5)]
    [SerializeField] private UnityEvent onAnyEnter;    // Fires for ANY object
    [SerializeField] private UnityEvent onAnyExit;    // Fires for ANY object
    [SerializeField] private UnityEvent onTagEnter;  // Fires ONLY if tag matches
    [SerializeField] private UnityEvent onTagExit;  // Fires ONLY if tag matches

    [Header("Optional Puzzle Controller")]
    [Space(5)]
    [SerializeField] private bool linkToPuzzleController = false;
    public PuzzleController puzzleController;

    [Header("Optional MeshRenderer reference for Material Handling")]
    [Space(5)]
    [SerializeField] private MeshRenderer meshRenderer;
    [SerializeField] private float delayBeforeMaterialChange = 0f;

    [HideInInspector] public bool isActivated = false;

    void Start()
    {
        if(puzzleController == null)
        {
            linkToPuzzleController = false;
        }
        else if(linkToPuzzleController && puzzleController != null)
        {
            puzzleController.AddPuzzleTrigger(this);
        }
    }

    private void OnTriggerEnter(Collider other)
    {
        if(triggerOnlyWhenEmpty)
        {
            objectsInside++;
            if(objectsInside > 1)
            {
                return; // Do not trigger if more than one object is inside
            }
        }

        onAnyEnter.Invoke();
        Debug.Log("Object entered PuzzleTrigger: " + other.name);

        // If tag matches, fire tag-specific exit
        if (!isActivated && other.CompareTag(requiredTag))
        {
            isActivated = true;
            onTagEnter.Invoke();
            
            if (linkToPuzzleController && puzzleController != null)
            {
                puzzleController.CheckPuzzleState();    
            }

            Debug.Log("PuzzleTrigger activated by " + other.name);
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if(triggerOnlyWhenEmpty)
        {
            objectsInside--;
            if(objectsInside <= 0)
            {
                objectsInside = 0;
            }
            else
            {
                return; // No objects inside, do not trigger
            }
        }

        onAnyExit.Invoke();
        Debug.Log("Object exited PuzzleTrigger: " + other.name);

        // If tag matches, fire tag-specific exit
        if (isActivated && other.CompareTag(requiredTag))
        {
            isActivated = false;
            onTagExit.Invoke();

            if (linkToPuzzleController && puzzleController != null)
            {
                puzzleController.CheckPuzzleState();    
            }

            Debug.Log("PuzzleTrigger deactivated by " + other.name);
        }
    }

    // Optional* Method to set material to the MeshRenderer
    public void SetMaterialToRenderer(Material mat)
    {
        if(meshRenderer != null && mat != null)
        {
            if(delayBeforeMaterialChange > 0f)
            {
                StartCoroutine(DelayedMaterialChange(mat, delayBeforeMaterialChange));
            }
            else
            {
                meshRenderer.material = mat;
            }
        }
    }

    private IEnumerator DelayedMaterialChange(Material mat, float delay)
    {
        yield return new WaitForSeconds(delay);
        meshRenderer.material = mat;
    }
}
