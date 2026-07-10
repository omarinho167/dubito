# Dubito — Development Roadmap

> This file is the execution plan. Each phase has a clear owner, an explicit file list, and a testable "done" criterion.
> **Living document** — update phase statuses and task boxes as work progresses.

---

## Environments

| Environment | When | Claude Code capabilities |
|---|---|---|
| **Web session** | Conception only | File read/write, git — no Unreal, no shell execution |
| **Local machine** | All development | Full shell, Unreal (UBT/UAT/`UnrealEditor-Cmd`), Steam, git, **+ Unreal MCP server** |

> **Unreal MCP** — target an Unreal MCP server connected on the local machine (the parallel to the reference project's
> in-editor MCP). It should let Claude Code drive the Editor directly: spawn/configure actors & components, create
> materials/Blueprints/Data Assets, build levels, compile, run Automation tests, read the Output Log, and capture
> viewport/PIE frames for visual QA. **Confirm which MCP is installed at the start of Phase 1 and record it here.**
> Until confirmed, MCP-driven steps have a human-in-editor fallback.

> **A structural advantage over the reference project:** Unreal's **Play In Editor (PIE) multiplayer** ("Number of
> Players" > 1, Net Mode = *Play As Listen Server*) runs a real host + N clients **inside the editor**. Networking
> (Phases 3 & 5) is therefore testable with no second machine. Only the actual **Steam transport** (Phase 6) needs two
> machines + two Steam accounts.

> **Path note:** the Unreal project lives at `DubitoUE/` (holding `Dubito.uproject`, `Source/`, `Content/`, `Config/`).
> These conception docs currently live in `docs-unreal/`; at bootstrap they move to `DubitoUE/Documentation/` and
> `CLAUDE.md` becomes the project-root instruction file.

---

## Phase Overview

| Phase | Name | Environment | Status |
|---|---|---|---|
| 0 | Conception | Web | ✅ Done |
| 1 | Unreal Bootstrap | Local | ⏳ Next |
| 2 | Core Game Logic | Local | 🔒 |
| 3 | Network Layer | Local | 🔒 |
| 4 | Levels & UI | Local | 🔒 |
| 5 | Gameplay Integration | Local | 🔒 |
| 6 | Multiplayer Test (Steam) | Local | 🔒 |
| 7 | V1 Polish | Local | 🔒 |
| 8 | Build & Release | Local | 🔒 |

---

## Phase 0 — Conception ✅

> Define every rule, constraint, and design decision before writing a single line of code.

**Environment:** Web session
**Output (consolidated to 4 files):** `DESIGN.md`, `ARCHITECTURE.md`, `ROADMAP.md`, `CLAUDE.md` (in `docs-unreal/`)

All decisions documented:
- [x] Game rules + edge cases (locked round value, doubt, turn timer, disconnect, win/pending-win)
- [x] Tech stack — **UE 5.8**, C++ core + UMG, native replication, **OSS Steam + Advanced Sessions**
- [x] Steam strategy (AppID 480 for dev, listen-server host, SDR relay free)
- [x] Network model (host-authoritative GameMode/GameState/PlayerState/PlayerController; owner-only private hands)
- [x] Art direction (infernal tavern; **greybox V1** → **Fab/MetaHuman/Lumen V2**)
- [x] UX, interactions, motion & **exhaustive edge-case coverage** (`DESIGN.md` Parts B–D); HUD = **hybrid** (in-world +
      minimal pinned overlay), motion = **sober + cinematic doubt reveal**, reduce-motion baseline
- [x] Announcement rule = **value locked per round, count is the free lie**
- [x] Ready system + post-game loop

---

## Phase 1 — Unreal Bootstrap ⏳

> Clean UE 5.8 C++ project, two modules, Steam plugins enabled, committed to git.

**Environment:** Local machine
**Prerequisites:** Phase 0 ✅

### Claude Code tasks
- [ ] Create a **UE 5.8 C++ project** `DubitoUE` (Blank/Games template, no starter content) at the repo location.
- [ ] Add the **`DubitoCore`** module (`Source/DubitoCore/`, `DubitoCore.Build.cs` — deps: `Core`, `CoreUObject` **only**)
      and the primary **`Dubito`** module (deps: `Engine`, `DubitoCore`, `OnlineSubsystem`, `OnlineSubsystemSteam`,
      `UMG`, `Slate`, `EnhancedInput`).
