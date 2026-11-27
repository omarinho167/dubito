using System;
using System.Linq;
using Dubito.Core;
using Dubito.Protocol;
using NUnit.Framework;
using static Dubito.Tests.Protocol.ProtocolTestData;

namespace Dubito.Tests.Protocol
{
    public class SerializedPrivacyTests
    {
        [TestCase(TestName = "C-PRV-01 (octets)")]
        public void OnlyOwnHandAndPublicRevelationAreReachable_Serialized()
        {
            for (int players = 2; players <= 6; players++)
            {
                var e = Game(players, 910 + players);
                CheckPrivacy(e);
                Send(e, CommandKind.Play, 4, CardRank.King); CheckPrivacy(e);
                Send(e, CommandKind.Doubt); CheckPrivacy(e);
                Assert.That(e.ResolveRevelation(e.State.Revision).Error, Is.EqualTo(CommandError.None));
                CheckPrivacy(e);
                Assert.That(e.AdvanceTime(45).Error, Is.EqualTo(CommandError.None)); CheckPrivacy(e);
                Assert.That(e.StopGame().Error, Is.EqualTo(CommandError.None)); CheckPrivacy(e);
            }
        }

        [TestCase(TestName = "C-PRV-02 (octets)")]
        public void PlayAnnouncesOnlyCountAndRank_Serialized()
        {
            var e = Game(6);
            int actor = e.State.ActivePlayerId;
            var played = e.State.Hands[actor].Take(4).ToArray();
            Send(e, CommandKind.Play, 4, CardRank.Queen);
            foreach (var message in CheckPrivacy(e))
            {
                var play = message.View.LastAnnouncement;
                Assert.That(play.PlayerId, Is.EqualTo(actor));
                Assert.That(play.CardCount, Is.EqualTo(4));
                Assert.That(play.AnnouncedRank, Is.EqualTo(CardRank.Queen));
                Assert.That(ReachableCards(play), Is.Empty);
                Assert.That(ReachableCards(message).Overlaps(played), Is.False);
            }
        }

        [TestCase(TestName = "C-PRV-03 (octets)")]
        public void DoubtRevealsLastPlayOnlyToEveryone_Serialized()
        {
            foreach (bool honest in new[] { false, true })
            {
                var e = BeforeDoubt(honest);
                var last = e.State.LastPlay.Cards.ToArray();
                var earlier = e.State.Pile.Except(last).ToArray();
                Send(e, CommandKind.Doubt);
                foreach (var message in CheckPrivacy(e))
                {
                    Assert.That(message.View.Revelation.Cards, Is.EqualTo(last));
                    Assert.That(message.View.Revelation.IsDoubtCorrect, Is.EqualTo(!honest));
                    Assert.That(ReachableCards(message).Overlaps(earlier), Is.False);
                    Assert.That(message.View.AllowedActions, Is.Empty);
                }
            }
        }

        [TestCase(TestName = "C-PRV-04 (octets)")]
        public void PickupCardsAreSentOnlyToRecipient_Serialized()
        {
            foreach (bool honest in new[] { false, true })
            {
                var e = BeforeDoubt(honest); var pile = e.State.Pile.ToArray();
                Send(e, CommandKind.Doubt);
                int recipient = e.State.Revelation.IsDoubtCorrect
                    ? e.State.Revelation.AccusedPlayerId : e.State.Revelation.DoubterId;
                CheckPrivacy(e);
                Assert.That(e.ResolveRevelation(e.State.Revision).Error, Is.EqualTo(CommandError.None));
                foreach (var message in CheckPrivacy(e))
                {
                    Assert.That(message.View.Revelation, Is.Null);
                    Assert.That(pile.All(c => ReachableCards(message).Contains(c)),
                        Is.EqualTo(message.View.PlayerId == recipient));
                }
            }
        }

