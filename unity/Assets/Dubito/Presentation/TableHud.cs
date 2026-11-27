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
        [SerializeField] Vector3 opponentLabelOffset = new Vector3(0, 1.38f, 0f); // on the head: name, card count below (owner 2026-11-27)
        const float LabelReferenceDistance = 2f;
        static Material labelOverlayMaterial; // one shared material: labels draw over bodies and cards
        public float TimerDisplayScale { get; set; } = 1f;
        Canvas canvas;
        TextMeshProUGUI round, pile, announcement, active, ownHand;
        enum Corner { Left, Right }
        readonly TextMeshPro[] labels = new TextMeshPro[6];
        readonly Transform[] anchors = new Transform[6];

        public void Render(PlayerView view, IReadOnlyList<int> seatOrder, int localSeatIndex, double? remainingTurnSeconds = null)
        {
            ViewSeats.Validate(view, seatOrder, localSeatIndex);
            if (seating == null) throw new InvalidOperationException("Assign seating before Render.");
            EnsureCanvas();
            round.text = view.RoundRank.HasValue ? CardNames.Rank(view.RoundRank.Value) : "Ouverture libre";
            pile.text = view.PileCount == 0 ? "Tas au centre : vide" : "Tas au centre : " + CardNames.CardCount(view.PileCount);
            ownHand.text = PlayerVoice.HotSeat
                ? $"Main de {CardNames.PlayerName(view.PlayerId)} : {CardNames.CardCount(view.Hand.Count)}"
                : $"Ta main : {CardNames.CardCount(view.Hand.Count)}";
            var last = view.LastAnnouncement;
            string author = last == null ? "" : PlayerVoice.Addresses(view, last.PlayerId) ? "Toi" : CardNames.PlayerName(last.PlayerId);
            announcement.text = last == null ? "" : $"{author} : {last.CardCount} × {CardNames.Rank(last.AnnouncedRank)}";
            string who = PlayerVoice.Addresses(view, view.ActivePlayerId) ? "toi" : CardNames.PlayerName(view.ActivePlayerId);
            active.text = view.Phase == GamePhase.Finished ? "Partie terminée" :
                view.IsTimerPaused ? $"Révélation · chrono en pause" :
                $"À {who} de jouer · {Math.Max(0, (int)Math.Ceiling((remainingTurnSeconds ?? view.RemainingTurnSeconds) / TimerDisplayScale))} s";
            for (int i = 0; i < labels.Length; i++)
            {
                bool visible = i < seatOrder.Count && i != localSeatIndex;
                if (!visible) { if (labels[i] != null) labels[i].gameObject.SetActive(false); continue; }
                anchors[i] = seating.SeatAnchor(i);
                if (labels[i] == null) labels[i] = CreateLabel(i);
                int id = seatOrder[i];
                labels[i].text = $"{CardNames.PlayerName(id)}\n<size=65%>{CardNames.CardCount(view.CardCounts[id])}</size>";
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
            // Bottom corners, either side of the own hand: the top of the screen stays free for names on heads.
            round = CreateText("RoundRank", 44, 84, 60, Corner.Right);
            pile = CreateText("PileCount", 28, 32, 40, Corner.Right);
            announcement = CreateText("LastAnnouncement", 30, 128, 44, Corner.Left);
            active = CreateText("ActivePlayer", 30, 76, 44, Corner.Left);
            ownHand = CreateText("OwnHand", 28, 28, 40, Corner.Left);
        }

        TextMeshProUGUI CreateText(string objectName, float size, float y, float height, Corner corner)
        {
            var go = new GameObject(objectName, typeof(RectTransform));
            go.transform.SetParent(canvas.transform, false);
            var text = go.AddComponent<TextMeshProUGUI>();
            text.font = font != null ? font : TMP_Settings.defaultFontAsset;
            text.fontSize = size;
            bool left = corner == Corner.Left;
            text.alignment = left ? TextAlignmentOptions.Left : TextAlignmentOptions.Right;
            text.raycastTarget = false;
            text.color = Color.white;
            text.fontSharedMaterial = text.font.material;
            var rt = text.rectTransform;
            rt.anchorMin = rt.anchorMax = left ? new Vector2(0, 0) : new Vector2(1, 0);
            rt.pivot = left ? new Vector2(0, 0) : new Vector2(1, 0);
            rt.anchoredPosition = new Vector2(left ? 32 : -32, y);
            rt.sizeDelta = new Vector2(520, height);
            return text;
        }

        TextMeshPro CreateLabel(int seat)
        {
            var go = new GameObject($"OpponentLabel_{seat}");
            go.transform.SetParent(transform, false);
            var text = go.AddComponent<TextMeshPro>();
            text.font = font != null ? font : TMP_Settings.defaultFontAsset;
            // World-space label: about 5 cm tall letters, one line, readable from across the table.
            text.fontSize = 0.7f;
            text.alignment = TextAlignmentOptions.Bottom; // grows upward from the top of the head
            text.textWrappingMode = TextWrappingModes.NoWrap;
            if (labelOverlayMaterial == null)
            {
                var overlay = Shader.Find("TextMeshPro/Distance Field Overlay");
                if (overlay != null) labelOverlayMaterial = new Material(text.font.material) { shader = overlay };
            }
            if (labelOverlayMaterial != null) text.fontSharedMaterial = labelOverlayMaterial;
            text.rectTransform.sizeDelta = new Vector2(1f, .2f);
            text.rectTransform.pivot = new Vector2(.5f, 0f);
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
