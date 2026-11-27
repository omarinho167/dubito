using System;
using Dubito.Networking;
using Dubito.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

namespace Dubito.Presentation
{
    // Step 3.4 (UX §6.2): start screen, local lobby (players, "Prêt", host settings, launch),
    // and the way back after a game. The Steam invitation and the Rules screen come later (4.x, 5.1).
    public sealed class LobbyScreen : MonoBehaviour
    {
        static readonly int[] TimerChoices = { 30, 45, 60, 90 }; // RULES §5

        [SerializeField] DubitoNetworkManager network = null;
        [SerializeField] LocalTableController table = null;
        [SerializeField] TMP_FontAsset font = null;

        GameObject canvas, startPage, lobbyPage, waitPage, hostSettings;
        TextMeshProUGUI status, lobbyTitle, lobbyInfo, lobbyHint, timerLabel, seatsLabel, waitText;
        readonly TextMeshProUGUI[] playerLines = new TextMeshProUGUI[6];
        Button readyButton, launchButton;
        TextMeshProUGUI readyLabel;
        bool localMode, tableBound;
        string failure;

        void Start()
        {
            BuildUI();
            network.Disconnected += OnDisconnected;
            table.LeaveRequested += Leave;
            table.NetworkRematchRequested += network.Rematch;
            table.Suspend();
        }

        void OnDisconnected(string message)
        {
            failure = message;
            localMode = false;
            tableBound = false;
            table.Suspend();
        }

        void Leave()
        {
            if (!localMode) network.StopSession();
            localMode = false;
            tableBound = false;
            failure = null;
            table.Suspend();
        }

        void Update()
        {
            if (canvas == null) return;
            var session = network.Session;
            bool inGame = localMode || session?.IsReady == true;
            if (session?.IsReady == true && !tableBound)
            {
                table.UseNetworkSession(session, network.IsHosting);
                tableBound = true;
            }
            else if (session?.Lobby != null && tableBound)
            {
                table.Suspend(); // "Revanche": back to the lobby
                tableBound = false;
            }

            canvas.SetActive(!inGame);
            if (inGame) return;

            bool busy = network.IsHosting || network.IsConnectingOrConnected;
            var lobby = session?.Lobby;
            startPage.SetActive(!busy);
            lobbyPage.SetActive(busy && lobby != null);
            waitPage.SetActive(busy && lobby == null);
            if (!busy) status.text = failure ?? "Choisis un mode de jeu.";
            else if (lobby == null) waitText.text = network.Status;
            else RenderLobby(lobby);
        }

        void RenderLobby(LobbyStateMessage lobby)
        {
            bool host = network.IsHosting;
            lobbyTitle.text = "Salon";
            for (int i = 0; i < playerLines.Length; i++)
            {
                bool used = i < lobby.Players.Count;
                playerLines[i].gameObject.SetActive(used);
                if (!used) continue;
                var p = lobby.Players[i];
                string who = p.PlayerName + (p.PlayerId == lobby.YourPlayerId ? " (toi)" : "");
                string state = p.PlayerId == lobby.HostPlayerId ? "hôte" : p.IsReady ? "prêt" : "pas prêt";
                playerLines[i].text = $"{who} · {state}";
                playerLines[i].color = p.PlayerId == lobby.HostPlayerId || p.IsReady ? new Color(.6f, .95f, .65f) : new Color(1f, .8f, .5f);
            }
            lobbyInfo.text = $"Chrono : {lobby.TurnDurationSeconds} s · Places : {lobby.Players.Count}/{lobby.MaxPlayers}";

            hostSettings.SetActive(host);
            timerLabel.text = $"Chrono : {network.TurnSeconds} s";
            seatsLabel.text = $"Places max : {network.MaxPlayers}";
            launchButton.gameObject.SetActive(host);
            launchButton.interactable = network.CanStartMatch;
            readyButton.gameObject.SetActive(!host);
            bool meReady = false;
            foreach (var p in lobby.Players) if (p.PlayerId == lobby.YourPlayerId) meReady = p.IsReady;
            readyLabel.text = meReady ? "Finalement, pas prêt" : "Je suis prêt";
            lobbyHint.text = host
                ? lobby.Players.Count < 2 ? "Il faut au moins 2 joueurs." :
                  network.CanStartMatch ? "Tout le monde est prêt." : "En attente : tous les joueurs doivent être prêts."
                : meReady ? "En attente du lancement par l'hôte…" : "Clique sur « Je suis prêt » quand tu es prêt.";
        }

