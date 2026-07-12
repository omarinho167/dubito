# Dubito - V1 Roadmap

This is the execution plan. It stays at phase, sub-phase, responsibility, and validation level. It intentionally contains no code snippets, no command blocks, and no implementation recipes.

## Roadmap Rules

- Phases are milestones. Sub-phases are the execution units agents should pick up.
- A sub-phase should be small enough for one focused agent session and large enough to leave the project in a testable state.
- Each sub-phase has one main risk surface. Avoid combining core rules, network privacy, UI flow, Steam setup, and asset work in one unit.
- Work should normally proceed one sub-phase at a time. Parallel work is allowed only where the dependency notes below explicitly say it is safe.
- Every sub-phase must end with a clear validation result, an updated status, and any unresolved risk called out.
- Human decisions, purchases, account actions, license checks, and external asset choices are explicit gates. Agents do not silently assume them.
- Conceptual documentation stays conceptual: no code blocks, no command blocks, and no step-by-step shell instructions. Inline identifiers are allowed when they name planned files, modules, folders, properties, or Unreal concepts.

## Phase Overview

| Phase | Name | Status | Goal | Sub-phase strategy |
|---|---|---|---|---|
| 0 | Conception | Done | compact, coherent V1 documentation | freeze the contract before implementation |
| 1 | Unreal Bootstrap | Done | clean UE project, first-party V1 stack, source-control foundation | build the project shell before any gameplay |
| 2 | Core Rules | Done | pure rule model with tests | prove game logic without actors, widgets, maps, or live networking |
| 3 | Greybox Table | Done | readable local table, cards, camera, basic UI shell | prove readability and input before multiplayer complexity |
| 4 | Network Framework | Done | host-authoritative replicated match state | connect Unreal framework to the pure rules while preserving privacy |
| 5 | Full Gameplay Loop | In progress | Play, Doubt, Discard, timer, win, post-game | add one player-facing action or flow at a time |
| 6 | Steam Multiplayer | Locked | Steam lobbies/invites tested on real machines | swap from local validation to the real Steam path |
| 7 | V1 Polish | Locked | clarity, accessibility, UX pass, packaging | harden the already-playable game |
| 8 | Release Prep | Locked | real AppID, depot, store/build checklist | prepare release only after V1 is proven |

## Dependency Strategy

- Phase 1 unlocks all implementation work. Do not start gameplay implementation before the project, modules, source control rules, and required Unreal systems exist.
- Phase 2 and Phase 3 can proceed partly in parallel after Phase 1, because pure rules and greybox readability are separate risk surfaces.
- Phase 4 depends on the Phase 2 rules contract and enough Phase 3 UI/map structure to observe replicated state.
- Phase 5 should be mostly sequential. Play, Doubt, Discard, timer, pending-win, and session flow each build on previous behavior.
- Local multiplayer validation escalates from PIE multi-player, to separate-process or standalone checks, to two packaged instances on one PC.
- Phase 6 depends on a validated local packaged loop. Steam should expose transport/session problems, not unfinished game-loop problems.
- Do not require friends or external testers before Phase 6; the pre-Steam target is owner-run testing on one PC.
- Phase 7 can identify polish needs earlier, but should not integrate external SFX or final UX polish before the full loop is stable.
- Phase 8 is release preparation, not feature development. Any new feature request at that point should be treated as a scope change.

## Sub-phase Template

Use this mental template for every implementation handoff:

- Objective: the one outcome this sub-phase must achieve.
- Scope: the systems that may be changed.
- Out of scope: adjacent work that must wait.
- Human gate: any required owner action or approval.
- Validation: what proves the sub-phase is complete.
- Closeout: update status, note risks, and keep conception docs consistent if the contract changed.

## Phase 0 - Conception

Goal: make V1 coherent before any gameplay code.

Status: Done.

Sub-phases:

