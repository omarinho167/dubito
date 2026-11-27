using System.Collections.Generic;
using Dubito.Core;
using Dubito.Networking;
using UnityEngine;
using UnityEngine.InputSystem;

namespace Dubito.Presentation
{
    // Phase 2 (ARCHITECTURE §3, "Session de jeu"): owns a LocalGameSession and shows the controlled
    // seat's PlayerView on the 3D table. Presentation only: every rule stays in Core.
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
        [SerializeField] TableActionPanel actionPanel = null;
        [SerializeField] RevelationPresenter revelation = null;
        [SerializeField] TableMessages messages = null;
        [SerializeField] bool debugNewGameHotkeys = true;
        [SerializeField] TableSeating seating = null;
        [SerializeField] TableCardsView cards = null;
        [SerializeField] TableHud hud = null;

        LocalGameSession session;
        int localSeat;
        int lastShownSecond = -1;
        bool switchPending, ticking, removing;
        PlayerView lastView;

        public IGameSession Session => session;
        float timerScale = 1f; // fixed per game so the countdown shown always matches the one ticked

        void Start()
        {
            messages.RematchRequested += () => NewGame(playerCount); // hot-seat rematch: same table, new deal
            messages.QuitRequested += Quit;
            NewGame(playerCount);
        }

        // Debug (D-56): keys 2-6 start a new game with that many players.
        public void NewGame(int players)
        {
            if (session != null) session.ViewChanged -= OnViewChanged;
            playerCount = players;
            lastView = null;
            messages.Clear();
            var random = seed == 0 ? new SystemRandomSource() : new SystemRandomSource(seed);
            session = new LocalGameSession(new GameConfig(players, turnSeconds), random);
            session.ViewChanged += OnViewChanged;
            localSeat = IndexOf(session.SeatOrder, session.ControlledPlayerId);
            seating.Build(players, localSeat);
            timerScale = debugShortTimer ? turnSeconds / 10f : 1f;
            hud.TimerDisplayScale = timerScale;
            OnViewChanged(session.CurrentView);
            actionPanel.Bind(session);
            switchPending = false;
        }

        void Update()
        {
            if (session == null) return;
            if (switchPending) SwitchToActivePlayer();
            // The chrono is paused during a revelation (RULES §5); RevelationPresenter ends it.
            if (runTimer && session.CurrentView.Phase == GamePhase.Playing)
            {
                ticking = true;
                session.Tick(Time.deltaTime * timerScale);
                ticking = false;
            }

            // Countdown ticks refresh CurrentView without a new revision: redraw the HUD once per second.
            var view = session.CurrentView;
            int second = Mathf.CeilToInt((float)view.RemainingTurnSeconds);
            if (second != lastShownSecond)
            {
                lastShownSecond = second;
                hud.Render(view, session.SeatOrder, localSeat);
            }

            if (debugNewGameHotkeys && (Application.isEditor || Debug.isDebugBuild)) ReadHotkeys();
        }

        void OnViewChanged(PlayerView view)
        {
            // UX §9: stay on the doubter's seat during a revelation, then follow the active player.
            // Switched on the next frame so every ViewChanged listener first finishes with this view.
            if (hotSeat && view.Phase == GamePhase.Playing && view.ActivePlayerId != session.ControlledPlayerId)
                switchPending = true;
            cards.Render(view, session.SeatOrder, localSeat);
            hud.Render(view, session.SeatOrder, localSeat);
            lastShownSecond = Mathf.CeilToInt((float)view.RemainingTurnSeconds);

            // UX §5 key moments.
            if (ticking) messages.ShowTransient(TableMessageText.AfterTimeout(lastView, view));
            if (removing && lastView != null)
                messages.ShowTransient($"{CardNames.PlayerName(lastView.PlayerId)} a quitté la partie");
            messages.Render(view);
            revelation.Show(view, session.SeatOrder, () => session.ResolveRevelation());
            lastView = view;
        }

        void Quit()
        {
#if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;
#else
            Application.Quit();
#endif
        }

        void SwitchToActivePlayer()
        {
            switchPending = false;
            var view = session.CurrentView;
            if (view.Phase != GamePhase.Playing || view.ActivePlayerId == session.ControlledPlayerId) return;
            localSeat = IndexOf(session.SeatOrder, view.ActivePlayerId);
            seating.Build(playerCount, localSeat);
            session.SetControlledPlayer(view.ActivePlayerId); // publishes the new seat's view
        }

        void ReadHotkeys()
        {
            var keyboard = Keyboard.current;
            if (keyboard == null || TableActionPanel.TextFieldFocused) return;
            // UX §9: simulate the departure (disconnection) of the displayed seat, to test RULES §7.
            if (keyboard.deleteKey.wasPressedThisFrame)
            {
                removing = true;
                session.RemovePlayer(session.ControlledPlayerId);
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
            if (session != null) session.ViewChanged -= OnViewChanged;
        }
    }
}