        void ChangeTimer(int direction)
        {
            int index = Array.IndexOf(TimerChoices, network.TurnSeconds);
            if (index < 0) index = 1;
            index = Mathf.Clamp(index + direction, 0, TimerChoices.Length - 1);
            network.SetLobbySettings(TimerChoices[index], network.MaxPlayers);
        }

        void ChangeSeats(int direction)
        {
            int present = network.Session?.Lobby?.Players.Count ?? 1;
            int seats = Mathf.Clamp(network.MaxPlayers + direction, Mathf.Max(2, present), 6);
            network.SetLobbySettings(network.TurnSeconds, seats);
        }

        void ToggleReady()
        {
            var lobby = network.Session?.Lobby;
            if (lobby == null) return;
            foreach (var p in lobby.Players)
                if (p.PlayerId == lobby.YourPlayerId) network.Session.SetReady(!p.IsReady);
        }

        // ---------- UI ----------

        void BuildUI()
        {
            canvas = new GameObject("LobbyCanvas", typeof(RectTransform), typeof(Canvas), typeof(CanvasScaler), typeof(GraphicRaycaster));
            canvas.transform.SetParent(transform, false);
            var c = canvas.GetComponent<Canvas>();
            c.renderMode = RenderMode.ScreenSpaceOverlay;
            c.sortingOrder = 100;
            var scaler = canvas.GetComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
            scaler.referenceResolution = new Vector2(1920, 1080);
            scaler.matchWidthOrHeight = .5f;
            var background = Panel("Background", canvas.transform, Vector2.zero, Vector2.zero, new Color(.035f, .045f, .055f, .96f));
            var bg = (RectTransform)background.transform;
            bg.anchorMin = Vector2.zero; bg.anchorMax = Vector2.one; bg.offsetMin = bg.offsetMax = Vector2.zero;

            startPage = Page("Start");
            Label(startPage.transform, "Dubito", new Vector2(0, 300), 80, FontStyles.Bold);
            status = Label(startPage.transform, "", new Vector2(0, 200), 30);
            MakeButton(startPage.transform, "Créer une partie", new Vector2(0, 90), () => { failure = null; network.HostLocal(); });
            MakeButton(startPage.transform, "Rejoindre (127.0.0.1)", new Vector2(0, 5), () => { failure = null; network.JoinLocal(); });
            MakeButton(startPage.transform, "Jeu local (hot-seat)", new Vector2(0, -80), () => { failure = null; localMode = true; table.StartLocal(); });
            MakeButton(startPage.transform, "Quitter", new Vector2(0, -165), QuitApplication);

            waitPage = Page("Wait");
            waitText = Label(waitPage.transform, "", new Vector2(0, 60), 34);
            MakeButton(waitPage.transform, "Annuler", new Vector2(0, -60), Leave);

            lobbyPage = Page("Lobby");
            lobbyTitle = Label(lobbyPage.transform, "Salon", new Vector2(0, 380), 60, FontStyles.Bold);
            for (int i = 0; i < playerLines.Length; i++)
                playerLines[i] = Label(lobbyPage.transform, "", new Vector2(0, 280 - i * 50), 32);
            lobbyInfo = Label(lobbyPage.transform, "", new Vector2(0, -40), 28);

            hostSettings = Page("HostSettings", lobbyPage.transform);
            timerLabel = Label(hostSettings.transform, "", new Vector2(0, -110), 30);
            MakeButton(hostSettings.transform, "‹", new Vector2(-230, -110), () => ChangeTimer(-1), 64);
            MakeButton(hostSettings.transform, "›", new Vector2(230, -110), () => ChangeTimer(1), 64);
            seatsLabel = Label(hostSettings.transform, "", new Vector2(0, -180), 30);
            MakeButton(hostSettings.transform, "‹", new Vector2(-230, -180), () => ChangeSeats(-1), 64);
            MakeButton(hostSettings.transform, "›", new Vector2(230, -180), () => ChangeSeats(1), 64);

            launchButton = MakeButton(lobbyPage.transform, "Lancer la partie", new Vector2(0, -270), () => network.StartMatch());
            readyButton = MakeButton(lobbyPage.transform, "Je suis prêt", new Vector2(0, -270), ToggleReady);
            readyLabel = readyButton.GetComponentInChildren<TextMeshProUGUI>();
            lobbyHint = Label(lobbyPage.transform, "", new Vector2(0, -340), 26);
            MakeButton(lobbyPage.transform, "Quitter le salon", new Vector2(0, -420), Leave);
        }

