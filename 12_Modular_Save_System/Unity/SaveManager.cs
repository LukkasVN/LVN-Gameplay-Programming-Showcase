using System.Collections;
using System.Collections.Generic;
using System.IO;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.SceneManagement;


public class SaveManager : MonoBehaviour
{
    private const string SaveFile = "Game_Save.json";
    // public GameObject loadingScreen; Optional Loading Screen

    public void SaveGame()
    {
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
    }

    public void LoadGameSave()
    {
        string path = GetSavesPath();
        if (!File.Exists(path))
            return;

        string json = File.ReadAllText(path);
        JsonWrapper wrapper = JsonUtility.FromJson<JsonWrapper>(json);

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
    }


    private string GetSavesPath()
    {
        string folder = Path.Combine(Application.persistentDataPath, "Saves");

        if (!Directory.Exists(folder))
            Directory.CreateDirectory(folder);

        return Path.Combine(folder, SaveFile);
    }

    public void DeleteSave()
    {
        string path = GetSavesPath();
        if (File.Exists(path))
            File.Delete(path);
    }

    public void ReloadScene()
    {
        SceneManager.LoadScene(SceneManager.GetActiveScene().buildIndex);
    }

    // For loading the save when the scene is loaded

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
