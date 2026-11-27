using System;
using Dubito.Core;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

namespace Dubito.Presentation
{
    // Key moments outside the revelation (UX §5): pending victory banner, skipped turn and exclusion
    // messages, exclusion warning, end-of-game screen with Revanche / Quitter.
    public sealed class TableMessages : MonoBehaviour
    {
        const float TransientSeconds = 3.5f;

        [SerializeField] TMP_FontAsset font = null;

        public event Action RematchRequested;
        public event Action QuitRequested;

        GameObject canvas, banner, endScreen;
        TextMeshProUGUI bannerText, warningText, winnerText;
        string persistent = "", transient = "";
        float transientUntil;

        public void Render(PlayerView view)
        {
            EnsureUI();
            persistent = TableMessageText.PendingVictory(view);
            warningText.text = TableMessageText.ExclusionWarning(view);
            string winner = TableMessageText.Winner(view);
            endScreen.SetActive(winner.Length > 0);
            winnerText.text = winner;
            Refresh();
        }

        public void ShowTransient(string message)
        {
            if (string.IsNullOrEmpty(message)) return;
            EnsureUI();
            transient = message;
            transientUntil = Time.unscaledTime + TransientSeconds;
            Refresh();
        }

        public void Clear()
        {
            persistent = transient = "";
            if (canvas == null) return;
            warningText.text = "";
            endScreen.SetActive(false);
            Refresh();
        }

        void Update()
        {
            if (transient.Length > 0 && Time.unscaledTime >= transientUntil)
            {
                transient = "";
                Refresh();
            }
        }

        void Refresh()
        {
            string text = transient.Length > 0 ? transient : persistent;
            bannerText.text = text;
            banner.SetActive(text.Length > 0);
        }

        void EnsureUI()
        {
            if (canvas != null) return;
            canvas = new GameObject("MessagesCanvas", typeof(RectTransform), typeof(Canvas), typeof(CanvasScaler), typeof(GraphicRaycaster));
            canvas.transform.SetParent(transform, false);
            var c = canvas.GetComponent<Canvas>();
            c.renderMode = RenderMode.ScreenSpaceOverlay;
            c.sortingOrder = 15;
            var scaler = canvas.GetComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
            scaler.referenceResolution = new Vector2(1920, 1080);
            scaler.matchWidthOrHeight = .5f;

            // Banner in the upper middle of the screen, under the opposite player's name.
            banner = Panel("Banner", canvas.transform, new Vector2(0, 230), new Vector2(1100, 70), new Color(.04f, .05f, .07f, .88f));
            bannerText = Label(banner.transform, "", 32, Vector2.zero, new Vector2(1080, 70));
            warningText = Label(canvas.transform, "", 30, new Vector2(0, 160), new Vector2(1100, 50));
            warningText.color = new Color(1f, .75f, .35f);

            endScreen = Panel("EndScreen", canvas.transform, new Vector2(0, 0), new Vector2(720, 330), new Color(.04f, .05f, .07f, .95f));
            winnerText = Label(endScreen.transform, "", 64, new Vector2(0, 80), new Vector2(700, 90));
            winnerText.fontStyle = FontStyles.Bold;
            Button(endScreen.transform, "Revanche", new Vector2(-150, -70), () => RematchRequested?.Invoke());
            Button(endScreen.transform, "Quitter", new Vector2(150, -70), () => QuitRequested?.Invoke());
            banner.SetActive(false);
            endScreen.SetActive(false);
        }

        GameObject Panel(string objectName, Transform parent, Vector2 position, Vector2 size, Color color)
        {
            var go = new GameObject(objectName, typeof(RectTransform));
            go.transform.SetParent(parent, false);
            var rt = (RectTransform)go.transform;
            rt.anchorMin = rt.anchorMax = rt.pivot = new Vector2(.5f, .5f);
            rt.anchoredPosition = position;
            rt.sizeDelta = size;
            go.AddComponent<Image>().color = color;
            return go;
        }

        TextMeshProUGUI Label(Transform parent, string value, float size, Vector2 position, Vector2 box)
        {
            var go = new GameObject("Text", typeof(RectTransform));
            go.transform.SetParent(parent, false);
            var text = go.AddComponent<TextMeshProUGUI>();
            text.font = font != null ? font : TMP_Settings.defaultFontAsset;
            text.text = value;
            text.fontSize = size;
            text.alignment = TextAlignmentOptions.Center;
            text.raycastTarget = false;
            var rt = text.rectTransform;
            rt.anchorMin = rt.anchorMax = rt.pivot = new Vector2(.5f, .5f);
            rt.anchoredPosition = position;
            rt.sizeDelta = box;
            return text;
        }

        void Button(Transform parent, string label, Vector2 position, Action clicked)
        {
            var go = Panel(label, parent, position, new Vector2(240, 70), new Color(.25f, .29f, .33f));
            var button = go.AddComponent<Button>();
            button.navigation = new Navigation { mode = Navigation.Mode.None };
            button.onClick.AddListener(() => clicked());
            Label(go.transform, label, 30, Vector2.zero, new Vector2(240, 70));
        }
    }
}