- [ ] Add `steam_appid.txt` (`480`) at the project root.
- [ ] Configure `Config/DefaultEngine.ini`: enable OSS Steam (`[OnlineSubsystem] DefaultPlatformService=Steam`,
      `[OnlineSubsystemSteam] bEnabled=true SteamDevAppId=480`), NetDriver for Steam sockets, default maps.
- [ ] Install **Advanced Sessions** + **Advanced Steam Sessions** plugins (branch matching UE 5.8) into `Plugins/`.
- [ ] Enable built-in UI/input plugins: **Common UI**, **Model View Viewmodel (MVVM)**, **Enhanced Input** (default),
      + **Functional Testing Editor** (for automation). Prove the pipeline with one "hello" CommonUI widget.
- [ ] Add a root `.gitignore` for Unreal (`Binaries/`, `Intermediate/`, `DerivedDataCache/`, `Saved/`, `.vs/`).
- [ ] Initialise **Git LFS** + a `.gitattributes` marking `*.uasset`/`*.umap` (+ textures/audio/models) as `lockable`
      binary, **before the first asset lands** (see `ARCHITECTURE.md` §5).
- [ ] Commit the initial project.

### Human / Unreal-MCP tasks
- [ ] Install/confirm the **Unreal MCP server** and record its name + capabilities here.
- [ ] Open `DubitoUE` in UE 5.8 — project compiles, editor opens with **zero errors**.
- [ ] Verify OSS Steam + Advanced Sessions are enabled in Plugins and the Steam overlay initializes (AppID 480).
- [ ] Install the **Fab** plugin (for later asset pulls).
- [ ] Source the V1 **3D card pack** from Fab → `Content/Cards/` (needed early; card mechanics depend on it).
- [ ] Commit project + card pack (`Binaries/`, `Intermediate/`, `Saved/` ignored).

### Validation — done when:
- The project compiles from a clean checkout (`UnrealBuildTool` succeeds) and the editor opens with **0 errors**.
- `DubitoCore` and `Dubito` modules both build; OSS Steam + Advanced Sessions visible & enabled in Plugins.
- `git status` shows only tracked files (`Binaries/`/`Intermediate/`/`Saved/`/`DerivedDataCache/` ignored).

---

## Phase 2 — Core Game Logic 🔒

> Pure C++ game logic with full Automation test coverage. Zero `Engine` dependency.

**Environment:** Local machine — Claude Code autonomous
**Prerequisites:** Phase 1 ✅

### Claude Code tasks

**In `Source/DubitoCore/`:**
- [ ] `GameConstants.h` — all constants (`MinPlayers`, `MaxPlayers`, `DeckSize`, `Min/MaxCardsPerPlay`, `Min/MaxCardValue`,
      `TurnSeconds`, `MaxConsecutiveTimeouts`, `NoRoundValue`, `NoPlayerId`).
- [ ] `Card.h/.cpp` — `USTRUCT FCard { ESuit, uint8 Value }` (validates range, `operator==`, `NetSerialize`).
- [ ] `Deck.h/.cpp` — 52-card deck, `Shuffle(FRandomStream&)` (injected RNG), `Deal(int32)`.
- [ ] `Hand.h/.cpp` — `AddCards`, all-or-nothing `RemoveCards`, `IsEmpty`.
- [ ] `Announcement.h` — `USTRUCT FAnnouncement { uint8 ClaimedValue, uint8 ClaimedCount }`.
- [ ] `PlayAction.h` — `PlayerId`, `ActualCards` (server-only by convention), `Announcement`.
- [ ] `GameStateData.h/.cpp` — `EGamePhase` + `FGameStateData`; `AdvanceTurn`, prev/active resolution, potential-win,
      round-value open/reset.
- [ ] `GameOverReason.h` — `EGameOverReason { LastPlayConfirmed, WrongDoubt, LastPlayerStanding }`.
- [ ] `TurnValidator.h/.cpp` — `IsValidPlay`, **`IsClaimedValueAllowed`** (locked-value rule), `IsDoubtCorrect`
      (count OR value mismatch vs the round value).

**Tests in `Source/DubitoTests/` (Automation Spec):**
- [ ] `Deck.spec.cpp` — 52 cards, all suits/values, deal, shuffle integrity (deterministic seed).
- [ ] `Hand.spec.cpp` — add/remove, all-or-nothing removal, IsEmpty.
- [ ] `TurnValidator.spec.cpp` — valid-play bounds, **locked value enforcement**, doubt correctness (count lie, value
      lie, honest).
