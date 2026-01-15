using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

public class ModularDialogueScript : MonoBehaviour
{
    [SerializeField] private DialoguePanelScript dialoguePanelScript;

    private bool hasDialogueBeenUsed = false;
    private bool isDialogueActive = false;

    [Header("Locked Dialogue Nodes")]
    public bool hasLockedBaseDialogueState = false;
    public List<DialogueNode> lockedDialogueNodes;

    [Header("Locked Dialogue Events")]
    [Space(10)]
    public UnityEvent onLockedDialogueStart;
    public UnityEvent onLockedDialogueEnd;

    [Header("Base Dialogue Nodes")]
    public List<DialogueNode> baseDialogueNodes;

    [Header("Base Dialogue Events")]
    [Space(10)]
    public UnityEvent onBaseDialogueStart;
    public UnityEvent onBaseDialogueEnd;

    [Header("Used Dialogue Nodes")]
    public bool hasUsedDialogueNodesState = false;
    public List<DialogueNode> hasUsedDialogueNodes;
    [Header("Used Dialogue Events")]
    [Space(10)]
    public UnityEvent onUsedDialogueStart;
    public UnityEvent onUsedDialogueEnd;
    private List<DialogueNode> currentDialogueList;
    private int currentIndex;


    void Start()
    {
        if (dialoguePanelScript == null)
            Debug.LogError("Dialogue Panel is not assigned in the inspector.");
    }

    private void StartDialogue()
    {
        if (isDialogueActive)
        {
            Debug.LogWarning("Dialogue is already active.");
            return;
        }

        isDialogueActive = true;

        if (hasLockedBaseDialogueState)
        {
            currentDialogueList = lockedDialogueNodes;
        }
        else
        {
            if (hasDialogueBeenUsed && hasUsedDialogueNodesState)
                currentDialogueList = hasUsedDialogueNodes;
            else
            {
                currentDialogueList = baseDialogueNodes;
            }
        }

        currentIndex = 0;

        if (currentDialogueList.Count == 0)
        {
            Debug.LogWarning("Dialogue list is empty.");
            EndDialogue();
            return;
        }

        dialoguePanelScript.DisplayDialogueNode(currentDialogueList[currentIndex]);

        if(hasLockedBaseDialogueState){
            Debug.Log("Dialogue started: Locked dialogue.");
            onLockedDialogueStart.Invoke();
            }
        else if (hasDialogueBeenUsed && hasUsedDialogueNodesState) {
            Debug.Log("Dialogue started: Used dialogue.");
            onUsedDialogueStart.Invoke();
        }
        else {
            Debug.Log("Dialogue started: Base dialogue.");
            onBaseDialogueStart.Invoke();
        }
    }

    /// <summary>
    /// Called to progress the dialogue to the next node or start it if not active
    /// </summary>
    public void HandleDialogueNode()
    {
        if (!isDialogueActive)
        {
            StartDialogue();
            return;
        }

        currentIndex++;

        if (currentIndex >= currentDialogueList.Count)
        {
            EndDialogue();
            return;
        }

        dialoguePanelScript.DisplayDialogueNode(currentDialogueList[currentIndex]);
    }

    private void EndDialogue()
    {
        if(hasLockedBaseDialogueState){
            Debug.Log("Dialogue ended: Locked dialogue.");
            onLockedDialogueEnd.Invoke();
            }
        else if (hasDialogueBeenUsed && hasUsedDialogueNodesState) {
            Debug.Log("Dialogue ended: Used dialogue.");
            onUsedDialogueEnd.Invoke();
        }
        else {
            Debug.Log("Dialogue ended: Base dialogue.");
            onBaseDialogueEnd.Invoke();
            hasDialogueBeenUsed = true;
        }
        isDialogueActive = false;
        dialoguePanelScript.HideDialoguePanel();
        currentDialogueList = null;
        currentIndex = 0;
    }

    /// <summary>
    /// Sets the lock state of the base dialogue
    /// </summary>
    public void SetDialogueLockState(bool isLocked)
    {
        hasLockedBaseDialogueState = isLocked;
    }


    /// <summary>
    /// Called to forcibly end the dialogue from external scripts
    /// </summary>
    public void ForceEndDialogue()
    {
        if (isDialogueActive)
        {
            isDialogueActive = false;
            dialoguePanelScript.ForceEndDialogue();
            currentDialogueList = null;
            currentIndex = 0;
        }
    }

    /// <summary>
    /// Sets the flag to determine if used dialogue nodes should be used
    /// </summary>
    public void SetUsedDialogueState(bool flag)
    {
        hasUsedDialogueNodesState = flag;
    }
}