| ID | Name | Objective | Validation |
|---|---|---|---|
| 0.0 | V1 product contract | Define V1 promise, scope, V2 exclusions, and player fantasy. | `Documentation/DESIGN.md` clearly separates V1 from V2. |
| 0.1 | Rules and hidden information | Define round value, claimed count bluff, Doubt, Discard, timer, pending-win, disconnect, and public ledgers. | Rules and edge cases agree with each other. |
| 0.2 | First-party Unreal stack | Choose UE, native replication, Online Subsystem Steam, CommonUI/UMG, Enhanced Input, and no third-party or V2 plugin dependency. | `Documentation/ARCHITECTURE.md` has no avoidable beta or third-party baseline dependency. |
| 0.3 | Agent routing | Keep docs concise and routed by task. | `AGENT.md`, `CLAUDE.md`, `Documentation/AGENT.md`, and `Documentation/CLAUDE.md` are identical and explain what to read. |
| 0.4 | Human action gates | Identify purchases, account actions, asset approval, and license checks before implementation. | Roadmap human-action table lists every known owner gate. |
| 0.5 | Contradiction pass | Remove stale paths, accidental V2 scope, and implementation detail from conception docs. | No conception doc contains code fences or command blocks. |

Phase 0 is complete when:

- docs under `Documentation/` are concise enough for agents to route by task;
- V1 stack has no avoidable beta or third-party dependency;
- no conception doc contains code fences or command blocks;
- all `AGENT.md` and `CLAUDE.md` copies are identical and explain what to read.

## Phase 1 - Unreal Bootstrap

Goal: create the project foundation without gameplay complexity.

Status: Done.

Sub-phases:

| ID | Name | Objective | Validation |
|---|---|---|---|
| 1.0 | Human bootstrap gates | Confirm installed Unreal version, local Steam testing availability, and Git LFS policy before binary assets appear. | Owner decisions are recorded before project validation. |
| 1.1 | Repository hygiene | Establish ignore and LFS rules before generated Unreal folders or binary assets accumulate. | Generated folders are not tracked, and binary asset policy is clear. |
| 1.2 | Unreal project root | Use `unreal-project/Dubito/` as the Unreal project root and make it C++ capable before gameplay work. | Project opens with zero startup errors. |
| 1.3 | Module foundation | Add `DubitoCore`, `Dubito`, and `DubitoTests` responsibilities without gameplay logic. | Modules compile and match the architecture contract. |
| 1.4 | Required V1 systems | Enable Online Subsystem Steam, Online Subsystem Utils, Common UI, UMG, Enhanced Input, and Automation support. | Required systems are enabled; excluded systems remain out of V1. |
| 1.5 | Documentation handoff | Copy the conception documentation into `unreal-project/Dubito/Documentation/` without fragmenting conceptual ownership. | Unreal project contains the docs, and `Documentation/` is canonical for implementation. |
| 1.6 | Steam dev setup | Add AppID 480 local development support without touching release configuration. | Local Steam testing prerequisites are present and documented. |

Phase 1 is complete when:

- project opens with zero startup errors;
- modules compile;
- required plugins and systems are enabled;
- source control ignores generated folders;
- no binary assets are tracked outside LFS rules;
- no gameplay code has been introduced prematurely.

## Phase 2 - Core Rules

Goal: implement the engine-independent rule model first.

Status: Done.

Sub-phases:

| ID | Name | Objective | Validation |
|---|---|---|---|
| 2.0 | Card, deck, and hand primitives | Model card identity, deck integrity, deterministic shuffle/deal, and hand add/remove semantics. | Tests prove deck uniqueness, deal behavior, and hand mutation rules. |
| 2.1 | Claims, round value, and ledgers | Model announcement validity, locked round value, claimed count, claimed pile ledger, and hidden actual pile state. | Tests prove public ledgers do not expose hidden actual counts. |
| 2.2 | Action legality | Validate Play, Doubt, and Discard independently from UI and networking. | Tests cover legal and illegal action boundaries. |
| 2.3 | Doubt and pile resolution | Resolve correct Doubt, wrong Doubt, pile transfer, round reset, and claimant/doubter turn outcomes. | Tests cover value lie, count lie, both lies, and honest play. |
| 2.4 | Timer, disconnect, and pending win | Model timeout auto-play, anti-AFK disconnect, pending-win window, and last-player-standing behavior. | Tests cover timeout branches, final Doubt window, and disconnect outcomes. |
| 2.5 | Edge-case matrix pass | Align implementation tests with `Documentation/v1/EDGE_CASES.md`. | Every must-cover edge case has an explicit test or a documented validation path. |

