using System;
using System.Collections.Generic;
using Dubito.Core;
using Dubito.Networking;
using UnityEngine;
using UnityEngine.InputSystem;

namespace Dubito.Presentation
{
    // Both modes render IGameSession. Only Local drives time, hot-seat and revelation resolution.
    public sealed class LocalTableController : MonoBehaviour
    {
        [SerializeField, Range(SeatLayout.MinSeats, SeatLayout.MaxSeats)] int playerCount = 4;
        [SerializeField, Min(1)] int turnSeconds = 45;
        [Tooltip("0 = random deal each game; any other value replays the same deal.")]
        [SerializeField] int seed = 0;
        [Tooltip("Run the turn timer (turns are skipped when it expires).")]
        [SerializeField] bool runTimer = true;
        [Tooltip("Debug (D-58): 10 real seconds per turn. Off by default since the hot-seat exists.")]
        [SerializeField] bool debugShortTimer = false;
        [Tooltip("Hot-seat (UX §9, D-47): after each change, show the active player's seat.")]
        [SerializeField] bool hotSeat = true;
        [SerializeField] bool startLocalAutomatically = true;
        [SerializeField] TableActionPanel actionPanel = null;
        [SerializeField] RevelationPresenter revelation = null;
        [SerializeField] TableMessages messages = null;
        [SerializeField] bool debugNewGameHotkeys = true;
        [SerializeField] TableSeating seating = null;
        [SerializeField] TableCardsView cards = null;
        [SerializeField] TableHud hud = null;

        IGameSession session;
        LocalGameSession LocalSession => session as LocalGameSession;
        NetworkGameSession networkSession;
        bool initialized, layoutReady;
        int localSeat;
        int lastShownSecond = -1;
        bool switchPending, ticking, removing;
        PlayerView lastView;

        public IGameSession Session => session;
        float timerScale = 1f; // fixed per game so the countdown shown always matches the one ticked

        void Start()
        {
            Initialize();
            if (startLocalAutomatically && session == null) NewGame(playerCount);
        }

        void Initialize()
        {
            if (initialized) return;
            initialized = true;
            messages.RematchRequested += () =>
            {
                if (LocalSession != null) NewGame(playerCount);
                else NetworkRematchRequested?.Invoke();
            };
            messages.QuitRequested += () => LeaveRequested?.Invoke();
        }

        // End screen buttons outside a local game (handled by the lobby screen).
        public event Action NetworkRematchRequested;
        public event Action LeaveRequested;

        public void StartLocal() => NewGame(playerCount);

        public void UseNetworkSession(NetworkGameSession value, bool allowRematch)
        {
            Suspend();
            Initialize();
            PlayerVoice.HotSeat = false;
            CardNames.PlayerNames = value.PlayerNames;
            messages.AllowRematch = allowRematch; // only the host can call a rematch
            session = value;
            networkSession = value;
            networkSession.TableEvent += OnTableEvent;
            timerScale = hud.TimerDisplayScale = 1f;
            session.ViewChanged += OnViewChanged;
            if (session.IsReady) OnViewChanged(session.CurrentView);
        }

        void OnTableEvent(Dubito.Protocol.TableEventMessage tableEvent) =>
            messages.ShowTransient(TableMessageText.ForEvent(tableEvent.Kind, tableEvent.PlayerId, session.ControlledPlayerId));

        public void Suspend()
        {
            if (session != null) session.ViewChanged -= OnViewChanged;
            if (networkSession != null) networkSession.TableEvent -= OnTableEvent;
            networkSession = null;
            revelation.Cancel();
            actionPanel.Bind(null);
            messages.Clear();
            session = null;
            lastView = null;
            layoutReady = switchPending = false;
            CardNames.PlayerNames = null;
            PlayerVoice.HotSeat = false;
            gameObject.SetActive(false);
        }

        // Debug (D-56): keys 2-6 start a new game with that many players.
        public void NewGame(int players)
        {
            Suspend();
            Initialize();
            gameObject.SetActive(true);
            PlayerVoice.HotSeat = hotSeat;
            messages.AllowRematch = true;
            playerCount = players;
            lastView = null;
            messages.Clear();
            var random = seed == 0 ? new SystemRandomSource() : new SystemRandomSource(seed);
            session = new LocalGameSession(new GameConfig(players, turnSeconds), random);
            session.ViewChanged += OnViewChanged;
            localSeat = IndexOf(session.SeatOrder, session.ControlledPlayerId);
            seating.Build(players, localSeat);
            layoutReady = true;
            timerScale = debugShortTimer ? turnSeconds / 10f : 1f;
            hud.TimerDisplayScale = timerScale;
            OnViewChanged(session.CurrentView);
            actionPanel.Bind(session);
            switchPending = false;
        }

