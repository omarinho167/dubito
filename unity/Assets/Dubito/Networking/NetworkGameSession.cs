using System;
using System.Collections.Generic;
using System.Diagnostics;
using Dubito.Core;
using Dubito.Protocol;

namespace Dubito.Networking
{
    public sealed class NetworkGameSession : IGameSession
    {
        readonly Action<byte[]> send;
        readonly Func<double> clock;
        double receivedAt;
        bool welcomed;
        public int ControlledPlayerId { get; private set; } = -1;
        public IReadOnlyList<int> SeatOrder { get; private set; } = Array.Empty<int>();
        public IReadOnlyDictionary<int, string> PlayerNames { get; private set; } = new Dictionary<int, string>();
        public PlayerView CurrentView { get; private set; }
        public bool IsReady => CurrentView != null;
        public double RemainingTurnSeconds => CurrentView == null ? 0 :
            CurrentView.IsTimerPaused ? CurrentView.RemainingTurnSeconds :
            Math.Max(0, CurrentView.RemainingTurnSeconds - Math.Max(0, clock() - receivedAt));
        public event Action<PlayerView> ViewChanged;
        public event Action<CommandError> CommandRejected;
        public event Action<int> VersionMismatch;
        public event Action<string> ProtocolError;
        public event Action<LobbyStateMessage> LobbyChanged;

        public NetworkGameSession(Action<byte[]> send, Func<double> clock = null)
        {
            this.send = send ?? throw new ArgumentNullException(nameof(send));
            this.clock = clock ?? (() => (double)Stopwatch.GetTimestamp() / Stopwatch.Frequency);
        }

        public void Receive(byte[] bytes)
        {
            var decoded = ProtocolCodec.Decode(bytes);
            if (!decoded.Success) { ProtocolError?.Invoke(decoded.Error); return; }
            switch (decoded.Message)
            {
                case VersionMismatchMessage mismatch: VersionMismatch?.Invoke(mismatch.ExpectedVersion); return;
                case WelcomeMessage welcome when !welcomed:
                    if (welcome.ProtocolVersion != ProtocolInfo.Version) { VersionMismatch?.Invoke(welcome.ProtocolVersion); return; }
                    ControlledPlayerId = welcome.PlayerId;
                    SeatOrder = welcome.SeatOrder;
                    PlayerNames = welcome.PlayerNames;
                    welcomed = true;
                    return;
                case ViewMessage message when welcomed:
                    var view = message.ToPlayerView();
                    if (view.PlayerId != ControlledPlayerId || view.CardCounts.Count != SeatOrder.Count)
                    { ProtocolError?.Invoke("Vue reçue pour un autre siège."); return; }
                    if (CurrentView != null && view.Revision < CurrentView.Revision) return;
                    CurrentView = view;
                    receivedAt = clock();
                    ViewChanged?.Invoke(view);
                    return;
                case CommandRejectedMessage rejected when welcomed: CommandRejected?.Invoke(rejected.Error); return;
                case LobbyStateMessage lobby when !welcomed: LobbyChanged?.Invoke(lobby); return;
                default: ProtocolError?.Invoke("Message inattendu de l'hôte."); return;
            }
        }

        public void Play(IReadOnlyList<Card> cards, CardRank announcedRank) => Send(CommandKind.Play, cards, announcedRank);
        public void Doubt() => Send(CommandKind.Doubt);
        public void Discard() => Send(CommandKind.Discard);
        void Send(CommandKind kind, IEnumerable<Card> cards = null, CardRank? rank = null)
        {
            if (!IsReady) { CommandRejected?.Invoke(CommandError.InvalidCommand); return; }
            send(ProtocolCodec.Encode(new CommandMessage(Guid.NewGuid(), CurrentView.Revision, kind, cards, rank)));
        }
    }
}