Phase 2 is complete when:

- rule tests pass;
- rules do not depend on actors, widgets, maps, or live networking;
- hidden-count invariants are tested;
- implementation behavior matches `Documentation/DESIGN.md` and `Documentation/v1/EDGE_CASES.md`.

## Phase 3 - Greybox Table

Goal: make the game readable locally before multiplayer complexity.

Sub-phases:

| ID | Name | Status | Objective | Validation |
|---|---|---|---|---|
| 3.0 | Map shell | Done | Build Main Menu, Waiting Room, Table, and Post Game maps as greybox surfaces. | Maps exist, load, and match the planned flow. |
| 3.1 | Seated table readability | Done | Add seated camera, limited free-look, table, 8 seats, center pile anchor, and simple player placeholders. | One local player can read table orientation, seats, and center pile area. |
| 3.2 | Card and pile presentation | Done | Add simple own-hand cards, card backs, neutral opponent packets, and claimed stake presentation. | Own hand is exact; opponent packet and pile visuals do not imply actual hidden counts. |
| 3.3 | UI shell | Done | Add CommonUI/UMG shell for menu, waiting room, HUD, action bar, confirm modal, reveal panel, help card, and post-game. | Surfaces exist with placeholder content and no gameplay authority. |
| 3.4 | Input baseline | Done | Prove mouse, keyboard, and gamepad focus/confirm for menus and action bar. | No V1 UI surface requires mouse-only interaction. |
| 3.5 | Visual asset gate | Done | Decide whether internal placeholders are readable enough, or ask the owner for approved card/table placeholder assets. | Either primitives are accepted, or owner-approved asset constraints and target folders are recorded before import. |

Phase 3.5 outcome: internal Unreal primitives, generated greybox maps, and shell placeholders are accepted for the V1 greybox baseline. No external card, table, character, or UI asset is imported for Phase 3. If later owner review or playtesting finds a readability blocker, the owner must approve the asset category, license constraints, and target folder before import.

Phase 3.5 follow-up (during Phase 5): the owner supplied and approved a standard 52-card face/back image pack. The 52 face textures were imported as `Texture2D` assets into `Content/Cards/Faces/` (named `T_Card_<Suit>_<NN>`, rank `01`=Ace..`13`=King), plus a single shared `Content/Cards/T_Card_Back` and `Content/Cards/T_Card_Side`. Raw source images and the original pack archive live outside the cooked tree under `Art/Cards/` so the editor directory watcher does not auto-reimport them. License is cleared: the owner confirmed the pack is original artwork made by a personal collaborator and authorized for the project's commercial Steam use. Wiring these textures into a card material and on-table rendering is Phase 5.1+ work; the shared single card back must be reused for every face-down card to preserve the hidden-information invariants.

Phase 3 is complete when:

- one local player can see the full table layout;
- own hand and center pile areas are readable;
- no UI surface requires mouse-only interaction;
- opponent packet and pile presentation do not imply actual hidden counts;
- external visual assets, if any, were explicitly approved by the owner.

## Phase 4 - Network Framework

Goal: connect Unreal gameplay framework to the pure rule model.

Sub-phases:

| ID | Name | Status | Objective | Validation |
|---|---|---|---|---|
| 4.0 | Server authority bridge | Done | Make GameMode own authoritative match state and route rule decisions through `DubitoCore`. | Actors and widgets render or send intent; they do not decide rules. |
| 4.1 | Public GameState replication | Done | Replicate phase, active player, round value, previous public claim, claimed pile ledger, pending-win flag, and timer deadline. | Replicated fields and authoritative public snapshot are verified by automation; live multi-client PIE observation is covered by 4.7 once identity, action, and private-hand paths exist. |
| 4.2 | PlayerState identity and ledgers | Done | Replicate public player identity, seat, readiness, and public hand ledger. | Lobby and table can show public player state consistently. |
| 4.3 | Owner-only private hand | Done | Replicate exact private hand only to the owning PlayerController. | Non-owners cannot observe hand contents or actual played counts before reveal. |
| 4.4 | Server action entry points | Done | Add server actions for Ready, Start Match, Play, Doubt, and Discard. | Valid actions reach the core rules and update replicated state. |
| 4.5 | Controlled rejection and resync | Done | Handle stale turns, expired windows, illegal UI attempts, and ordinary invalid gameplay without disconnecting normal players. | Invalid ordinary actions produce a controlled rejection and state resync. |
| 4.6 | Public events | Done | Add self-contained reveal and game-over events. | Reveal and game-over UI can render from event payloads without guessing hidden state. |
| 4.7 | PIE privacy validation | Done | Run a 2 to 3 player listen-server PIE validation pass, with separate-process checks when needed. | Deal, turn advance, public claim, public ledgers, owner-only hands, and illegal-action behavior are verified. |

