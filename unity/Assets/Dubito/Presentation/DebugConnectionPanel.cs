using System;
using Dubito.Networking;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

namespace Dubito.Presentation
{
    // Temporary localhost controls for 3.2. The complete ready/settings lobby is 3.4.
    public sealed class DebugConnectionPanel : MonoBehaviour
    {
        [SerializeField] DubitoNetworkManager network = null;
        [SerializeField] LocalTableController table = null;
        [SerializeField] TMP_FontAsset font = null;
        GameObject canvas;
        TextMeshProUGUI status;
        Button local, host, join, launch, back;
        bool localMode;
        string failure;

        void Start()
        {
            BuildUI();
            network.SessionCreated += OnSession;
            network.Disconnected += OnDisconnected;
            table.Suspend();
        }

        void OnSession(NetworkGameSession session)
        {
            localMode = false;
            table.UseNetworkSession(session);
        }

        void OnDisconnected(string message)
        {
            failure = message;
            localMode = false;
            table.Suspend();
            canvas.SetActive(true);
        }

        void Update()
        {
            if (canvas == null) return;
            bool playing = localMode || network.Session?.IsReady == true;
            canvas.SetActive(!playing);
            if (playing) return;
            bool busy = network.IsConnectingOrConnected || network.IsHosting;
            local.gameObject.SetActive(!busy);
            host.gameObject.SetActive(!busy);
            join.gameObject.SetActive(!busy);
            launch.gameObject.SetActive(network.IsHosting);
            launch.interactable = network.CanStartMatch;
            back.gameObject.SetActive(busy);
            status.text = network.IsHosting ? $"{network.ConnectedPlayers} joueur(s) connecté(s)" : failure ?? network.Status;
        }

        void BuildUI()
        {
            canvas = new GameObject("DebugConnectionCanvas", typeof(RectTransform), typeof(Canvas), typeof(CanvasScaler), typeof(GraphicRaycaster));
            canvas.transform.SetParent(transform, false);
            var c = canvas.GetComponent<Canvas>();
            c.renderMode = RenderMode.ScreenSpaceOverlay;
            c.sortingOrder = 100;
            var scaler = canvas.GetComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
            scaler.referenceResolution = new Vector2(1920, 1080);
            scaler.matchWidthOrHeight = .5f;
            var background = new GameObject("Background", typeof(RectTransform), typeof(Image));
            background.transform.SetParent(canvas.transform, false);
            var rect = (RectTransform)background.transform;
            rect.anchorMin = Vector2.zero; rect.anchorMax = Vector2.one;
            rect.offsetMin = rect.offsetMax = Vector2.zero;
            background.GetComponent<Image>().color = new Color(.035f, .045f, .055f, .96f);
            Text("Dubito — connexion locale (debug)", 230, 45);
            status = Text("", 135, 30);
            local = MakeButton("Jeu local (hot-seat)", 35, () => { localMode = true; failure = null; table.StartLocal(); });
            host = MakeButton("Héberger", -50, () => { failure = null; network.HostLocal(); });
            join = MakeButton("Rejoindre 127.0.0.1", -135, () => { failure = null; network.JoinLocal(); });
            launch = MakeButton("Lancer la partie", 0, () => network.StartMatch());
            back = MakeButton("Annuler / retour", -100, () => { network.StopSession(); table.Suspend(); failure = null; });
        }

        TextMeshProUGUI Text(string value, float y, float size)
        {
            var go = new GameObject("Text", typeof(RectTransform), typeof(TextMeshProUGUI));
            go.transform.SetParent(canvas.transform, false);
            var text = go.GetComponent<TextMeshProUGUI>();
            text.font = font;
            text.text = value;
            text.fontSize = size;
            text.alignment = TextAlignmentOptions.Center;
            text.raycastTarget = false;
            var rect = text.rectTransform;
            rect.anchorMin = rect.anchorMax = rect.pivot = new Vector2(.5f, .5f);
            rect.anchoredPosition = new Vector2(0, y);
            rect.sizeDelta = new Vector2(1300, 100);
            return text;
        }

        Button MakeButton(string label, float y, Action clicked)
        {
            var go = new GameObject(label, typeof(RectTransform), typeof(Image), typeof(Button));
            go.transform.SetParent(canvas.transform, false);
            var rect = (RectTransform)go.transform;
            rect.anchorMin = rect.anchorMax = rect.pivot = new Vector2(.5f, .5f);
            rect.anchoredPosition = new Vector2(0, y);
            rect.sizeDelta = new Vector2(550, 64);
            go.GetComponent<Image>().color = new Color(.22f, .29f, .33f);
            var button = go.GetComponent<Button>();
            button.navigation = new Navigation { mode = Navigation.Mode.None };
            button.onClick.AddListener(() => clicked());
            var text = Text(label, 0, 30);
            text.transform.SetParent(go.transform, false);
            text.rectTransform.anchoredPosition = Vector2.zero;
            text.rectTransform.sizeDelta = rect.sizeDelta;
            return button;
        }

        void OnDestroy()
        {
            if (network == null) return;
            network.SessionCreated -= OnSession;
            network.Disconnected -= OnDisconnected;
        }
    }
}
