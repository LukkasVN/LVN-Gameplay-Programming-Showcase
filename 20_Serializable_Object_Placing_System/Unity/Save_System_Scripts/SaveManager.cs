using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SaveManager : MonoBehaviour
{
    private const string SaveFile = "Game_Save.json";
    [SerializeField] private ObjectPlacementManager placementManager;
    // public GameObject loadingScreen; // Optional Loading Screen

    public void SaveGame()
    {
        // Give the placement manager a chance to clean up staged removals and prepare any pending placed objects for saving before we capture the scene state.
        placementManager?.PrepareForSave();

        JsonWrapper wrapper = new JsonWrapper();

        foreach (var saveable in FindObjectsByType<MonoBehaviour>(FindObjectsSortMode.None))
        {
            if (saveable is ISaveable saveableComponent)
            {
                var guidComponent = saveable.GetComponent<GUIDComponent>();
                if (guidComponent != null)
                {
                    object state = saveableComponent.CaptureState();
                    string json = JsonUtility.ToJson(state);

                    wrapper.entries.Add(new SaveEntry
                    {
                        id = guidComponent.ID,
                        jsonData = json,
                        type = state.GetType().AssemblyQualifiedName
                    });
                }
            }
        }

        string wrapperJson = JsonUtility.ToJson(wrapper, true);
        File.WriteAllText(GetSavesPath(), wrapperJson);

        Debug.Log("[SaveManager] Game saved.");
    }

    public void LoadGameSave()
    {
        string path = GetSavesPath();
        if (!File.Exists(path)) return;

        string json = File.ReadAllText(path);
        JsonWrapper wrapper = JsonUtility.FromJson<JsonWrapper>(json);

        // First pass: restore all ISaveable objects in the scene to their saved state.
        foreach (var saveable in FindObjectsByType<MonoBehaviour>(FindObjectsSortMode.None))
        {
            if (saveable is ISaveable saveableComponent)
            {
                var guidComponent = saveable.GetComponent<GUIDComponent>();
                if (guidComponent != null)
                {
                    string id = guidComponent.ID;
                    SaveEntry entry = wrapper.entries.Find(e => e.id == id);

                    if (entry != null)
                    {
                        System.Type type = System.Type.GetType(entry.type);
                        object state = JsonUtility.FromJson(entry.jsonData, type);
                        saveableComponent.RestoreState(state);
                    }
                }
            }
        }

        // Second pass: tell the placement manager to load any placed objects from the save data. 
        // This is necessary because placed objects are instantiated at runtime and won't exist in the scene until we explicitly spawn them based on the saved PlacedObjectSaveData.
        placementManager?.LoadPlacedObjects();

        Debug.Log("[SaveManager] Game loaded.");
    }

    // Deletes the save file and clears all placed objects from the scene. 
    public void DeleteSave()
    {
        // Clear placed objects from the scene before wiping the file
        placementManager?.ClearAllPlacedObjects();

        string path = GetSavesPath();
        if (File.Exists(path))
        {
            File.Delete(path);
            Debug.Log("[SaveManager] Save file deleted.");
        }
        else
        {
            Debug.Log("[SaveManager] No save file found to delete.");
        }
    }

    public void ReloadScene()
    {
        SceneManager.LoadScene(SceneManager.GetActiveScene().buildIndex);
    }

    public string GetSavesPath()
    {
        string folder = Path.Combine(Application.persistentDataPath, "Saves");
        if (!Directory.Exists(folder)) Directory.CreateDirectory(folder);
        return Path.Combine(folder, SaveFile);
    }

    // private void OnEnable()
    // {
    //     SceneManager.sceneLoaded += OnSceneLoaded;
    // }

    // private void OnDisable()
    // {
    //     SceneManager.sceneLoaded -= OnSceneLoaded;
    // }

    // private void OnSceneLoaded(Scene scene, LoadSceneMode mode)
    // {
    //     StartCoroutine(LoadAfterScene());
    // }

    // private IEnumerator LoadAfterScene()
    // {
    //     loadingScreen.SetActive(true);
    //     yield return new WaitForEndOfFrame();
    //     LoadGameSave();
    //     loadingScreen.SetActive(false);
    // }
}