Phase 4.0 outcome: `ADubitoGameMode` now owns the complete hidden match state on the server side, supports deterministic match setup from dealt hands or a shuffled deck, and exposes authority-only action methods that delegate legality and state mutation to `DubitoCore`. Replication, RPC entry points, UI binding, and live PIE privacy checks remain in later Phase 4 sub-phases.

Phase 4.1 outcome: `ADubitoGameState` now carries the persistent public match snapshot for phase, active player, round value, previous public claim, claimed pile ledger, pending-win flag, and turn deadline. `ADubitoGameMode` synchronizes it after authoritative setup and rule mutations. Hidden hands, actual played cards, actual pile contents, and exact pile size remain server-only.

Phase 4.2 outcome: `ADubitoPlayerState` now carries public identity, seat, readiness, and public hand ledger. `ADubitoGameMode` can register server-side player states and synchronize their public ledgers from `DubitoCore` after setup, play, Doubt, Discard, timeout, and disconnect outcomes. Exact private hands are still not replicated and remain scheduled for Phase 4.3.

Phase 4.3 outcome: `ADubitoPlayerController` now carries the exact private hand through owner-only replication. `ADubitoGameMode` can register authority-side player controllers and synchronizes exact hands after setup, play, Doubt, Discard, timeout, and disconnect outcomes. Non-owner surfaces still receive only public player ledgers, public claims, and public match state.

Phase 4.4 outcome: `ADubitoPlayerController` now exposes reliable server actions for Ready, Start Match, Play, Doubt, and Discard. These actions resolve the server-side player id from registered controller/player state ownership, route valid gameplay through `ADubitoGameMode` and `DubitoCore`, and update public and owner-only replicated state. Detailed rejection payloads and resync feedback remain scheduled for Phase 4.5.

Phase 4.5 outcome: ordinary invalid server actions now produce controlled owner-facing rejection reasons without using RPC validation disconnect paths. Rejected Ready, Start Match, Play, Doubt, and Discard requests ask `ADubitoGameMode` to resync public and owner-only state, and automation covers stale turns, unavailable windows, bad play payloads, not-ready start, and desynchronized private-hand repair.

Phase 4.6 outcome: `ADubitoGameState` now publishes reliable public reveal and game-over events with self-contained payloads. Reveal events carry claimant, doubter, public claim, the doubted actual play, verdict, loser, claimed stake transfer, and pending-win confirmation data. Game-over events carry winner and reason for failed final Doubt, declined pending-win response, pending-win timeout, pending-win responder disconnect, last-player-standing, or no-players-remaining outcomes. Automation covers payload completeness and confirms timeout, disconnect, and declined pending-win paths do not expose reveal payloads.

Phase 4.7 outcome: CQTest PIE network automation now launches a 2-client listen-server session, registers three deterministic players, starts a match from fixed hands, verifies public deal state and owner-only private hand replication on both clients, advances a bluffable claim without leaking opponent hands, and verifies stale illegal Discard returns a controlled rejection without disconnect. CommonUI now uses `UCommonGameViewportClient` so PIE validation starts without CommonUI viewport errors.

Phase 4 is complete when:

- PIE listen-server test runs with 2 to 3 players;
- deal, turn advance, public claim, public ledgers, and owner-only hands replicate correctly;
- non-owners cannot observe actual played counts before reveal;
- illegal actions reject without disconnecting ordinary players;
- reveal and game-over events are self-contained enough for UI.

