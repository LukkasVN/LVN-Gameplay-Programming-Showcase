using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "DialogueNode", menuName = "Scriptable Objects/DialogueNode")]
public class DialogueNode : ScriptableObject
{
    public Sprite dialogueImage;
    public string headlineText;
    [TextArea(3, 10)]
    public string dialogueText;
    public float textDelay = 0.02f;
}
