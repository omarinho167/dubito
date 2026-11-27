using System;
using System.Collections.Generic;
using Dubito.Core;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

namespace Dubito.Presentation
{
    // The only cinematic moment (UX §5): slight zoom on the pile, the contested cards turn over one
    // by one, verdict in big letters, then the pile slides towards the player who picks it up.
    // Calls `finished` at the end; the session then applies the verdict Core already computed.
    public sealed class RevelationPresenter : MonoBehaviour
    {
        const int MaxCards = 4;
        const float CardThickness = 0.0008f;

        [SerializeField] TableCardsView cards = null;
        [SerializeField] TableSeating seating = null;
        [SerializeField] Camera viewCamera = null;
        [SerializeField] GameObject cardPrefab = null;
        [SerializeField] CardMaterialLibrary materials = null;
        [SerializeField] TMP_FontAsset font = null;
        [SerializeField] float zoomFieldOfView = 46f;

        readonly Transform[] shown = new Transform[MaxCards];
        Action finished;
        RevelationView revelation;
        long playingRevision = -1;
        float elapsed, baseFieldOfView = 60f;
        int cardCount, pileCount;
        Vector3 rowCenter, slideOffset;
        Quaternion faceUp;
        GameObject canvas;
        TextMeshProUGUI verdict, outcome;

        public bool IsPlaying => playingRevision >= 0;

        public void Show(PlayerView view, IReadOnlyList<int> seatOrder, Action onFinished)
        {
            if (view.Phase != GamePhase.Revelation || view.Revelation == null)
            {
                if (IsPlaying) Stop();
                return;
            }
            if (IsPlaying)
            {
                // Same revelation republished (e.g. a departure, RULES §7): keep the timing, update
                // who picks up the pile and the wording; the verdict itself never changes.
                revelation = view.Revelation;
                ApplyOutcome(view, seatOrder);
                return;
            }

            finished = onFinished;
            revelation = view.Revelation;
            playingRevision = view.Revision;
            elapsed = 0f;
            cardCount = Mathf.Min(revelation.Cards.Count, MaxCards);
            pileCount = view.PileCount;
            if (viewCamera == null) viewCamera = Camera.main;
            baseFieldOfView = viewCamera != null ? viewCamera.fieldOfView : 60f;

            // Row of revealed cards just above the pile, readable from the controlled seat.
            Vector3 pile = cards.PileCenter;
            float yaw = viewCamera != null ? viewCamera.transform.eulerAngles.y : 0f;
            var seatYaw = Quaternion.Euler(0f, yaw, 0f);
            rowCenter = pile + Vector3.up * (pileCount * CardThickness + 0.006f) + seatYaw * new Vector3(0f, 0f, -0.06f);
            faceUp = seatYaw;
            for (int i = 0; i < MaxCards; i++)
            {
                if (shown[i] == null) shown[i] = CreateCard(i);
                bool used = i < cardCount;
                shown[i].gameObject.SetActive(used);
                if (used) SetFace(shown[i], materials.Face(revelation.Cards[i]));
            }

            EnsureCanvas();
            verdict.text = RevelationScript.Verdict(revelation);
            verdict.color = revelation.IsDoubtCorrect ? new Color(1f, .42f, .36f) : new Color(.55f, .95f, .6f);
            ApplyOutcome(view, seatOrder);
            canvas.SetActive(false);
            Animate();
        }

        // Pile destination and outcome text come from Core's RevelationView.PileRecipientId.
        void ApplyOutcome(PlayerView view, IReadOnlyList<int> seatOrder)
        {
            Vector3 pile = cards.PileCenter;
            int? picker = RevelationScript.PickerId(revelation);
            Vector3 target = picker.HasValue ? SeatPosition(seatOrder, picker.Value) : pile + faceUp * new Vector3(1.2f, 0f, 0f);
            slideOffset = Vector3.ProjectOnPlane(target - pile, Vector3.up) * 0.65f;
            outcome.text = RevelationScript.Outcome(revelation, pileCount, view.PendingWinnerId);
        }

        void Update()
        {
            if (!IsPlaying) return;
            elapsed += Time.deltaTime;
            if (elapsed >= RevelationScript.Duration(cardCount))
            {
                var done = finished;
                Stop();
                done?.Invoke();
                return;
            }
            Animate();
        }