## Phase 5 - Full Gameplay Loop

Goal: make one complete local/networked game playable.

Sub-phases:

| ID | Name | Status | Objective | Validation |
|---|---|---|---|---|
| 5.0 | State display and table binding | Done | Bind replicated state to the table HUD, seats, public claim, public ledgers, timer, and action availability. | A player can always answer whose turn it is, what claim can be judged, what actions are legal, and what public stake is shown. |
| 5.1 | Play interaction | Done | Implement selection, claim controls, server confirmation, hand update, claimed ledger update, and turn advance. | Play works from input to confirmed state without exposing actual hidden count to non-owners. |
| 5.2 | Doubt interaction and reveal | Done | Implement hold-to-confirm Doubt, reveal presentation, verdict, pile transfer, and post-reveal turn outcome. | Correct Doubt, wrong Doubt, value lie, count lie, and honest play are all readable and correct. |
| 5.3 | Discard interaction | Done | Implement Discard confirmation, pile clear, round value reset, and skipped turn. | Discard is legal only in the intended states and clearly explains blocked states. |
| 5.4 | Timer and pending requests | Done | Implement turn countdown, timeout auto-play, anti-AFK disconnect, and one-pending-request behavior. | Timeout branches match the rules and spammed input cannot create duplicate actions. |
| 5.5 | Pending-win and Post Game | Done | Implement final Doubt window, win confirmation, game-over reason, and post-game presentation. | Last-card play, correct final Doubt, wrong final Doubt, timeout confirmation, and last-player-standing are validated. |
| 5.6 | Local session flow | Next | Implement local host/join or null/LAN flow from Main Menu to Waiting Room to Table to Post Game and back, suitable for same-PC loopback testing. | Two local instances can enter a match, ready up, start, finish, and return without developer tools. |
| 5.7 | First-run help | Locked | Add first-run help card, persistent help access, and contextual hints without blocking experienced play. | A new player can complete a turn and understand Doubt from on-screen cues. |
| 5.8 | Edge-case validation | Locked | Verify every relevant case in `Documentation/v1/EDGE_CASES.md`. | Winner, wrong Doubt, right Doubt, discard, timeout, disconnect, modal, resize, and input robustness paths are covered. |
| 5.9 | Packaged local full-game pass | Locked | Validate the loop by launching two packaged instances on one PC. | Two packaged local instances complete a full game while preserving hidden information rules. |

Phase 5.0 outcome: a pure, engine-independent presentation view-model (`BuildDubitoTableHudSnapshot`) derives the table HUD from
replicated public state plus the local player's own identity and hand: whose turn it is, the previous public claim and whether
it is doubtable, per-seat public ledgers, the claimed pile stake, the turn countdown, and best-effort Play/Doubt/Discard
availability that mirrors the server legality predicates. `ADubitoGameState` now also replicates a public
`bPreviousClaimDoubtable` flag and broadcasts a native state-changed delegate, and `ADubitoPlayerState` broadcasts an
equivalent delegate, so a C++ HUD refreshes event-driven from replicated state. `UDubitoTableHudWidget` renders the snapshot and
`ADubitoHUD` (wired as the GameMode `HUDClass`) shows it to the owning client on the Table map. The view-model is covered by
automation (whose-turn, action matrix incl. pending-win and stale-claim, claim/stake presentation, timer math, seat ledgers);
the widget still needs an on-screen in-match visual pass, which depends on the Phase 5.6 ready/start flow (or a live PIE session
driven through the editor) to populate a real match.

