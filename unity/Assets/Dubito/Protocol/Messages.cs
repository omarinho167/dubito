using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using Dubito.Core;

namespace Dubito.Protocol
{
    public static class ProtocolInfo
    {
        public const int Version = 1;
        public const int MaxMessageBytes = 16 * 1024;
        public const int MaxPlayerNameCharacters = 32;
        public const int MaxPlayerNameBytes = 128;
    }

    public interface IProtocolMessage { }

    public sealed class HelloMessage : IProtocolMessage
    {
        public int ProtocolVersion { get; }
        public string PlayerName { get; }
        public HelloMessage(int protocolVersion, string playerName)
        {
            Check.PositiveVersion(protocolVersion);
            Check.Name(playerName);
            ProtocolVersion = protocolVersion;
            PlayerName = playerName;
        }
    }

    // No player ID: the host must obtain identity from the sending connection.
    public sealed class CommandMessage : IProtocolMessage
    {
        public Guid Id { get; }
        public long ExpectedRevision { get; }
        public CommandKind Kind { get; }
        public IReadOnlyList<Card> Cards { get; }
        public CardRank? AnnouncedRank { get; }

        public CommandMessage(Guid id, long expectedRevision, CommandKind kind,
            IEnumerable<Card> cards = null, CardRank? announcedRank = null)
        {
            Check.Require(id != Guid.Empty, "Empty command ID.");
            Check.Require(expectedRevision >= 0, "Negative revision.");
            Check.EnumValue(kind);
            var copy = new List<Card>(cards ?? Array.Empty<Card>()).AsReadOnly();
            Check.Cards(copy, 4);
            Check.Rank(announcedRank);
            Id = id;
            ExpectedRevision = expectedRevision;
            Kind = kind;
            Cards = copy;
            AnnouncedRank = announcedRank;
        }

        public static CommandMessage FromGameCommand(GameCommand command)
        {
            if (command == null) throw new ArgumentNullException(nameof(command));
            return new CommandMessage(command.Id, command.ExpectedRevision, command.Kind,
                command.Cards, command.AnnouncedRank);
        }

        public GameCommand ToGameCommand() => new GameCommand(Id, ExpectedRevision, Kind, Cards, AnnouncedRank);
    }

    public sealed class ReadyMessage : IProtocolMessage
    {
        public bool IsReady { get; }
        public ReadyMessage(bool isReady) { IsReady = isReady; }
    }

    public sealed class WelcomeMessage : IProtocolMessage
    {
        public int ProtocolVersion { get; }
        public int PlayerId { get; }
        public IReadOnlyDictionary<int, string> PlayerNames { get; }
        public IReadOnlyList<int> SeatOrder { get; }

        public WelcomeMessage(int protocolVersion, int playerId,
            IReadOnlyDictionary<int, string> playerNames, IEnumerable<int> seatOrder)
        {
            Check.PositiveVersion(protocolVersion);
            if (playerNames == null) throw new ArgumentNullException(nameof(playerNames));
            var names = new Dictionary<int, string>();
            foreach (var entry in playerNames)
            {
                Check.Id(entry.Key);
                Check.Name(entry.Value);
                names.Add(entry.Key, entry.Value);
            }
            Check.Range(names.Count, 2, 6, "Player count");
            Check.Require(names.ContainsKey(playerId), "Unknown recipient.");
            var seats = new List<int>(seatOrder).AsReadOnly();
            Check.Require(seats.Count == names.Count, "Seat count mismatch.");
            var seen = new HashSet<int>();
            foreach (int seat in seats)
                Check.Require(names.ContainsKey(seat) && seen.Add(seat), "Invalid or duplicate seat.");
            ProtocolVersion = protocolVersion;
            PlayerId = playerId;
            PlayerNames = new ReadOnlyDictionary<int, string>(names);
            SeatOrder = seats;
        }
    }

    // The only construction path accepts the Core snapshot, never authoritative state,
    // a pile, or opponents' hands. There is no host reference or additional card field.
    public sealed class ViewMessage : IProtocolMessage
    {
        public PlayerView View { get; }
        public ViewMessage(PlayerView view)
        {
            Check.View(view);
            View = view;
        }
        public PlayerView ToPlayerView() => View;
    }

    public sealed class CommandRejectedMessage : IProtocolMessage
    {
        public Guid CommandId { get; }
        public CommandError Error { get; }
        public CommandRejectedMessage(Guid commandId, CommandError error)
        {
            Check.Require(commandId != Guid.Empty, "Empty command ID.");
            Check.EnumValue(error);
            CommandId = commandId;
            Error = error;
        }
    }

    public sealed class VersionMismatchMessage : IProtocolMessage
    {
        public int ExpectedVersion { get; }
        public VersionMismatchMessage(int expectedVersion)
        {
            Check.PositiveVersion(expectedVersion);
            ExpectedVersion = expectedVersion;
        }
    }

    public sealed class LobbyPlayer
    {
        public int PlayerId { get; }
        public string PlayerName { get; }
        public bool IsReady { get; }
        public LobbyPlayer(int playerId, string playerName, bool isReady)
        {
            Check.Id(playerId);
            Check.Name(playerName);
            PlayerId = playerId;
            PlayerName = playerName;
            IsReady = isReady;
        }
    }

    public sealed class LobbyStateMessage : IProtocolMessage
    {
        // Entries are the connected players, including the host; IDs need not be dense.
        public IReadOnlyList<LobbyPlayer> Players { get; }
        public int TurnDurationSeconds { get; }
        public int MaxPlayers { get; }
        public LobbyStateMessage(IEnumerable<LobbyPlayer> players, int turnDurationSeconds, int maxPlayers)
        {
            Check.Range(maxPlayers, 2, 6, "Maximum players");
            Check.Require(turnDurationSeconds == 30 || turnDurationSeconds == 45 ||
                turnDurationSeconds == 60 || turnDurationSeconds == 90, "Invalid timer setting.");
            var copy = new List<LobbyPlayer>(players).AsReadOnly();
            Check.Range(copy.Count, 1, maxPlayers, "Connected players");
            var seen = new HashSet<int>();
            foreach (var player in copy)
                Check.Require(player != null && seen.Add(player.PlayerId), "Null or duplicate player.");
            Players = copy;
            TurnDurationSeconds = turnDurationSeconds;
            MaxPlayers = maxPlayers;
        }
    }
}
