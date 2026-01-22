using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.Events;

public class PuzzleController : MonoBehaviour
{
    private List<PuzzleTrigger> puzzleTriggers = new List<PuzzleTrigger>();

    [Header("Puzzle State Events")]
    [Space(5)]
    public UnityEvent onPuzzleComplete;
    public UnityEvent onPuzzleCancelComplete;

    [Header("Optional Delay Before Checking Puzzle State")]
    [SerializeField] private float delayBeforeCheck = 0f;

    private bool puzzleComplete = false;

    public void AddPuzzleTrigger(PuzzleTrigger trigger)
    {
        if (!puzzleTriggers.Contains(trigger))
        {
            puzzleTriggers.Add(trigger);
        }
    }

    public void CheckPuzzleState()
    {
        if(delayBeforeCheck > 0f)
        {
            StartCoroutine(DelayedCheckPuzzleState());
            return;
        }
        else
        {
            CheckPuzzleStateMethod();
        }
    }

    private IEnumerator DelayedCheckPuzzleState()
    {
        yield return new WaitForSeconds(delayBeforeCheck);
        CheckPuzzleStateMethod();
    }

    public void CheckPuzzleStateMethod()
    {
        bool allActivated = true;

        foreach (var t in puzzleTriggers)
        {
            if (!t.isActivated)
            {
                allActivated = false;
                break;
            }
        }

        if (allActivated && !puzzleComplete)
        {
            puzzleComplete = true;
            onPuzzleComplete.Invoke();
        }
        else if (!allActivated && puzzleComplete)
        {
            puzzleComplete = false;
            onPuzzleCancelComplete.Invoke();
        }
    }
    
}

