using System;
using System.Collections.Generic;

namespace Dubito.Core
{
    public enum CommandKind { Play = 0, Doubt = 1, Discard = 2 }
    public enum CommandStatus { Applied = 0, Rejected = 1, Ignored = 2 }

    // Explicit numeric values are part of the error contract. Append, never renumber.
    public enum CommandError
    {
        None = 0,
        InvalidCommand = 1,
        StaleRevision = 2,
        GameFinished = 3,
        RevelationInProgress = 4,
        NotYourTurn = 5,
        InvalidCardCount = 6,
        DuplicateCard = 7,
        CardNotInHand = 8,
        OpeningAnnouncementRequired = 9,
        InvalidAnnouncement = 10,
        AnnouncementMustMatchRound = 11,
        NoDoubtablePlay = 12,
        EmptyPile = 13,
        PendingVictory = 14,
        NoRevelation = 15,
        InvalidElapsedTime = 16,
        PlayerNotInGame = 17
    }

    // Player identity is supplied separately by the authenticated host, not by the command.
    public sealed class GameCommand
    {
        public Guid Id { get; }
        public long ExpectedRevision { get; }
        public CommandKind Kind { get; }
        public IReadOnlyList<Card> Cards { get; }
        public CardRank? AnnouncedRank { get; }

        public GameCommand(Guid id, long expectedRevision, CommandKind kind,
            IEnumerable<Card> cards = null, CardRank? announcedRank = null)
        {
            Id = id;
            ExpectedRevision = expectedRevision;
            Kind = kind;
            Cards = (cards == null ? new List<Card>() : new List<Card>(cards)).AsReadOnly();
            AnnouncedRank = announcedRank;
        }
    }

    public readonly struct CommandResult
    {
        public CommandStatus Status { get; }
        public CommandError Error { get; }
        public long Revision { get; }

        internal CommandResult(CommandStatus status, CommandError error, long revision)
        {
            Status = status;
            Error = error;
            Revision = revision;
        }
    }
}
