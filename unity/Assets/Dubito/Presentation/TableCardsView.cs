using System;
using System.Collections.Generic;
using Dubito.Core;
using UnityEngine;

namespace Dubito.Presentation
{
    public sealed class TableCardsView : MonoBehaviour
    {
        const float CardWidth = 0.063f, CardHeight = 0.088f; // ASSETS §2
        // Selected card slides up in its own plane (towards the top of the image), keeping its depth layer:
        // it no longer comes in front of its neighbours (owner 2026-11-27).
        const float SelectedLift = 0.03f;
        [SerializeField] GameObject cardPrefab = null;
        [SerializeField] CardMaterialLibrary materials = null;
        [SerializeField] TableSeating seating = null;
        [SerializeField] Transform pileAnchor = null;
        [SerializeField] Vector3 localHandOffset = new Vector3(0, .88f, .38f);
        [SerializeField] float localHandTilt = -55f;
        [SerializeField] float fanRadius = HandFanLayout.DefaultRadius;
        [SerializeField] float fanMaxAngle = HandFanLayout.DefaultMaxAngle;
        [SerializeField] float fanSpacing = HandFanLayout.DefaultSpacing;
        [SerializeField] float cardHalfThickness = .0004f;

        sealed class CardInstance
        {
            public Transform Transform;
            public BoxCollider Hit;
            public Card? LocalCard;
            public Vector3 BasePosition;
            public Renderer[] Renderers;
            public Material[][] Slots;
            public bool[][] Faces;
        }
        readonly List<CardInstance> pool = new List<CardInstance>(52);
        readonly Card[] sortedHand = new Card[52];
        Transform poolRoot;
        int used;
        readonly HashSet<Card> selected = new HashSet<Card>();
        public void SetSelection(IReadOnlyList<Card> values)
        {
            selected.Clear();
            foreach (var card in values) selected.Add(card);
            foreach (var card in pool)
                card.Transform.position = card.BasePosition + (card.LocalCard.HasValue && selected.Contains(card.LocalCard.Value) ? card.Transform.forward * SelectedLift : Vector3.zero);
        }
        public bool TryGetLocalCard(Collider collider, out Card value)
        {
            foreach (var card in pool)
                if (card.Hit == collider && card.LocalCard.HasValue && card.Transform.gameObject.activeInHierarchy)
                { value = card.LocalCard.Value; return true; }
            value = default; return false;
        }

        public void Render(PlayerView view, IReadOnlyList<int> seatOrder, int localSeatIndex)
        {
            ViewSeats.Validate(view, seatOrder, localSeatIndex);
            if (cardPrefab == null || materials == null || seating == null)
                throw new InvalidOperationException("Assign cardPrefab, materials and seating before Render.");
            if (poolRoot == null)
            {
                poolRoot = new GameObject("RenderedCards").transform;
                poolRoot.SetParent(transform, false);
            }
            used = 0;
            HandOrder.CopySorted(view.Hand, sortedHand);
            for (int seat = 0; seat < seatOrder.Count; seat++)
            {
                Transform anchor = seating.SeatAnchor(seat);
                if (anchor == null) throw new InvalidOperationException("Build TableSeating before Render.");
                bool local = seat == localSeatIndex;
                Transform hand = local ? anchor : anchor.Find("Opponent/HandFan");
                if (hand == null) throw new InvalidOperationException("Opponent requires HandFan anchor.");
                int count = local ? view.Hand.Count : view.CardCounts[seatOrder[seat]];
                Vector3 origin = local ? anchor.TransformPoint(localHandOffset) : hand.position;
                Quaternion rotation = local ? anchor.rotation * Quaternion.Euler(localHandTilt, 0, 0)
                    : hand.rotation * Quaternion.Euler(-70, 0, 0);
                for (int i = 0; i < count; i++)
                {
                    Pose pose = HandFanLayout.CardPose(i, count, fanRadius, fanMaxAngle, fanSpacing);
                    Place(origin + rotation * pose.position, rotation * pose.rotation,
                        local ? materials.Face(sortedHand[i]) : materials.Back, local ? sortedHand[i] : (Card?)null);
                }
            }
            Vector3 pile = pileAnchor != null ? pileAnchor.position : new Vector3(0, SeatLayout.TableHeight, 0);
            for (int i = 0; i < view.PileCount; i++)
                Place(pile + Vector3.up * (cardHalfThickness + i * .0008f),
                    Quaternion.Euler(0, ((i * 37) % 17) - 8, 0), materials.Back);
            for (int i = used; i < pool.Count; i++) pool[i].Transform.gameObject.SetActive(false);
        }

