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
        [SerializeField] bool debugShortTimer = true;
        [SerializeField] TableActionPanel actionPanel = null;
        float revelationElapsed;
        [SerializeField] bool debugNewGameHotkeys = true;
        [SerializeField] TableSeating seating = null;
        [SerializeField] TableCardsView cards = null;
        [SerializeField] TableHud hud = null;

        LocalGameSession session;
        int localSeat;
        int lastShownSecond = -1;

        public IGameSession Session => session;
        float TimerScale => debugShortTimer ? turnSeconds / 10f : 1f;

        void Start() => NewGame(playerCount);

        // Debug (D-56): keys 2-6 start a new game with that many players.
        public void NewGame(int players)
        {
            if (session != null) session.ViewChanged -= OnViewChanged;
            playerCount = players;
            var random = seed == 0 ? new SystemRandomSource() : new SystemRandomSource(seed);
            session = new LocalGameSession(new GameConfig(players, turnSeconds), random);
            session.ViewChanged += OnViewChanged;
            localSeat = IndexOf(session.SeatOrder, session.ControlledPlayerId);
            seating.Build(players, localSeat);
            hud.TimerDisplayScale = TimerScale;
            OnViewChanged(session.CurrentView);
            actionPanel.Bind(session);
            revelationElapsed = 0;
        }

        void Update()
        {
            if (session == null) return;
            if (session.CurrentView.Phase == GamePhase.Revelation)
            {
                revelationElapsed += Time.deltaTime;
                if (revelationElapsed >= 2f) { revelationElapsed = 0; session.ResolveRevelation(); }
            }
            else { revelationElapsed = 0; if (runTimer) session.Tick(Time.deltaTime * TimerScale); }

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
            cards.Render(view, session.SeatOrder, localSeat);
            hud.Render(view, session.SeatOrder, localSeat);
            lastShownSecond = Mathf.CeilToInt((float)view.RemainingTurnSeconds);
        }

        void ReadHotkeys()
        {
            var keyboard = Keyboard.current;
            if (keyboard == null || TableActionPanel.TextFieldFocused) return;
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