        void Update()
        {
            if (session == null || !session.IsReady) return;
            if (LocalSession != null && switchPending) SwitchToActivePlayer();
            // The chrono is paused during a revelation (RULES §5); RevelationPresenter ends it.
            if (LocalSession != null && runTimer && session.CurrentView.Phase == GamePhase.Playing)
            {
                ticking = true;
                LocalSession.Tick(Time.deltaTime * timerScale);
                ticking = false;
            }

            // Countdown ticks refresh CurrentView without a new revision: redraw the HUD once per second.
            var view = session.CurrentView;
            int second = Mathf.CeilToInt((float)session.RemainingTurnSeconds);
            if (second != lastShownSecond)
            {
                lastShownSecond = second;
                hud.Render(view, session.SeatOrder, localSeat, session.RemainingTurnSeconds);
            }

            if (LocalSession != null && debugNewGameHotkeys && (Application.isEditor || Debug.isDebugBuild)) ReadHotkeys();
            // Debug (step 3.5): Suppr leaves the online game, to test departures (RULES §7);
            // for the host this ends the game for everyone (C-DEC-06).
            else if (networkSession != null && (Application.isEditor || Debug.isDebugBuild) &&
                     Keyboard.current != null && Keyboard.current.deleteKey.wasPressedThisFrame && !TableActionPanel.TextFieldFocused)
                LeaveRequested?.Invoke();
        }

        void OnViewChanged(PlayerView view)
        {
            bool firstView = !layoutReady;
            if (firstView)
            {
                gameObject.SetActive(true);
                localSeat = IndexOf(session.SeatOrder, session.ControlledPlayerId);
                seating.Build(session.SeatOrder.Count, localSeat);
                layoutReady = true;
                if (session is NetworkGameSession network) CardNames.PlayerNames = network.PlayerNames;
            }
            // UX §9: stay on the doubter's seat during a revelation, then follow the active player.
            // Switched on the next frame so every ViewChanged listener first finishes with this view.
            if (LocalSession != null && hotSeat && view.Phase == GamePhase.Playing && view.ActivePlayerId != session.ControlledPlayerId)
                switchPending = true;
            cards.Render(view, session.SeatOrder, localSeat);
            hud.Render(view, session.SeatOrder, localSeat, session.RemainingTurnSeconds);
            lastShownSecond = Mathf.CeilToInt((float)session.RemainingTurnSeconds);

            // UX §5 key moments.
            if (ticking) messages.ShowTransient(TableMessageText.AfterTimeout(lastView, view));
            if (removing && lastView != null)
                messages.ShowTransient($"{CardNames.PlayerName(lastView.PlayerId)} a quitté la partie");
            messages.Render(view);
            revelation.Show(view, session.SeatOrder, () => LocalSession?.ResolveRevelation());
            lastView = view;
            if (firstView) actionPanel.Bind(session);
        }

        void SwitchToActivePlayer()
        {
            switchPending = false;
            var view = session.CurrentView;
            if (view.Phase != GamePhase.Playing || view.ActivePlayerId == session.ControlledPlayerId) return;
            localSeat = IndexOf(session.SeatOrder, view.ActivePlayerId);
            seating.Build(playerCount, localSeat);
            LocalSession.SetControlledPlayer(view.ActivePlayerId); // publishes the new seat's view
            messages.ShowTransientIfIdle($"Au tour de {CardNames.PlayerName(view.ActivePlayerId)}");
        }

        void ReadHotkeys()
        {
            var keyboard = Keyboard.current;
            if (keyboard == null || TableActionPanel.TextFieldFocused) return;
            // UX §9: simulate the departure (disconnection) of the displayed seat, to test RULES §7.
            if (keyboard.deleteKey.wasPressedThisFrame)
            {
                removing = true;
                LocalSession.RemovePlayer(session.ControlledPlayerId);
                removing = false;
                return;
            }
            for (int n = SeatLayout.MinSeats; n <= SeatLayout.MaxSeats; n++)
            {
                if (keyboard[(Key)((int)Key.Digit2 + n - 2)].wasPressedThisFrame ||
                    keyboard[(Key)((int)Key.Numpad2 + n - 2)].wasPressedThisFrame)
                {
                    NewGame(n);
                    return;
                }
            }
        }

        static int IndexOf(IReadOnlyList<int> order, int playerId)
        {
            for (int i = 0; i < order.Count; i++)
                if (order[i] == playerId) return i;
            return 0;
        }

        void OnDestroy()
        {
            PlayerVoice.HotSeat = false; // static: never leak into another scene or into edit-mode tests
            CardNames.PlayerNames = null;
            if (session != null) session.ViewChanged -= OnViewChanged;
        }
    }
}
