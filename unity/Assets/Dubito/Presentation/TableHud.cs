using System;
using System.Collections.Generic;
using Dubito.Core;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

namespace Dubito.Presentation
{
    public sealed class TableHud : MonoBehaviour
    {
        [SerializeField] TableSeating seating = null;
        [SerializeField] Camera viewCamera = null;
        [SerializeField] TMP_FontAsset font = null;
        [SerializeField] Vector3 opponentLabelOffset = new Vector3(0, 1.02f, 0.3f); // just above the opponent's fan (UX §2)
        const float LabelReferenceDistance = 2f;
        static Material labelOverlayMaterial; // one shared material: labels draw over bodies and cards
        Canvas canvas;
        TextMeshProUGUI round, pile, announcement, active;
        readonly TextMeshPro[] labels = new TextMeshPro[6];
        readonly Transform[] anchors = new Transform[6];

        public void Render(PlayerView view, IReadOnlyList<int> seatOrder, int localSeatIndex)
        {
            ViewSeats.Validate(view, seatOrder, localSeatIndex);
            if (seating == null) throw new InvalidOperationException("Assign seating before Render.");
            EnsureCanvas();
            round.text = view.RoundRank.HasValue ? CardNames.Rank(view.RoundRank.Value) : "Ouverture libre";
            pile.text = "Tas : " + CardNames.CardCount(view.PileCount);
            var last = view.LastAnnouncement;
            announcement.text = last == null ? "" : $"{CardNames.PlayerName(last.PlayerId)} : {last.CardCount} × {CardNames.Rank(last.AnnouncedRank)}";
            string who = view.ActivePlayerId == view.PlayerId ? "toi" : CardNames.PlayerName(view.ActivePlayerId);
            active.text = view.Phase == GamePhase.Finished ? "Partie terminée" :
                view.IsTimerPaused ? $"Révélation · chrono en pause" :
                $"À {who} de jouer · {Math.Max(0, (int)Math.Ceiling(view.RemainingTurnSeconds))} s";
            for (int i = 0; i < labels.Length; i++)
            {
                bool visible = i < seatOrder.Count && i != localSeatIndex;
                if (!visible) { if (labels[i] != null) labels[i].gameObject.SetActive(false); continue; }
                anchors[i] = seating.SeatAnchor(i);
                if (labels[i] == null) labels[i] = CreateLabel(i);
                int id = seatOrder[i];
                labels[i].text = $"{CardNames.PlayerName(id)} · {CardNames.CardCount(view.CardCounts[id])}";
                bool turn = id == view.ActivePlayerId && view.Phase != GamePhase.Finished;
                labels[i].fontStyle = turn ? FontStyles.Bold : FontStyles.Normal;
                labels[i].color = turn ? new Color(1, .9f, .45f) : Color.white;
                labels[i].gameObject.SetActive(true);
            }
            LateUpdate();
        }

        void EnsureCanvas()
        {
            if (canvas != null) return;
            var go = new GameObject("TableHudCanvas", typeof(RectTransform), typeof(Canvas), typeof(CanvasScaler));
            go.transform.SetParent(transform, false);
            canvas = go.GetComponent<Canvas>();
            canvas.renderMode = RenderMode.ScreenSpaceOverlay;
            var scaler = go.GetComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
            scaler.referenceResolution = new Vector2(1920, 1080);
            scaler.matchWidthOrHeight = .5f;
            round = CreateText("RoundRank", 44, -18, 60);
            pile = CreateText("PileCount", 28, -80, 40);
            // Turn and last announcement on the top-left, clear of the opposite player's label.
            active = CreateText("ActivePlayer", 30, -24, 44, true);
            announcement = CreateText("LastAnnouncement", 30, -72, 44, true);
        }

        TextMeshProUGUI CreateText(string objectName, float size, float y, float height, bool topLeft = false)
        {
            var go = new GameObject(objectName, typeof(RectTransform));
            go.transform.SetParent(canvas.transform, false);
            var text = go.AddComponent<TextMeshProUGUI>();
            text.font = font != null ? font : TMP_Settings.defaultFontAsset;
            text.fontSize = size;
            text.alignment = topLeft ? TextAlignmentOptions.Left : TextAlignmentOptions.Center;
            text.raycastTarget = false;
            text.color = Color.white;
            text.fontSharedMaterial = text.font.material;
            var rt = text.rectTransform;
            rt.anchorMin = rt.anchorMax = topLeft ? new Vector2(0, 1) : new Vector2(.5f, 1);
            rt.pivot = topLeft ? new Vector2(0, 1) : new Vector2(.5f, 1);
            rt.anchoredPosition = new Vector2(topLeft ? 32 : 0, y);
            rt.sizeDelta = new Vector2(1000, height);
            return text;
        }

        TextMeshPro CreateLabel(int seat)
        {
            var go = new GameObject($"OpponentLabel_{seat}");
            go.transform.SetParent(transform, false);
            var text = go.AddComponent<TextMeshPro>();
            text.font = font != null ? font : TMP_Settings.defaultFontAsset;
            // World-space label: about 5 cm tall letters, one line, readable from across the table.
            text.fontSize = 0.6f;
            text.alignment = TextAlignmentOptions.Center;
            text.textWrappingMode = TextWrappingModes.NoWrap;
            if (labelOverlayMaterial == null)
            {
                var overlay = Shader.Find("TextMeshPro/Distance Field Overlay");
                if (overlay != null) labelOverlayMaterial = new Material(text.font.material) { shader = overlay };
            }
            if (labelOverlayMaterial != null) text.fontSharedMaterial = labelOverlayMaterial;
            text.rectTransform.sizeDelta = new Vector2(1f, .1f);
            text.raycastTarget = false;
            return text;
        }

        void LateUpdate()
        {
            if (viewCamera == null) viewCamera = Camera.main;
            for (int i = 0; i < labels.Length; i++)
            {
                if (labels[i] == null || !labels[i].gameObject.activeSelf || anchors[i] == null) continue;
                labels[i].transform.position = anchors[i].TransformPoint(opponentLabelOffset);
                if (viewCamera == null) continue;
                labels[i].transform.rotation = viewCamera.transform.rotation;
                // Same apparent size for near and far seats.
                float distance = Vector3.Distance(viewCamera.transform.position, labels[i].transform.position);
                labels[i].transform.localScale = Vector3.one * (distance / LabelReferenceDistance);
            }
        }

        void OnDisable()
        {
            if (canvas != null) canvas.gameObject.SetActive(false);
            foreach (var label in labels) if (label != null) label.gameObject.SetActive(false);
        }
        void OnEnable() { if (canvas != null) canvas.gameObject.SetActive(true); }
    }
}