        void Place(Vector3 position, Quaternion rotation, Material face, Card? localCard = null)
        {
            if (used == pool.Count) pool.Add(CreateCard());
            CardInstance card = pool[used++];
            card.LocalCard = localCard;
            card.BasePosition = position;
            card.Hit.enabled = localCard.HasValue;
            card.Transform.gameObject.layer = localCard.HasValue ? LayerMask.NameToLayer("SelectableCards") : 0;
            card.Transform.SetPositionAndRotation(position + (localCard.HasValue && selected.Contains(localCard.Value) ? rotation * Vector3.forward * SelectedLift : Vector3.zero), rotation);
            for (int r = 0; r < card.Renderers.Length; r++)
            {
                bool changed = false;
                for (int s = 0; s < card.Slots[r].Length; s++)
                {
                    Material next = card.Faces[r][s] ? face : materials.Back;
                    if (card.Slots[r][s] == next) continue;
                    card.Slots[r][s] = next;
                    changed = true;
                }
                if (changed) card.Renderers[r].sharedMaterials = card.Slots[r];
            }
            card.Transform.gameObject.SetActive(true);
        }

        CardInstance CreateCard()
        {
            GameObject instance = Instantiate(cardPrefab, poolRoot);
            // Only this root collider is selectable. Other prefab colliders do not participate.
            foreach (var collider in instance.GetComponentsInChildren<Collider>(true)) collider.enabled = false;
            instance.layer = 0;
            var hit = instance.AddComponent<BoxCollider>();
            // Card size from ASSETS §2 (the imported mesh is not readable at runtime); a bit thicker to click easily.
            hit.center = Vector3.zero;
            hit.size = new Vector3(CardWidth, .002f, CardHeight);
            var card = new CardInstance { Transform = instance.transform, Hit = hit, Renderers = instance.GetComponentsInChildren<Renderer>(true) };
            card.Slots = new Material[card.Renderers.Length][];
            card.Faces = new bool[card.Renderers.Length][];
            for (int r = 0; r < card.Renderers.Length; r++)
            {
                card.Slots[r] = card.Renderers[r].sharedMaterials;
                card.Faces[r] = new bool[card.Slots[r].Length];
                for (int s = 0; s < card.Slots[r].Length; s++)
                    card.Faces[r][s] = card.Slots[r][s] != null && card.Slots[r][s].name.StartsWith("M_Card_", StringComparison.Ordinal);
            }
            return card;
        }

        void OnDisable() { if (poolRoot != null) poolRoot.gameObject.SetActive(false); }
        void OnEnable() { if (poolRoot != null) poolRoot.gameObject.SetActive(true); }
    }

    internal static class ViewSeats
    {
        public static void Validate(PlayerView view, IReadOnlyList<int> order, int local)
        {
            if (view == null) throw new ArgumentNullException(nameof(view));
            if (order == null) throw new ArgumentNullException(nameof(order));
            if (order.Count < 2 || order.Count > 6 || local < 0 || local >= order.Count)
                throw new ArgumentOutOfRangeException(nameof(local));
            if (order[local] != view.PlayerId) throw new ArgumentException("Local seat must match PlayerView.PlayerId.");
            for (int i = 0; i < order.Count; i++)
            {
                if (order[i] < 0 || order[i] >= view.CardCounts.Count) throw new ArgumentException("Unknown player in seat order.");
                for (int j = 0; j < i; j++) if (order[j] == order[i]) throw new ArgumentException("Duplicate player in seat order.");
            }
        }
    }
}
