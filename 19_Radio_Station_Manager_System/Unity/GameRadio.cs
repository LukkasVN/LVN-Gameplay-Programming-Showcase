using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class GameRadio : MonoBehaviour
{
    private AudioSource audioSource;
    private Animator gameAnimator;
    [SerializeField] private ParticleSystem radioParticles;
    [SerializeField] private GameRadioManager manager;
    [SerializeField] private float fadeDuration = 1f;

    private int currentStationIndex = 0;
    private bool isPlaying = false;
    private List<AudioClip> songHistory = new List<AudioClip>();
    private Coroutine fadeCoroutine;
    private float startVolume;

    private void Awake()
    {
        audioSource = GetComponent<AudioSource>();
        gameAnimator = GetComponent<Animator>();

        if (audioSource != null)
            startVolume = audioSource.volume;
    }

    private void OnEnable()
    {
        GameRadioManager.TrackChanged += OnStationTrackChanged;
    }

    private void OnDisable()
    {
        GameRadioManager.TrackChanged -= OnStationTrackChanged;
    }

    private void Update()
    {
        if (isPlaying && audioSource != null && audioSource.isPlaying)
            manager.UpdateStationTime(currentStationIndex, audioSource.time, true);

        if (isPlaying && audioSource != null && !audioSource.isPlaying)
            PlayNextTrackInStation();
    }

    public void ChangeStation(int stationIndex)
    {
        if (manager == null || stationIndex < 0 || stationIndex >= manager.Stations.Count) return;

        if (isPlaying)
            StopRadioWithFade();
        else
            audioSource?.Stop();

        isPlaying = false;
        currentStationIndex = stationIndex;
        songHistory.Clear();

        manager.SetStationActive(stationIndex, audioSource);

        var stationClip = manager.GetActiveStationClip(currentStationIndex);
        if (stationClip != null)
        {
            audioSource.clip = stationClip;
            audioSource.time = manager.GetStationTime(currentStationIndex);
            PlayRadioWithFade();
        }
        else
        {
            PlayNextTrackInStation();
        }
    }

    public void PlayRadio()
    {
        if (audioSource == null || manager == null || manager.Stations.Count == 0) return;
        if (manager.Stations[currentStationIndex].tracks.Count == 0) return;

        if (!manager.IsStationActive(currentStationIndex))
            manager.SetStationActive(currentStationIndex, audioSource);

        var stationClip = manager.GetActiveStationClip(currentStationIndex);
        if (stationClip != null)
        {
            audioSource.clip = stationClip;
            audioSource.time = manager.GetStationTime(currentStationIndex);

            if (songHistory.Count == 0 || songHistory[songHistory.Count - 1] != stationClip)
                songHistory.Add(stationClip);
        }
        else
        {
            PlayNextTrackInStation();
            return;
        }

        PlayRadioWithFade();
    }

    public void TurnOffRadio()
    {
        if (audioSource == null) return;
        StopRadioWithFade();
    }

    public void RefreshCustomStation(System.Action onComplete = null)
    {
        if (manager == null) return;

        if (currentStationIndex == manager.customStationIndex && isPlaying)
            StopRadioWithFade();

        manager.RefreshCustomStation(onComplete: () => onComplete?.Invoke());
    }

    private void PlayRadioWithFade()
    {
        if (fadeCoroutine != null)
            StopCoroutine(fadeCoroutine);

        audioSource.volume = 0f;
        audioSource.Play();
        isPlaying = true;

        fadeCoroutine = StartCoroutine(FadeVolume(0f, startVolume, fadeDuration));

        if (gameAnimator != null)
            gameAnimator.SetBool("isPlaying", isPlaying);

        if (radioParticles != null && !radioParticles.isPlaying)
            radioParticles.Play();
    }

    private void StopRadioWithFade()
    {
        if (fadeCoroutine != null)
            StopCoroutine(fadeCoroutine);

        isPlaying = false;

        fadeCoroutine = StartCoroutine(FadeVolume(audioSource.volume, 0f, fadeDuration, () =>
            audioSource.Stop()));

        if (gameAnimator != null)
            gameAnimator.SetBool("isPlaying", isPlaying);

        if (radioParticles != null && radioParticles.isPlaying)
            radioParticles.Stop();
    }

    private IEnumerator FadeVolume(float startVol, float endVol, float duration, System.Action onComplete = null)
    {
        float elapsed = 0f;

        while (elapsed < duration)
        {
            elapsed += Time.deltaTime;
            audioSource.volume = Mathf.Lerp(startVol, endVol, elapsed / duration);
            yield return null;
        }

        audioSource.volume = endVol;
        onComplete?.Invoke();
    }

    public void NextTrack()
    {
        if (audioSource != null)
            audioSource.Stop();
        SkipToNextTrack();
    }

    public void PreviousTrack()
    {
        if (audioSource == null || manager == null || songHistory.Count < 2) return;

        audioSource.Stop();
        songHistory.RemoveAt(songHistory.Count - 1);
        AudioClip previousClip = songHistory[songHistory.Count - 1];

        manager.RegisterTrack(currentStationIndex, previousClip);
        audioSource.clip = previousClip;
        audioSource.time = 0f;
        audioSource.Play();

        isPlaying = true;

        if (gameAnimator != null)
            gameAnimator.SetBool("isPlaying", isPlaying);

        if (radioParticles != null && !radioParticles.isPlaying)
            radioParticles.Play();

        manager.BroadcastTrackChange(currentStationIndex, previousClip);
    }

    private void PlayNextTrackInStation()
    {
        if (manager == null || currentStationIndex < 0 || currentStationIndex >= manager.Stations.Count) return;

        var station = manager.Stations[currentStationIndex];
        if (station.tracks.Count == 0) return;

        if (!manager.IsStationActive(currentStationIndex))
            manager.SetStationActive(currentStationIndex, audioSource);

        AudioClip newClip = station.tracks[Random.Range(0, station.tracks.Count)];

        songHistory.Add(newClip);
        audioSource.clip = newClip;
        manager.RegisterTrack(currentStationIndex, newClip);

        PlayRadioWithFade();
    }

    private void SkipToNextTrack()
    {
        if (manager == null || currentStationIndex < 0 || currentStationIndex >= manager.Stations.Count) return;

        var station = manager.Stations[currentStationIndex];
        if (station.tracks.Count == 0) return;

        AudioClip currentClip = audioSource.clip;
        AudioClip newClip = null;
        int attempts = 0;
        const int MAX_ATTEMPTS = 5;

        do
        {
            newClip = station.tracks[Random.Range(0, station.tracks.Count)];
            attempts++;
        } while (newClip == currentClip && attempts < MAX_ATTEMPTS);

        songHistory.Add(newClip);
        audioSource.clip = newClip;
        manager.RegisterTrack(currentStationIndex, newClip);

        PlayRadioWithFade();
        manager.BroadcastTrackChange(currentStationIndex, newClip);
    }

    private void OnStationTrackChanged(int stationIndex, AudioClip newClip)
    {
        if (currentStationIndex != stationIndex || !isPlaying) return;

        audioSource.clip = newClip;
        audioSource.time = 0f;
        audioSource.Play();
        songHistory.Add(newClip);
    }

    public string GetCurrentStationName() => manager != null ? manager.Stations[currentStationIndex].stationName : "None";
    public string GetCurrentTrackName() => audioSource?.clip?.name ?? "None";
    public int GetCurrentStationIndex() => currentStationIndex;
    public int GetStationCount() => manager != null ? manager.Stations.Count : 0;
    public bool IsPlaying() => isPlaying;
}