        void Animate()
        {
            float zoomIn = Smooth(elapsed / RevelationScript.ZoomIn);
            float slide = Smooth((elapsed - RevelationScript.SlideStart(cardCount)) / RevelationScript.Slide);
            if (viewCamera != null)
                viewCamera.fieldOfView = Mathf.Lerp(baseFieldOfView, zoomFieldOfView, zoomIn * (1f - slide));

            Vector3 offset = slideOffset * slide;
            for (int i = 0; i < cardCount; i++)
            {
                // Flip around the card's own vertical image axis, with a small lift during the turn.
                float start = RevelationScript.ZoomIn + RevelationScript.FlipEach * i;
                float flip = Smooth((elapsed - start) / RevelationScript.FlipTime);
                float x = (i - (cardCount - 1) / 2f) * 0.075f;
                Vector3 position = rowCenter + faceUp * new Vector3(x, 0f, 0f) + Vector3.up * (Mathf.Sin(flip * Mathf.PI) * 0.04f + i * 0.0002f);
                shown[i].SetPositionAndRotation(position + offset, faceUp * Quaternion.Euler(0f, 0f, 180f * (1f - flip)));
            }
            cards.SetPileOffset(offset);
            canvas.SetActive(elapsed >= RevelationScript.FlipsEnd(cardCount));
        }

        void Stop()
        {
            playingRevision = -1;
            finished = null;
            if (viewCamera != null) viewCamera.fieldOfView = baseFieldOfView;
            foreach (var card in shown) if (card != null) card.gameObject.SetActive(false);
            if (canvas != null) canvas.SetActive(false);
            cards.SetPileOffset(Vector3.zero);
        }

        Vector3 SeatPosition(IReadOnlyList<int> seatOrder, int playerId)
        {
            for (int i = 0; i < seatOrder.Count; i++)
                if (seatOrder[i] == playerId) return seating.SeatAnchor(i).position;
            return cards.PileCenter;
        }

        Transform CreateCard(int index)
        {
            var card = Instantiate(cardPrefab, transform);
            card.name = $"RevealedCard_{index}";
            foreach (var collider in card.GetComponentsInChildren<Collider>(true)) collider.enabled = false;
            card.SetActive(false);
            return card.transform;
        }

        void SetFace(Transform card, Material face)
        {
            foreach (var renderer in card.GetComponentsInChildren<Renderer>(true))
            {
                var slots = renderer.sharedMaterials;
                for (int s = 0; s < slots.Length; s++)
                    slots[s] = slots[s] != null && slots[s].name.StartsWith("M_CardBack", StringComparison.Ordinal) ? materials.Back : face;
                renderer.sharedMaterials = slots;
            }
        }

        void EnsureCanvas()
        {
            if (canvas != null) return;
            canvas = new GameObject("RevelationCanvas", typeof(RectTransform), typeof(Canvas), typeof(CanvasScaler));
            canvas.transform.SetParent(transform, false);
            var c = canvas.GetComponent<Canvas>();
            c.renderMode = RenderMode.ScreenSpaceOverlay;
            c.sortingOrder = 20;
            var scaler = canvas.GetComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
            scaler.referenceResolution = new Vector2(1920, 1080);
            scaler.matchWidthOrHeight = .5f;
            verdict = Label("Verdict", 120, new Vector2(0, 150), FontStyles.Bold);
            outcome = Label("Outcome", 36, new Vector2(0, 60), FontStyles.Normal);
        }

        TextMeshProUGUI Label(string objectName, float size, Vector2 position, FontStyles style)
        {
            var go = new GameObject(objectName, typeof(RectTransform));
            go.transform.SetParent(canvas.transform, false);
            var text = go.AddComponent<TextMeshProUGUI>();
            text.font = font != null ? font : TMP_Settings.defaultFontAsset;
            text.fontSize = size;
            text.fontStyle = style;
            text.alignment = TextAlignmentOptions.Center;
            text.raycastTarget = false;
            text.outlineWidth = 0.2f;
            text.outlineColor = new Color32(0, 0, 0, 200);
            var rt = text.rectTransform;
            rt.anchorMin = rt.anchorMax = rt.pivot = new Vector2(.5f, .5f);
            rt.anchoredPosition = position;
            rt.sizeDelta = new Vector2(1400, size * 1.4f);
            return text;
        }

        static float Smooth(float t) => Mathf.SmoothStep(0f, 1f, Mathf.Clamp01(t));
    }
}
