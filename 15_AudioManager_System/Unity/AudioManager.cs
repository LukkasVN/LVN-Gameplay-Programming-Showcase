using UnityEngine;
using UnityEngine.Audio;
using System.Collections;
using System.Collections.Generic;

public class AudioManager : MonoBehaviour
{
    public static AudioManager Instance;

    [Header("Mixer")]
    [SerializeField] private AudioMixer masterMixer;

    [Header("Exposed Mixer Parameters")]
    [SerializeField] public string masterParam = "MasterVolume";
    [SerializeField] public string musicParam = "MusicVolume";
    [SerializeField] public string sfxParam = "SFXVolume";
    [SerializeField] public string uiParam = "UIVolume";

    [Header("Audio Sources")]
    [SerializeField] private AudioSource stereoMusicSourceA;
    [SerializeField] private AudioSource stereoMusicSourceB;
    [SerializeField] private AudioSource stereoSfxSource;
    [SerializeField] private AudioSource stereoUiSource;

    [Header("Crossfade")]
    [SerializeField] private float fadeTime = 1f;
    private Coroutine crossfadeRoutine;

    private AudioSource activeMusicSource;
    private AudioSource inactiveMusicSource;

    private float defaultPitch = 1f;

    [Header("Volume Curve")]
    [SerializeField]
    private AnimationCurve volumeCurve = new AnimationCurve(
        new Keyframe(0f, -80f),
        new Keyframe(0.01f, -30f),
        new Keyframe(0.5f, -6f),
        new Keyframe(1f, 0f)
    );

    [Header("3D SFX Pool")]
    [SerializeField] private int poolSize = 10;
    private Queue<AudioSource> sfx3DPool = new Queue<AudioSource>();


    private void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject);

            activeMusicSource = stereoMusicSourceA;
            inactiveMusicSource = stereoMusicSourceB;

            InitializePool();
        }
        else
        {
            Destroy(gameObject);
        }
    }

    #region Pool
    private void InitializePool()
    {
        for (int i = 0; i < poolSize; i++)
        {
            GameObject obj = new GameObject("Pooled3DAudio");
            obj.transform.parent = transform;

            AudioSource a = obj.AddComponent<AudioSource>();
            a.spatialBlend = 1f;
            a.playOnAwake = false;
            a.outputAudioMixerGroup = stereoSfxSource.outputAudioMixerGroup;

            obj.SetActive(false);
            sfx3DPool.Enqueue(a);
        }
    }

    private AudioSource GetPooled3DSource()
    {
        AudioSource src = sfx3DPool.Dequeue();
        sfx3DPool.Enqueue(src);
        return src;
    }
    #endregion


    #region SaveLoad
    public void SaveVolumes(int master, int music, int sfx, int ui)
    {
        PlayerPrefs.SetInt("MasterAudio", master);
        PlayerPrefs.SetInt("MusicAudio", music);
        PlayerPrefs.SetInt("SFXAudio", sfx);
        PlayerPrefs.SetInt("UIAudio", ui);
        PlayerPrefs.Save();
    }

    public (int master, int music, int sfx, int ui) LoadVolumes()
    {
        int master = PlayerPrefs.GetInt("MasterAudio", 100);
        int music = PlayerPrefs.GetInt("MusicAudio", 100);
        int sfx = PlayerPrefs.GetInt("SFXAudio", 100);
        int ui = PlayerPrefs.GetInt("UIAudio", 100);

        return (master, music, sfx, ui);
    }
    #endregion


    #region Mixer
    public void SetMixerVolume(int volumePercent, string exposedName)
    {
        float normalized = volumePercent / 100f;
        float dB = volumeCurve.Evaluate(normalized);
        masterMixer.SetFloat(exposedName, dB);
    }
    #endregion


    #region Music & Crossfade
    public void PlayMusic(AudioClip clip)
    {
        if (clip == null) return;

        inactiveMusicSource.clip = clip;
        inactiveMusicSource.Play();

        if (crossfadeRoutine != null)
            StopCoroutine(crossfadeRoutine);

        crossfadeRoutine = StartCoroutine(CrossfadeMusic(fadeTime));
    }


    private IEnumerator CrossfadeMusic(float duration)
    {
        float time = 0f;

        while (time < duration)
        {
            float t = time / duration;
            activeMusicSource.volume = Mathf.Lerp(1f, 0f, t);
            inactiveMusicSource.volume = Mathf.Lerp(0f, 1f, t);

            time += Time.deltaTime;
            yield return null;
        }

        activeMusicSource.Stop();

        var temp = activeMusicSource;
        activeMusicSource = inactiveMusicSource;
        inactiveMusicSource = temp;

        activeMusicSource.volume = 1f;
        inactiveMusicSource.volume = 0f;
    }

    public void StopMusic(float fadeTime = 1f)
    {
        if (crossfadeRoutine != null)
        {
            StopCoroutine(crossfadeRoutine);
            crossfadeRoutine = null;
        }

        if (fadeTime <= 0f)
        {
            activeMusicSource.Stop();
            inactiveMusicSource.Stop();
            activeMusicSource.volume = 1f;
            inactiveMusicSource.volume = 0f;
            return;
        }

        crossfadeRoutine = StartCoroutine(FadeOutMusic(fadeTime));
    }


    private IEnumerator FadeOutMusic(float duration)
    {
        float startVolume = activeMusicSource.volume;
        float time = 0f;

        while (time < duration)
        {
            activeMusicSource.volume = Mathf.Lerp(startVolume, 0f, time / duration);
            inactiveMusicSource.volume = Mathf.Lerp(inactiveMusicSource.volume, 0f, time / duration);
            time += Time.deltaTime;
            yield return null;
        }

        activeMusicSource.Stop();
        inactiveMusicSource.Stop();

        activeMusicSource.volume = 1f;
        inactiveMusicSource.volume = 0f;

        crossfadeRoutine = null;
    }


    #endregion


    #region SFX
    public void PlaySFX(AudioClip clip)
    {
        if (clip == null) return;
        stereoSfxSource.pitch = defaultPitch;
        stereoSfxSource.PlayOneShot(clip);
    }

    public void PlaySFXRandomPitch(AudioClip clip)
    {
        if (clip == null) return;
        stereoSfxSource.pitch = Random.Range(defaultPitch - 0.3f, defaultPitch + 0.3f);
        stereoSfxSource.PlayOneShot(clip);
        stereoSfxSource.pitch = defaultPitch;
    }

    public void PlayAtPosition(AudioClip clip, Vector3 position, float pitchRange = 0.3f)
    {
        if (clip == null) return;

        AudioSource src = GetPooled3DSource();
        src.transform.position = position;

        src.pitch = Random.Range(defaultPitch - pitchRange, defaultPitch + pitchRange);

        src.gameObject.SetActive(true);
        src.PlayOneShot(clip);

        StartCoroutine(DisableAfter(src, clip.length));
    }

    private IEnumerator DisableAfter(AudioSource src, float delay)
    {
        yield return new WaitForSeconds(delay);
        src.gameObject.SetActive(false);
    }
    #endregion


    #region UI
    public void PlayUISound(AudioClip clip)
    {
        if (clip == null) return;
        stereoUiSource.pitch = defaultPitch;
        stereoUiSource.PlayOneShot(clip);
    }

    public void PlayUISoundRandomPitch(AudioClip clip)
    {
        if (clip == null) return;
        stereoUiSource.pitch = Random.Range(defaultPitch - 0.3f, defaultPitch + 0.3f);
        stereoUiSource.PlayOneShot(clip);
        stereoUiSource.pitch = defaultPitch;
    }
    #endregion
}
