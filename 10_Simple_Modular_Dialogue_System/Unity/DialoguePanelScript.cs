using System.Collections;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class DialoguePanelScript : MonoBehaviour
{
    [SerializeField] private GameObject dialoguePanelObject;
    [SerializeField] private Image dialoguePanelImage;
    [SerializeField] private TextMeshProUGUI HeadlineText;
    [SerializeField] private TextMeshProUGUI dialogueText;
    private Coroutine textCoroutine;
    void Start()
    {
        if (dialoguePanelImage == null)
        {
            Debug.LogError("Dialogue Panel Image is not assigned in the inspector.");
        }
        if (HeadlineText == null)
        {
            Debug.LogError("Headline Text is not assigned in the inspector.");
        }
        if (dialogueText == null)
        {
            Debug.LogError("Dialogue Text is not assigned in the inspector.");
        }
    }

    private void SetDialoguePanelImage(Sprite newSprite)
    {
        if (newSprite != null)
        {
            dialoguePanelImage.sprite = newSprite;
            dialoguePanelImage.color = Color.white;
        }
        else
        {
            dialoguePanelImage.color = Color.clear;
        }
    }

    private void SetHeadlineText(string newHeadline)
    {
        if (string.IsNullOrEmpty(newHeadline))
        {
            HeadlineText.text = "";
            return;
        }
        HeadlineText.text = newHeadline;
    }

    public void ClearDialoguePanel()
    {
        dialoguePanelImage.sprite = null;
        HeadlineText.text = "";
        dialogueText.text = "";
    }

    private IEnumerator TextCoroutine(string newDialogue, float textDelay)
    {
        dialogueText.text = "";
        foreach (char letter in newDialogue.ToCharArray())
        {
            dialogueText.text += letter;
            yield return new WaitForSeconds(textDelay);
        }
    }

    public void DisplayDialogueNode(DialogueNode node)
    {
        if (!dialoguePanelObject.activeSelf)
            ShowDialoguePanel();

        if (textCoroutine != null)
            StopCoroutine(textCoroutine);

        ClearDialoguePanel();
        SetDialoguePanelImage(node.dialogueImage);
        SetHeadlineText(node.headlineText);

        textCoroutine = StartCoroutine(TextCoroutine(node.dialogueText, node.textDelay));
    }

    public void DisplayDialogueNodeImmediate(DialogueNode node)
    {
        SetDialoguePanelImage(node.dialogueImage);
        SetHeadlineText(node.headlineText);
    }

    public void ShowDialoguePanel()
    {
        dialoguePanelObject.SetActive(true);
    }

    public void HideDialoguePanel()
    {
        ClearDialoguePanel();
        dialoguePanelObject.SetActive(false);
    }

    public void ForceEndDialogue()
    {
        if (textCoroutine != null) {StopCoroutine(textCoroutine);}
        ClearDialoguePanel();
        dialoguePanelObject.SetActive(false);
    }
    
}
