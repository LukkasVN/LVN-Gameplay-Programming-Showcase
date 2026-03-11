#if UNITY_EDITOR
using UnityEditor;
using UnityEngine;

[CustomPropertyDrawer(typeof(AssetData))]
public class AssetDataDrawer : PropertyDrawer
{
    public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
    {
        EditorGUI.BeginProperty(position, label, property);

        property.isExpanded = EditorGUI.Foldout(new Rect(position.x, position.y, position.width - 60, EditorGUIUtility.singleLineHeight), property.isExpanded, label);

        // Draw Reset button
        if (GUI.Button(new Rect(position.width - 55, position.y, 50, EditorGUIUtility.singleLineHeight), "Reset"))
        {
            var prefabProp = property.FindPropertyRelative("prefab");
            var scaleProp = property.FindPropertyRelative("scaleMultiplier");
            var rotationProp = property.FindPropertyRelative("rotation");
            var offsetProp = property.FindPropertyRelative("positionOffset");

            prefabProp.objectReferenceValue = null;
            scaleProp.floatValue = 1f;
            rotationProp.vector3Value = Vector3.zero;
            offsetProp.vector3Value = Vector3.zero;

            property.serializedObject.ApplyModifiedProperties();
        }

        if (property.isExpanded)
        {
            EditorGUI.indentLevel++;
            float yOffset = EditorGUIUtility.singleLineHeight + 2f;

            // Prefab
            var prefabProp = property.FindPropertyRelative("prefab");
            EditorGUI.PropertyField(new Rect(position.x, position.y + yOffset, position.width, EditorGUIUtility.singleLineHeight), prefabProp);
            yOffset += EditorGUIUtility.singleLineHeight + 2f;

            // Scale Multiplier
            var scaleProp = property.FindPropertyRelative("scaleMultiplier");
            EditorGUI.PropertyField(new Rect(position.x, position.y + yOffset, position.width, EditorGUIUtility.singleLineHeight), scaleProp);
            yOffset += EditorGUIUtility.singleLineHeight + 2f;

            // Rotation
            var rotationProp = property.FindPropertyRelative("rotation");
            EditorGUI.PropertyField(new Rect(position.x, position.y + yOffset, position.width, EditorGUIUtility.singleLineHeight), rotationProp);
            yOffset += EditorGUIUtility.singleLineHeight + 2f;

            // Position Offset
            var offsetProp = property.FindPropertyRelative("positionOffset");
            EditorGUI.PropertyField(new Rect(position.x, position.y + yOffset, position.width, EditorGUIUtility.singleLineHeight), offsetProp);

            EditorGUI.indentLevel--;
        }

        EditorGUI.EndProperty();
    }

    public override float GetPropertyHeight(SerializedProperty property, GUIContent label)
    {
        if (!property.isExpanded)
            return EditorGUIUtility.singleLineHeight;

        return EditorGUIUtility.singleLineHeight * 5 + 8f;
    }
}
#endif