Phase 5.1 outcome: a pure, engine-independent Play composition view-model (`BuildDubitoPlaySelectionView`) turns the local
player's exact hand plus the replicated public round context plus in-progress intent (picked card indices, chosen claimed
value, claimed count) into the flags an action bar needs and a submittable `(ActualCards, Announcement)` payload. It reuses
`DubitoRules::IsAnnouncementValidForRound` so the value-lock rule is never duplicated: the value control is exposed only while
opening a round and locked to the round value once open, while the claimed count stays a free 1..4 bluff. Selection is gated to
1..4 unique in-hand cards and mirrors the server Play predicates as a best-effort client gate; the server still validates and
resyncs. `UDubitoPlayActionWidget` (added to the Table HUD via `ADubitoHUD`, gated to the owning local player) reads the exact
hand from `ADubitoPlayerController`, drives selection/claim through discrete cursor + toggle + stepper controls so it is usable
without a mouse, and confirms by calling the existing `RequestPlayCards` server path, after which the confirmed hand/ledger/turn
arrive as replicated state and clear the in-progress intent. The view-model is covered by automation (turn gating, opening vs
locked value, selection bounds, claimed-count bounds incl. bluff, payload determinism); the widget still needs an on-screen
in-match interaction pass, which depends on the Phase 5.6 ready/start flow (or a live PIE session driven through the editor) to
populate a real turn. It only ever reads the local exact hand, so it cannot leak an opponent's actual count.

Phase 5.2 outcome: a pure, engine-independent reveal presentation view-model (`BuildDubitoRevealView`) turns the self-contained
public `FDubitoRevealInfo` plus the local player's identity into legible facts: the verdict (correct vs wrong doubt), the exact
lie kind (honest, count lie, value lie, or both) derived from the claim vs the revealed cards, the claim/actual comparison, who
takes the pile and the claimed stake moved, any confirmed win, and local-perspective flags (claimant/doubter/loser/winner). It
reads only the server-scrubbed public payload, so it never leaks hidden state. `ADubitoGameState` now also exposes native
`OnPublicRevealEvent`/`OnPublicGameOverEvent` broadcasts (mirroring `OnPublicMatchStateChanged`) so a C++ widget renders the beat
event-driven. `UDubitoRevealWidget` (added to the Table HUD via `ADubitoHUD`) presents the reveal and game-over cards and
auto-hides the reveal after a short beat, and `UDubitoPlayActionWidget` gained hold-to-confirm Doubt: a press begins the hold, a
release before the threshold cancels, and holding past it sends the existing `RequestDoubt` server path, gated by a public-state
doubtability check that mirrors `DubitoRules::CanDoubt`. The view-model is covered by automation (invalid/default, honest wrong
doubt, count lie, value lie, count-and-value lie, and win-confirmed with local-perspective flags); the on-screen reveal beat and
the Doubt hold interaction still need a live in-match pass, which depends on the Phase 5.6 ready/start flow (or a live PIE
session driven through the editor) to populate a real Doubt.

Phase 5.3 outcome: a pure, engine-independent Discard availability view-model (`BuildDubitoDiscardView`) mirrors
`DubitoRules::CanDiscard` from public state and, when Discard is unavailable, names the specific blocked reason (not your turn,
a win is pending, or an empty pile) so the disabled control can explain itself. `UDubitoPlayActionWidget` gained a Discard
control with a light confirmation: the first click arms it (with a short auto-disarm timer), a second click within the window
sends the existing `RequestDiscard` server path, and the status line always explains the current available/armed/blocked state.
The server still owns the pile clear, round-value reset, and skipped turn, which arrive as replicated state. The view-model is
covered by automation (available case, and each blocked reason with turn precedence); the on-screen confirmation still needs a
live in-match pass, which depends on the Phase 5.6 ready/start flow (or a live PIE session driven through the editor).

Phase 5.4 outcome: `ADubitoGameMode` now owns an authoritative turn timer (`FTimerHandle` kept in lockstep with the turn
deadline in `RefreshTurnDeadlineForCurrentState`): when a turn's `TurnSeconds` elapse it fires `AuthorityResolveTimeout` for the
player on the clock, which routes through `DubitoRules::ResolveTimeout` to auto-play one card (truthful when possible, a forced
minimal bluff otherwise), confirm a pending winner instead when a win is pending, and count the anti-AFK streak so the third
consecutive timeout removes the player via `HandleDisconnect`. The timer reschedules for the next player after each voluntary or
timed-out action and clears on any non-`PlayerTurn` phase (reveal, game over); `IsTurnTimerActive` and
`GetTurnDeadlineServerTimeSeconds` expose it for validation. Spammed input cannot create duplicate actions: the server already
rejects any second action from a player who is no longer active (the authoritative turn/`CanDoubt` checks), and
`UDubitoPlayActionWidget` adds a one-pending-request guard that blocks a second Play/Doubt/Discard send until replicated state
confirms the first (with a short safety window in case a rejected action produces no state change). Automation covers the timer
being scheduled during a turn and cleared at game over, timeout auto-play advancing the turn, stale timeout callbacks being a
safe no-op, and duplicate Play/Doubt being rejected without double-applying; the turn countdown display itself already ships from
Phase 5.0, and the on-screen pending/timeout feel still needs a live in-match pass gated on the Phase 5.6 flow.