        void QuitApplication()
        {
#if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;
#else
            Application.Quit();
#endif
        }

        GameObject Page(string objectName, Transform parent = null)
        {
            var go = new GameObject(objectName, typeof(RectTransform));
            go.transform.SetParent(parent ?? canvas.transform, false);
            var rt = (RectTransform)go.transform;
            rt.anchorMin = Vector2.zero; rt.anchorMax = Vector2.one; rt.offsetMin = rt.offsetMax = Vector2.zero;
            return go;
        }

        GameObject Panel(string objectName, Transform parent, Vector2 position, Vector2 size, Color color)
        {
            var go = new GameObject(objectName, typeof(RectTransform), typeof(Image));
            go.transform.SetParent(parent, false);
            var rt = (RectTransform)go.transform;
            rt.anchorMin = rt.anchorMax = rt.pivot = new Vector2(.5f, .5f);
            rt.anchoredPosition = position;
            rt.sizeDelta = size;
            go.GetComponent<Image>().color = color;
            return go;
        }

        TextMeshProUGUI Label(Transform parent, string value, Vector2 position, float size, FontStyles style = FontStyles.Normal)
        {
            var go = new GameObject("Text", typeof(RectTransform), typeof(TextMeshProUGUI));
            go.transform.SetParent(parent, false);
            var text = go.GetComponent<TextMeshProUGUI>();
            text.font = font;
            text.text = value;
            text.fontSize = size;
            text.fontStyle = style;
            text.alignment = TextAlignmentOptions.Center;
            text.raycastTarget = false;
            var rt = text.rectTransform;
            rt.anchorMin = rt.anchorMax = rt.pivot = new Vector2(.5f, .5f);
            rt.anchoredPosition = position;
            rt.sizeDelta = new Vector2(1300, size * 1.6f);
            return text;
        }

        Button MakeButton(Transform parent, string label, Vector2 position, Action clicked, float width = 550)
        {
            var go = Panel(label, parent, position, new Vector2(width, 64), new Color(.22f, .29f, .33f));
            var button = go.AddComponent<Button>();
            button.navigation = new Navigation { mode = Navigation.Mode.None };
            button.onClick.AddListener(() => clicked());
            var text = Label(go.transform, label, Vector2.zero, 30);
            text.rectTransform.sizeDelta = new Vector2(width, 64);
            return button;
        }

        void OnDestroy()
        {
            if (network != null) network.Disconnected -= OnDisconnected;
            if (table != null)
            {
                table.LeaveRequested -= Leave;
                if (network != null) table.NetworkRematchRequested -= network.Rematch;
            }
        }
    }
}
