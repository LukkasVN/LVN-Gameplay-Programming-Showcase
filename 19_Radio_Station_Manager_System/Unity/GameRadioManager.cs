using UnityEngine;
using System.Collections.Generic;
using System.IO;
using UnityEngine.Networking;
using System.Collections;

public class GameRadioManager : MonoBehaviour
{
    [System.Serializable]
    public class Station
    {
        public string stationName = "Station 1";
        public List<AudioClip> tracks = new List<AudioClip>();
    }

    [System.Serializable]
    public class ActiveStation
    {
        public int stationIndex;
        public AudioClip currentClip;
        public float playbackTime;
        public bool isPlaying;
        public float lastAccessTime;
        public float lastUpdateTime;
        public int tracksPlayedCount;
        public AudioClip lastClip;
    }

    public delegate void OnTrackChanged(int stationIndex, AudioClip newClip);
    public static event OnTrackChanged TrackChanged;

    [SerializeField] private List<Station> stations = new List<Station>();
    public List<Station> Stations => stations;

    private Dictionary<int, ActiveStation> activeStations = new Dictionary<int, ActiveStation>();

    [SerializeField] private string musicFolderName = "GameCustomRadioMusic";
    private string musicPath;
    private const float STATION_CLEANUP_TIME = 180f;
    private const int MAX_TRACK_ATTEMPTS = 5;
    public int customStationIndex { get; private set; }

    private void Awake()
    {
        musicPath = Path.Combine(Application.persistentDataPath, musicFolderName);
        if (!Directory.Exists(musicPath))
        {
            Directory.CreateDirectory(musicPath);
            Debug.Log($"Music folder created at: {musicPath}");
        }
    }

    private void Start()
    {
        if (Stations.Count == 0 || Stations[Stations.Count - 1].stationName != "Custom")
            stations.Add(new Station { stationName = "Custom" });

        customStationIndex = GetStationCount() - 1;
        LoadMusicFromFolder(Stations[customStationIndex], () =>
            Debug.Log($"Loaded {Stations[customStationIndex].tracks.Count} tracks into Custom station"));
    }

    private void Update()
    {
        TrackActiveStations();
        CleanupInactiveStations();
    }

    public void SetStationActive(int stationIndex, AudioSource audioSource)
    {
        if (activeStations.ContainsKey(stationIndex))
        {
            var active = activeStations[stationIndex];
            active.lastAccessTime = Time.realtimeSinceStartup;

            if (active.currentClip != null)
            {
                audioSource.clip = active.currentClip;
                audioSource.time = active.playbackTime;
                if (active.isPlaying)
                    audioSource.Play();
            }
        }
        else
        {
            activeStations[stationIndex] = new ActiveStation
            {
                stationIndex = stationIndex,
                currentClip = null,
                playbackTime = 0f,
                isPlaying = false,
                lastAccessTime = Time.realtimeSinceStartup,
                lastUpdateTime = Time.realtimeSinceStartup,
                tracksPlayedCount = 0,
                lastClip = null
            };
        }
    }

    public void RegisterTrack(int stationIndex, AudioClip clip)
    {
        if (!activeStations.ContainsKey(stationIndex)) return;

        var s = activeStations[stationIndex];
        s.currentClip = clip;
        s.lastClip = clip;
        s.playbackTime = 0f;
        s.isPlaying = true;
        s.lastAccessTime = Time.realtimeSinceStartup;
        s.lastUpdateTime = Time.realtimeSinceStartup;
    }

    public void TrackActiveStations()
    {
        foreach (var kvp in activeStations)
        {
            var station = kvp.Value;
            if (station.currentClip == null) continue;

            float delta = Time.realtimeSinceStartup - station.lastUpdateTime;
            station.playbackTime += delta;

            if (station.playbackTime >= station.currentClip.length)
            {
                station.tracksPlayedCount++;
                AdvanceStationTrack(kvp.Key);
            }

            station.lastUpdateTime = Time.realtimeSinceStartup;
        }
    }

    private void AdvanceStationTrack(int stationIndex)
    {
        var stationData = Stations[stationIndex];
        if (stationData.tracks.Count == 0) return;

        AudioClip newClip = null;
        int attempts = 0;

        do
        {
            newClip = stationData.tracks[Random.Range(0, stationData.tracks.Count)];
            attempts++;
        } while (newClip == activeStations[stationIndex].lastClip && attempts < MAX_TRACK_ATTEMPTS);

        var s = activeStations[stationIndex];
        s.lastClip = newClip;
        s.currentClip = newClip;
        s.playbackTime = 0f;
        s.isPlaying = true;
        s.lastUpdateTime = Time.realtimeSinceStartup;

        TrackChanged?.Invoke(stationIndex, newClip);
    }