Phase 5.5 outcome: the pending-win rules and the server-side game-over broadcasting already shipped (core `ApplyPlay` flags the
pending win, `ResolveDoubt`/`ConfirmWin`/`ResolveTimeout`/`HandleDisconnect` resolve it, and `ADubitoGameMode` emits the reveal +
`FDubitoGameOverInfo` with the correct `EDubitoGameOverReason` in Phases 4.6/5.4). Phase 5.5 added the client terminal
presentation. `EDubitoGameOverReason`/`FDubitoGameOverInfo` moved into a light `DubitoGameOver.h` so a pure view-model can use
them without depending on the GameState actor. A new pure `BuildDubitoPostGameView` turns the game-over payload plus the local
identity into the terminal result: the winner (or none), the human-facing reason, and whether the local player won, lost, or
watched. `UDubitoPostGameWidget` (added to the Table HUD via `ADubitoHUD`, top-most) binds the native `OnPublicGameOverEvent` and
presents a persistent result card, and the reveal panel's game-over handling moved here so the reveal stays the cinematic beat and
post-game owns the terminal screen. The final Doubt window itself is already covered: during a pending win the action bar keeps
Play available and the previous last-card claim doubtable (emphasized hold-to-confirm Doubt), and the HUD shows the pending-win
stake. Automation covers the post-game view-model (invalid/default, win/loss/spectator perspective, no-winner ending, and every
rule-defined winning reason). The terminal card's return-to-menu affordance is intentionally deferred to the Phase 5.6 local
session flow, and the on-screen post-game/pending-win feel needs that same live pass.

Phase 5 is complete when:

- two packaged local instances on one PC can complete a full game;
- winner, wrong Doubt, right Doubt, discard, timeout, and disconnect paths are all validated;
- public ledger UI remains understandable and never pretends hidden counts are exact;
- no required V1 flow depends on developer-only tools.

## Phase 6 - Steam Multiplayer

Goal: prove the real Steam transport.

Sub-phases:

| ID | Name | Objective | Validation |
|---|---|---|---|
| 6.0 | Steam test gate | Confirm two Steam accounts, two test machines, AppID 480 expectations, and owner availability. | Human prerequisites are explicit before Steam validation begins. |
| 6.1 | Steam session configuration | Configure Steam lobbies and sessions through Online Subsystem Steam. | Host session creation works outside PIE with Steam running. |
| 6.2 | Discovery, join, and invite | Test host, find, join, invite, full lobby, not found, and already-started paths. | Join paths are reliable enough for V1 and failure states are understandable. |
| 6.3 | Steam travel flow | Validate waiting room to table and back through the Steam session path. | Steam travel does not bypass readiness, rules, or privacy validation. |
| 6.4 | Two-machine full-game test | Play a full game on two machines with two Steam accounts after the one-PC packaged test has passed. | Both players complete a game; each sees only their own exact hand. |
| 6.5 | Steam regression pass | Re-test host leave, disconnect, illegal actions, and hidden-information invariants under Steam. | No Steam-only path bypasses rule validation or hidden information rules. |

Phase 6 is complete when:

- Steam-hosted match works outside PIE;
- invite and join flow is reliable enough for V1;
- host leave ends the session gracefully;
- no Steam-only path bypasses rule validation or hidden information rules.

## Phase 7 - V1 Polish

Goal: make the playable game clear, robust, and shippable enough for first release testing.

Sub-phases:

| ID | Name | Objective | Validation |
|---|---|---|---|
| 7.0 | Disabled-action clarity | Pass through every disabled-action reason on hover, focus, or press. | Players understand why every unavailable action is blocked. |
| 7.1 | Input and layout robustness | Pass through gamepad, keyboard, mouse, resize, and common desktop resolutions. | Critical text and action controls remain visible and usable. |
| 7.2 | Accessibility and motion | Add reduce-motion support and readable pending-state feedback. | Reduced motion preserves clarity without cinematic camera movement. |
| 7.3 | SFX gate and integration | Ask the owner for approved SFX only after exact event needs are known, then add simple feedback sounds. | Audio exists for key actions without introducing unapproved assets. |
| 7.4 | Repeated playtest hardening | Run repeated full-game playtests and fix friction, dead ends, and unclear states. | Packaged build survives repeated full-game playtests. |
| 7.5 | External playtest build | Package a Development build suitable for owner-approved external playtest. | No V1 flow requires developer tools, and known limitations are recorded. |

Phase 7 is complete when:

- new players understand turn, claim, legal actions, and public stake without verbal explanation;
- no V1 flow requires developer tools;
- packaged build survives repeated full-game playtests;
- any external SFX were owner-approved with license and target folder constraints.

## Phase 8 - Release Prep

Goal: prepare the Steam release path after V1 is proven.

Sub-phases:

| ID | Name | Objective | Validation |
|---|---|---|---|
| 8.0 | Steamworks human gate | Purchase or activate the real Steam app credit and obtain the real AppID. | Owner account/payment actions are complete. |
| 8.1 | Release app configuration | Replace development AppID usage with release configuration. | Shipping build launches through Steam with the real AppID. |
| 8.2 | Depot hygiene | Ensure local development files are not shipped in the Steam depot. | Release package contains no local-only Steam development file. |
| 8.3 | Store assets and screenshots | Prepare or import owner-approved capsule art, screenshots, description, and store metadata. | Steamworks store checklist has no blocking asset or metadata item. |
| 8.4 | Release packaging validation | Validate the final Shipping build and depot upload path. | Steamworks release checklist has no blocking technical item. |
| 8.5 | V1 scope freeze | Freeze V1 unless a release blocker appears. | No new feature work enters the release branch without explicit scope change. |

Phase 8 is complete when:

- Steamworks release checklist has no blocking technical item;
- Shipping build launches through Steam;
- V1 scope is frozen unless a release blocker appears.

## Human Actions By Phase

Agents must call these out when the phase or sub-phase is reached instead of silently assuming them.

| Phase or sub-phase | Human action | Trigger |
|---|---|---|
| 1.0 | Confirm installed Unreal version and Steam account availability for local testing | before bootstrap validation |
| 1.0 | Confirm Git LFS and locking policy | before any `.uasset`, `.umap`, audio, or texture file is tracked |
| 1.0 | Install UE C++ build prerequisites: Visual Studio 2022 C++ toolchain, Windows 10/11 SDK, and .NET Framework 4.6+ SDK (required by SwarmInterface) | before the first C++ module build |
| 3.5 | Select or approve external visual assets only if placeholders are not readable enough | before import into `unreal-project/Dubito/Content/Cards/` or `unreal-project/Dubito/Content/Art/Prototype/` — card face/back pack supplied and imported during Phase 5; license cleared (original art by owner's collaborator, commercial use authorized) |
| 6.0 | Provide two Steam accounts and two test machines, or explicitly accept a reduced test pass | after the packaged one-PC full-game pass, before Steam multiplayer validation |
| 7.3 | Select or approve external SFX only after exact event needs are known | before import into `unreal-project/Dubito/Content/Audio/SFX/` |
| 8.0 | Purchase or activate Steam app credit and provide real AppID | before release app configuration |
| 8.3 | Provide or approve Steam store assets and marketing copy | before release packaging |

## Agent Closeout Criteria

Before an agent marks any sub-phase complete:

- the validation criteria for that exact sub-phase are met or the sub-phase stays in progress;
- any changed rule, architecture, UX, asset, or roadmap decision is reflected in the matching conception doc;
- hidden-information and owner-approval constraints have not been weakened;
- no conceptual doc gained code fences or command blocks;
- remaining risks are named in the handoff instead of being hidden in implementation details.
