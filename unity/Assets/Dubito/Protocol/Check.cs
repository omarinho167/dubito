using System;
using System.Collections.Generic;
using System.Text;
using Dubito.Core;

namespace Dubito.Protocol
{
    // Wire/schema validation only. Core remains responsible for legality of commands.
    internal static class Check
    {
        internal static readonly UTF8Encoding Utf8 = new UTF8Encoding(false, true);
        internal static void Require(bool condition, string error)
        {
            if (!condition) throw new ArgumentException(error);
        }
        internal static void Range(int value, int min, int max, string field)
            => Require(value >= min && value <= max, field + " out of range.");
        internal static void Id(int id, int count = 6) => Range(id, 0, count - 1, "Player ID");
        internal static void OptionalId(int? id, int count)
        {
            if (id.HasValue) Id(id.Value, count);
        }
        internal static void PositiveVersion(int version) => Require(version > 0, "Invalid protocol version.");
        internal static void EnumValue<T>(T value) where T : struct, Enum
            => Require(Enum.IsDefined(typeof(T), value), "Unknown " + typeof(T).Name + ".");
        internal static void Rank(CardRank? rank)
        {
            if (rank.HasValue) EnumValue(rank.Value);
        }
        internal static void Name(string name)
        {
            if (name == null) throw new ArgumentNullException(nameof(name));
            Range(name.Length, 0, ProtocolInfo.MaxPlayerNameCharacters, "Name characters");
            Range(Utf8.GetByteCount(name), 0, ProtocolInfo.MaxPlayerNameBytes, "Name bytes");
        }
        internal static void Cards(IReadOnlyList<Card> cards, int max)
        {
            Range(cards.Count, 0, max, "Card count");
            var distinct = new HashSet<Card>();
            foreach (var card in cards)
            {
                EnumValue(card.Rank);
                EnumValue(card.Suit);
                Require(distinct.Add(card), "Duplicate card.");
            }
        }
        internal static void View(PlayerView view)
        {
            if (view == null) throw new ArgumentNullException(nameof(view));
            int count = view.CardCounts.Count;
            Range(count, 2, 6, "Player count");
            Id(view.PlayerId, count);
            Cards(view.Hand, 52);
            foreach (int cards in view.CardCounts) Range(cards, 0, 52, "Hand count");
            Require(view.CardCounts[view.PlayerId] == view.Hand.Count, "Own hand count mismatch.");
            Range(view.TurnOrder.Count, 0, count, "Turn order count");
            var seen = new HashSet<int>();
            foreach (int id in view.TurnOrder)
            {
                Id(id, count);
                Require(seen.Add(id), "Duplicate turn seat.");
            }
            Range(view.PileCount, 0, 52, "Pile count");
            Rank(view.RoundRank);
            Range(view.ActivePlayerId, -1, count - 1, "Active player ID");
            Require(!double.IsNaN(view.RemainingTurnSeconds) && !double.IsInfinity(view.RemainingTurnSeconds)
                && view.RemainingTurnSeconds >= 0 && view.RemainingTurnSeconds <= 90, "Invalid remaining time.");
            EnumValue(view.Phase);
            OptionalId(view.PendingWinnerId, count);
            OptionalId(view.WinnerId, count);
            Range(view.OwnSkippedTurns, 0, 2, "Skipped turns");
            Require(view.Revision >= 0, "Negative revision.");
            Range(view.AllowedActions.Count, 0, 3, "Action count");
            var actions = new HashSet<CommandKind>();
            foreach (var kind in view.AllowedActions)
            {
                EnumValue(kind);
                Require(actions.Add(kind), "Duplicate allowed action.");
            }
            if (view.LastAnnouncement != null)
            {
                Id(view.LastAnnouncement.PlayerId, count);
                Range(view.LastAnnouncement.CardCount, 1, 4, "Announcement count");
                EnumValue(view.LastAnnouncement.AnnouncedRank);
            }
            if (view.Revelation != null)
            {
                Id(view.Revelation.AccusedPlayerId, count);
                Id(view.Revelation.DoubterId, count);
                OptionalId(view.Revelation.PileRecipientId, count);
                Cards(view.Revelation.Cards, 4);
                Require(view.Revelation.Cards.Count > 0, "Empty revelation.");
            }
        }
    }
}