- [ ] `GameState.spec.cpp` — AdvanceTurn wrap-around, potential-win flag, round-value open/reset on discard & doubt.

### Validation — done when:
```bash
# Headless Automation run (Editor closed):
UnrealEditor-Cmd.exe DubitoUE.uproject -ExecCmds="Automation RunTests Dubito.Core; Quit" -unattended -nop4 -nosplash -log
# (or Session Frontend ▸ Automation ▸ run the DubitoCore group in-editor)
```
- All DubitoCore Automation tests **PASS**, zero failures. `DubitoCore` links without `Engine`.

---

## Phase 3 — Network Layer 🔒

> Host-authoritative gameplay over native UE replication. Private hands owner-only. Local-first sessions.

**Environment:** Local machine — Claude Code autonomous (validated in **PIE multiplayer**)
**Prerequisites:** Phase 2 ✅

### Claude Code tasks

**In `Source/Dubito/`:**
- [ ] `DubitoGameMode.h/.cpp` — **server-only** authority: owns `FGameStateData`, deals privately, validates every action
      via `DubitoCore`, resolves doubts/wins, runs the turn timer.
- [ ] `DubitoGameState.h/.cpp` — replicated public state (phase, active id, pile **count**, `RoundValue`, turn order,
      public claim, pending-win flags, `TurnEndsServerTime`) + `OnRep_*` + C++ delegates for the UI.
- [ ] `DubitoPlayerState.h/.cpp` — replicated `PlayerId`, `DisplayName`, `SeatIndex`, `CardCount`, `bReady`.
- [ ] `DubitoPlayerController.h/.cpp` — `Server_PlayCards/Server_Doubt/Server_DiscardPile/Server_SetReady`
      (Reliable, WithValidation); **owner-only** private hand (`ReplicatedUsing=OnRep_Hand`, `COND_OwnerOnly`);
      `Multicast_*`/`Client_*` receive points.
- [ ] `DubitoGameInstance.h/.cpp` + `Session/DubitoSessionSubsystem.h/.cpp` — OSS session lifecycle (Host/Find/Join/
      Destroy), transport-agnostic; **local/null + LAN first** (host + join-by-IP), Steam wired but deferred to Phase 6.
- [ ] Reveal/gameover payloads: `USTRUCT FRevealInfo`, `FGameOverInfo` for `Multicast_RevealCards` / `Multicast_GameOver`.

### Validation — done when (in **PIE**, Net Mode = *Play As Listen Server*, 2–3 players):
- Zero compile errors; classes appear with their replicated properties.
- A host + clients connect in PIE; dealing, a play, and turn advance replicate to all; **pile count** updates for
  everyone while **actual cards stay hidden**.
