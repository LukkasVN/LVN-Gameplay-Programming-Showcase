using UnityEngine;
using TMPro;
using System.Diagnostics;

public class GameRadioUI : MonoBehaviour
{
    private GameRadio currentGameRadio;
    [SerializeField] private TextMeshProUGUI stationNameText;
    [SerializeField] private TextMeshProUGUI trackNameText;
    [SerializeField] private GameObject radioMenuPanel;
    private string musicPath;

    private void Start()
    {
        musicPath = System.IO.Path.Combine(Application.persistentDataPath, "GameCustomRadioMusic");
    }

    public void SetActiveRadio(GameRadio gameRadio)
    {
        currentGameRadio = gameRadio;
        if (currentGameRadio == null) return;

        UpdateDisplay();
        if (radioMenuPanel != null)
            radioMenuPanel.SetActive(true);
    }

    public void OnOpenCustomMusicFolder()
    {
        if (currentGameRadio == null) return;
        Process.Start(musicPath);
    }

    public void OnRefresh()
    {
        if (currentGameRadio == null) return;

        currentGameRadio.RefreshCustomStation(onComplete: () => UpdateDisplay());
    }

    public void OnPreviousStation()
    {
        if (currentGameRadio == null || currentGameRadio.GetStationCount() <= 1) return;
        int newIndex = currentGameRadio.GetCurrentStationIndex() - 1;
        if (newIndex < 0) newIndex = currentGameRadio.GetStationCount() - 1;
        currentGameRadio.ChangeStation(newIndex);
        UpdateDisplay();
    }

    public void OnNextStation()
    {
        if (currentGameRadio == null || currentGameRadio.GetStationCount() <= 1) return;
        int newIndex = currentGameRadio.GetCurrentStationIndex() + 1;
        if (newIndex >= currentGameRadio.GetStationCount()) newIndex = 0;
        currentGameRadio.ChangeStation(newIndex);
        UpdateDisplay();
    }

    public void OnPreviousTrack()
    {
        if (currentGameRadio == null) return;
        currentGameRadio.PreviousTrack();
        UpdateDisplay();
    }

    public void OnNextTrack()
    {
        if (currentGameRadio == null) return;
        currentGameRadio.NextTrack();
        UpdateDisplay();
    }

    public void OnPlayPressed()
    {
        if (currentGameRadio == null) return;
        currentGameRadio.PlayRadio();
        UpdateDisplay();
    }

    public void OnTurnOffRadio()
    {
        if (currentGameRadio == null) return;
        currentGameRadio.TurnOffRadio();
        UpdateDisplay();
    }

    public void ToggleRadioMenu()
    {
        if (radioMenuPanel != null)
            radioMenuPanel.SetActive(!radioMenuPanel.activeSelf);
    }

    private void UpdateDisplay()
    {
        if (currentGameRadio == null) return;

        if (stationNameText != null)
            stationNameText.text = currentGameRadio.GetCurrentStationName();

        if (trackNameText != null)
            trackNameText.text = currentGameRadio.IsPlaying()
                ? currentGameRadio.GetCurrentTrackName()
                : "???";
    }
}