        [TestCase(TestName = "C-PRV-05 (octets)")]
        public void DiscardAndDepartureRevealNoNewCards_Serialized()
        {
            var e = BeforeDoubt();
            Send(e, CommandKind.Discard);
            foreach (var message in CheckPrivacy(e))
                Assert.That(ReachableCards(message).Overlaps(e.State.RemovedCards), Is.False);
            Assert.That(e.RemovePlayer(e.State.ActivePlayerId).Error, Is.EqualTo(CommandError.None));
            foreach (var message in CheckPrivacy(e))
                Assert.That(ReachableCards(message).Overlaps(e.State.RemovedCards), Is.False);
        }

        [TestCase(TestName = "C-PRV-06 (octets)")]
        public void PendingWinnerDepartureRevealsTwoOfNineCards_Serialized()
        {
            // Start with a genuine shuffled deal. Drain one player's hand through legal
            // play/discard cycles until it has two cards, without editing the host state.
            var e = Game(3, 6206);
            int target = e.State.TurnOrder[1];
            int safety = 0;
            while (e.State.Hands[target].Count > 2)
            {
                Assert.That(++safety, Is.LessThan(50));
                int active = e.State.ActivePlayerId;
                int count = active == target ? Math.Min(4, e.State.Hands[target].Count - 2) : 1;
                Send(e, CommandKind.Play, count, CardRank.King);
                Send(e, CommandKind.Discard);
                CheckPrivacy(e);
            }
            // Open immediately after target in the ring, allowing the other two seats
            // to put 4+3 cards on the pile before target empties its last two cards.
            int next = e.State.TurnOrder[(e.State.TurnOrder.ToList().IndexOf(target) + 1) % 3];
            if (e.State.ActivePlayerId == target)
                Assert.That(e.AdvanceTime(45).Error, Is.EqualTo(CommandError.None));
            else if (e.State.ActivePlayerId != next)
            { Send(e, CommandKind.Play, 1, CardRank.King); Send(e, CommandKind.Discard); }
            Assert.That(e.State.ActivePlayerId, Is.EqualTo(next));
            Send(e, CommandKind.Play, 4, CardRank.King);
            Send(e, CommandKind.Play, 3);
            Assert.That(e.State.ActivePlayerId, Is.EqualTo(target));
            var last = e.State.Hands[target].ToArray();
            Send(e, CommandKind.Play, 2);
            Assert.That(e.State.Pile.Count, Is.EqualTo(9));
            Assert.That(e.State.PendingWinnerId, Is.EqualTo(target));
            var hidden = e.State.Pile.Except(last).ToArray();
            CheckPrivacy(e);
            Assert.That(e.RemovePlayer(target).Error, Is.EqualTo(CommandError.None));
            foreach (var message in CheckPrivacy(e))
            {
                Assert.That(message.View.Revelation.Cards, Is.EqualTo(last));
                Assert.That(message.View.Revelation.IsAutomatic, Is.True);
                Assert.That(message.View.Revelation.PileRecipientId, Is.Null);
                Assert.That(message.View.PendingWinnerId, Is.EqualTo(target));
                Assert.That(ReachableCards(message).Overlaps(hidden), Is.False);
            }
            Assert.That(e.ResolveRevelation(e.State.Revision).Error, Is.EqualTo(CommandError.None));
            CheckPrivacy(e);
        }

        [Test]
        public void ViewMessageExposesOnlyAPlayerViewConstructionPath()
        {
            var constructors = typeof(ViewMessage).GetConstructors();
            Assert.That(constructors.Length, Is.EqualTo(1));
            Assert.That(constructors[0].GetParameters().Select(p => p.ParameterType),
                Is.EqualTo(new[] { typeof(PlayerView) }));
            Assert.That(typeof(ViewMessage).GetFields(System.Reflection.BindingFlags.Instance |
                System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Public)
                .Select(f => f.FieldType), Is.EqualTo(new[] { typeof(PlayerView) }));
        }
    }
}