    public void UpdateStationTime(int stationIndex, float time, bool isPlaying)
    {
        if (!activeStations.ContainsKey(stationIndex)) return;

        var s = activeStations[stationIndex];
        s.playbackTime = time;
        s.isPlaying = isPlaying;
        s.lastAccessTime = Time.realtimeSinceStartup;
        s.lastUpdateTime = Time.realtimeSinceStartup;
    }

    public float GetStationTime(int stationIndex)
        => activeStations.ContainsKey(stationIndex) ? activeStations[stationIndex].playbackTime : 0f;

    public AudioClip GetActiveStationClip(int stationIndex)
        => activeStations.ContainsKey(stationIndex) ? activeStations[stationIndex].currentClip : null;

    public bool IsStationActive(int stationIndex)
        => activeStations.ContainsKey(stationIndex);

    private void CleanupInactiveStations()
    {
        var toRemove = new List<int>();
        foreach (var kvp in activeStations)
        {
            if (Time.realtimeSinceStartup - kvp.Value.lastAccessTime > STATION_CLEANUP_TIME)
                toRemove.Add(kvp.Key);
        }
        foreach (int index in toRemove)
            activeStations.Remove(index);
    }

    public void LoadMusicFromFolder(Station targetStation, System.Action onComplete = null)
    {
        if (!Directory.Exists(musicPath))
        {
            Directory.CreateDirectory(musicPath);
            Debug.LogWarning($"Music folder recreated at: {musicPath}");
        }

        targetStation.tracks.Clear();

        var allAudioFiles = new List<string>();
        allAudioFiles.AddRange(Directory.GetFiles(musicPath, "*.wav"));
        allAudioFiles.AddRange(Directory.GetFiles(musicPath, "*.mp3"));

        if (allAudioFiles.Count == 0)
        {
            Debug.LogWarning($"No audio files found in {musicPath}");
            onComplete?.Invoke();
            return;
        }

        StartCoroutine(LoadAudioFilesCoroutine(allAudioFiles, targetStation, onComplete));
    }

    public void RefreshCustomStation(System.Action onComplete = null)
    {
        if (activeStations.ContainsKey(customStationIndex))
            activeStations.Remove(customStationIndex);

        var customStation = Stations[customStationIndex];
        customStation.tracks.Clear();

        var allAudioFiles = new List<string>();
        allAudioFiles.AddRange(Directory.GetFiles(musicPath, "*.wav"));
        allAudioFiles.AddRange(Directory.GetFiles(musicPath, "*.mp3"));

        if (allAudioFiles.Count == 0)
        {
            Debug.LogWarning($"No audio files found in {musicPath}");
            onComplete?.Invoke();
            return;
        }

        StartCoroutine(LoadAudioFilesCoroutine(allAudioFiles, customStation, onComplete));
    }

    private IEnumerator LoadAudioFilesCoroutine(List<string> filePaths, Station targetStation, System.Action onComplete)
    {
        foreach (string filePath in filePaths)
        {
            yield return StartCoroutine(LoadAudioClipCoroutine(filePath, targetStation));
        }
        onComplete?.Invoke();
    }

    private IEnumerator LoadAudioClipCoroutine(string filePath, Station targetStation)
    {
        using (UnityWebRequest www = UnityWebRequestMultimedia.GetAudioClip("file:///" + filePath, AudioType.UNKNOWN))
        {
            yield return www.SendWebRequest();

            if (string.IsNullOrEmpty(www.error))
            {
                AudioClip clip = DownloadHandlerAudioClip.GetContent(www);
                clip.name = Path.GetFileNameWithoutExtension(filePath);
                targetStation.tracks.Add(clip);
            }
            else
            {
                Debug.LogError($"Failed to load {Path.GetFileName(filePath)}: {www.error}");
            }
        }
    }

    public int GetStationCount() => Stations.Count;

    public void BroadcastTrackChange(int stationIndex, AudioClip newClip)
        => TrackChanged?.Invoke(stationIndex, newClip);
}