- Each PIE client sees **only its own hand** (owner-only replication verified — no hand data in other clients' state).
- Turn timer deadline replicates; a forced timeout auto-resolves on the server.

---

## Phase 4 — Levels & UI 🔒

> Three maps. First-person camera. Functional (not yet wired) HUD. Greybox tavern.

**Environment:** Local machine — Claude Code via Unreal MCP
**Prerequisites:** Phase 3 ✅

### Claude Code tasks (actors & pawns — `Source/Dubito/`)
- [ ] `ASeatPawn` — **free-look** first-person camera rig at the local seat (eye height; clamped yaw/pitch, recenter + idle
      auto-recenter; screen-space HUD unaffected; cinematic beats take/return control). See `DESIGN.md` Part E.
- [ ] Shared **animation token set** + a code-based UI **tween helper** (durations/easings) used by cards & HUD
      (`DESIGN.md` §C.3; tech `ARCHITECTURE.md` §5).
- [ ] `ACardActor` — thin card mesh, runtime face material (MID) + uniform back, flip, click collider, hover/selection outline.
- [ ] `APileActor` — center-pile anchor, stacked face-down cards, big count label.
- [ ] `ASeatActor` — seated placeholder figure + billboarded name/count labels + turn highlight + claim bubble.
- [ ] `CardDataTable` / Data Asset — `(ESuit,Value) → face Texture2D` + uniform back (52 faces, 0 missing).

### Claude Code tasks (UI — **CommonUI + MVVM + Enhanced Input**; C++ bases + `WBP_` subclasses — `Source/Dubito/UI/` + `Content/UI/`)
- [ ] HUD **viewmodels** (MVVM) fed by `ADubitoGameState`/`ADubitoPlayerState` `OnRep_`; modals & menus as **CommonUI
      activatable widgets** (input routing, gamepad/Steam Deck). UX principles: `DESIGN.md` §B.0.
- [ ] `UHUDRootWidget`, claim banner, round tag, turn banner/prompt (+ timer ring), pile count.
- [ ] `UActionBarWidget` (Play/Doubt/Discard), `UAnnouncePanelWidget`, `UDoubtRevealPanelWidget`, `UConfirmPanelWidget`.
- [ ] `UMainMenuWidget`, `UWaitingRoomWidget`, `UPostGameWidget`.

### Claude Code tasks (levels via Unreal MCP — `Content/Maps/`)
- [ ] `MainMenu.umap` — menu widget over a torch-lit backdrop.
- [ ] `WaitingRoom.umap` — player-list UI + session/NetworkManager wiring.
- [ ] `Table.umap` — **greybox infernal tavern**: box room + floor, warm point lights (Lumen), a table, **8 seat
      positions** ringed with a center-pile anchor, `ASeatPawn` at the local seat, HUD added.
- [ ] Register the three maps as default/travel maps in `DefaultEngine.ini`.

### Validation — done when (self-verified via MCP viewport/PIE capture):
- Navigate MainMenu → WaitingRoom → Table with 0 errors.
- First-person **free-look** camera renders at seated eye height (clamped pan works, recenter works); the rest framing shows the greybox table, 8 seats + pile anchor.
- `ACardActor` renders the correct face from the Data Table (a demo hand verified).
- HUD + action bar present (button behavior wired in Phase 5).

---

## Phase 5 — Gameplay Integration 🔒

> Full local loop works end-to-end and is **intuitive**: a first-timer knows whose turn it is, what was claimed, and
> what they can do — at every instant.
>
> **UX is the deliverable.** Read `DESIGN.md` **Parts B–D** (UX foundations; HUD/interactions/motion; and the Part D
> edge-case matrix) before each sub-phase. The edge-case coverage is a build checklist, not just prose — each case has a
> defined response to implement. Animations are the **presentation layer** (`DESIGN.md` §C.4): they play server-confirmed
> state and finish-fast/snap on interruption — they never gate the host.

**Environment:** Local machine — Claude Code (validated in PIE)
**Prerequisites:** Phase 4 ✅ · design ref: `DESIGN.md` Parts B–D (UX, HUD/interactions/motion, edge-case matrix)

Split into small, individually testable sub-phases — *see state correctly before allowing input*, then add **one action
at a time** (Play → Doubt → Discard), then clarity/feedback, then win/end, then lobby flow, then onboarding.

- [ ] **5.1 Read-only state display** — HUD binds to `ADubitoGameState` `OnRep_*`: turn banner, claim banner, round tag,
      pile count, per-seat names/counts/glow. No inputs yet.
- [ ] **5.2 Hand render & selection** — private hand → `ACardActor`s at the bottom anchor; hover-lift + selection outline;
      the 1–4 cap + "Selected: n/4" helper.
- [ ] **5.3 PLAY** — action-bar gating (§5 matrix); announce panel (count stepper always; value stepper only when
      opening); `Server_PlayCards`; hand resyncs via owner-only replication; pile ticks; turn passes.
- [ ] **5.4 DOUBT + reveal** — hold-to-doubt; `Multicast_RevealCards` → `UDoubtRevealPanelWidget` claim-vs-actual + verdict
      + transfer; host authoritative for the pile move; round value resets.
- [ ] **5.5 DISCARD** — confirm panel → `Server_DiscardPile`; pile clears, round value + claim reset, turn skips; blocked
      during pending win.
- [ ] **5.6 Clarity & timer** — contextual prompt line (all §5 branches + spectator line), primary-action emphasis;
      45 s server timer (replicated deadline), timeout auto-resolve, 3-strike drop, "Auto-played" toast.
- [ ] **5.7 Win / pending-win / PostGame** — pending-win banner + Discard block; `UPostGameWidget` (winner name + plain
      reason; Victory/Defeat).
- [ ] **5.8 Lobby & scene flow** — Main Menu Host / Join-by-address (local), Waiting Room ready + host Start gating,
      `ServerTravel` MainMenu → WaitingRoom → Table → PostGame → WaitingRoom; Play Again / Ready loop.
- [ ] **5.9 Onboarding (light stub)** — first-run hints (open / follow / doubt), persistent `?` help card (`DESIGN.md` §C.5).

### Validation — Phase 5 done when:
- From a packaged build launched **twice on one PC** (host + join `127.0.0.1`), two players play a full game to a win and
  return to the Waiting Room; each sees only their own hand.
- Play / Doubt / Discard each respond correctly (state×action matrix holds); the doubt reveal shows claim-vs-actual + the
  correct verdict + pile transfer.
- A first-time player, from on-screen cues alone, can tell whose turn it is, what was claimed, and act correctly.

---

## Phase 6 — Multiplayer Test (Steam) 🔒

> Real 2-player game over Steam P2P (OSS Steam).

**Environment:** Local machine — human + Claude Code
**Prerequisites:** Phase 5 ✅ + 2 Steam accounts + AppID 480

### Claude Code tasks
- [ ] Swap the session path to **OSS Steam** (Create → Steam session; Join via **Steam invite** + **lobby code**);
      `DubitoSessionSubsystem` Steam wiring; connecting/error states ("Lobby full / not found / already started").
- [ ] Package a **Development** Win64 build (via UAT / MCP).
- [ ] Fix networking bugs found in real Steam testing.
- [ ] Verify **hand privacy** over the wire (a client never receives another player's hand — owner-only holds under Steam).
- [ ] Handle mid-game disconnect (discard hand, remove from turn order; host-leave ends session cleanly).
- [ ] *(Optional)* stand up a **Gauntlet** smoke test (server + 2 clients, scripted play → doubt → win) for CI
      regressions — see `ARCHITECTURE.md` §5. PIE multiplayer stays the daily driver.

### Human tasks
- [ ] Run the build on two machines (or one + Steam family share) — two signed-in Steam clients.
- [ ] P1 Create → invite/share code; P2 Join; both Ready → host Start; play a full game.

### Validation — done when:
- 2 players connect via Steam (AppID 480); each sees only their own hand.
- All 3 actions work in real multiplayer; disconnect mid-game doesn't crash the remaining player.
- Full game completes with the correct winner.

---

## Phase 7 — V1 Polish 🔒

> Minimum viable feel. Names, card animation, SFX, atmosphere.

**Environment:** Local machine — Claude Code + human
**Prerequisites:** Phase 6 ✅

### Claude Code tasks
- [ ] Assign predefined names (Mortem, Lucian, Vesper, Cain, Malachar, Seraph, Dante, Abyssia) + per-seat colors by seat.
- [ ] Implement the **animation catalog** V1 tier (`DESIGN.md` §C.3): deal, select-lift, play-arc-to-pile, **lift-then-flip**
      reveal, pile transfer, discard — all as functional tweens, **presenting confirmed state** with finish-fast/snap on
      interruption (`DESIGN.md` §C.4). Reduce-motion mode honored.
- [ ] Placeholder audio on key events (`card_place`, `card_flip`, `doubt_call/correct/wrong`, `win`).
- [ ] Polish the PostGame screen (winner name, reason, play count).
- [ ] Basic fire/point-light atmosphere in the Table map (still greybox, but warm + moody via Lumen).

### Validation — done when:
- Player names display above the figures; cards animate onto the pile; audio plays on key events; PostGame shows the winner.

---

## Phase 8 — Build & Release 🔒

> Steam Direct submission ready.

**Environment:** Local machine — human + Claude Code
**Prerequisites:** Phase 7 ✅ + Steam Direct app purchased ($100)

### Human tasks
- [ ] Purchase Steam Direct ($100 — recouped at $1,000 sales).
- [ ] Set up the Steamworks App page (store description, screenshots, capsule art) + achievements in the dashboard.

### Claude Code tasks
- [ ] Replace `480` → real AppID in `steam_appid.txt` + `DefaultEngine.ini` (drop `SteamDevAppId`).
- [ ] Configure packaging: **Shipping**, Win64; cook content; verify the Steam overlay with the real AppID.
- [ ] Set up the depot upload script (`steamcmd` / Steamworks SDK ContentBuilder).

### Validation — done when:
- A Shipping build runs with the real AppID (Steam overlay appears); achievements fire; submitted to the Steam review queue.

---

## Note on V2 (post-V1, out of this roadmap's scope)

Once V1 ships, the visual payoff of the engine switch begins (see `DESIGN.md` Part E → V2): Fab/Megascans tavern, MetaHuman souls +
reactive animation, Niagara fire/embers, Lumen/Nanite finish, post-processing, custom card art, the **emote/gesture wheel**
(designed in `DESIGN.md` Part E — `Server_Emote` → `Multicast_Emote`, cosmetic + rate-limited), and the **Guillotine**
mechanic (`DESIGN.md` Part A). None of it blocks V1; all of it is additive.
