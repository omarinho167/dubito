# Dubito — Architecture & Technology

> The complete **"how it's engineered"**: the tech/plugin study (with rejected alternatives), the networking model, the
> data model, the folder structure, and the constants. The **"what to build"** (rules, UX, interactions, art) is in
> `DESIGN.md`; the **execution order** is in `ROADMAP.md`; conventions + entry point are in `CLAUDE.md`.
>
> Living document. Constants and rule-validation here must always match `DESIGN.md` Part A — if they diverge, `DESIGN.md`
> Part A wins for the *rule* and this file wins for the *implementation*; fix both.

**Contents:** 1) Stack & decisions · 2) Engine & modules · 3) Networking & online (study) · 4) UI framework (study) ·
5) Other subsystems (audio/data/persistence/rendering/source-control/testing/build) · 6) Game state machine · 7) Data
model · 8) Network architecture · 9) Project structure · 10) Constants · 11) Deliberately NOT using · 12) Out of scope.

> **Guiding principle:** prefer first-party + built-in; adopt the modern UE stacks that *reduce* long-term complexity and
> improve UX (CommonUI, MVVM, Enhanced Input); refuse anything a small, simple card game doesn't need. Restraint = simplicity.

---

## 1. Stack & decisions

Every choice, with the alternatives considered and **why rejected/deferred**:

| Subsystem | Choice (V1) | Why | Considered & rejected/deferred |
|---|---|---|---|
| Engine | **UE 5.8** (fallback 5.7) | Latest UE5; the **last major UE5** release → stable, long-lived (UE6 EA end-2027) | 5.6/5.7 (fine fallbacks if a plugin lags) |
| Language | **C++** (logic/net) + **UMG/Blueprint** (UI/cosmetics) | Testable, correct replication; fast visual iteration where it helps | Blueprint-only (untestable logic), pure-C++ UI (slow iteration) |
| Networking | **Native UE replication** (listen server, host-authoritative) | First-party, documented, maps 1:1 to the design | Any third-party netcode — unnecessary in Unreal |
| Online / Steam | **OSS Steam (v1)** + **Advanced Sessions** plugin | Best-documented Steam path; free; invites/overlay/code; blueprint-friendly | EOS, Online Services (OSSv2), SteamCore PRO, Photon — §3 |
| UI framework | **CommonUI** + **UMG Viewmodel (MVVM)** + **Enhanced Input** | Modern, input-agnostic (KBM/gamepad/**Steam Deck**), clean data-binding, less UI spaghetti | Plain UMG only (polling, input pain), full Slate (overkill) |
| Audio | **SoundCue/SoundWave** (V1) → **MetaSounds** (V2 ambience) | Simple SFX need nothing fancy; MetaSounds when we want procedural hell ambience | MetaSounds-everywhere (over-engineered for 6 clips) |
| Data | **DataTables + Primary Data Assets** | Card face lookup, tunables — no runtime scans | Hardcoding, asset scans |
| Persistence | **`USaveGame`** (settings, onboarding flags) | Built-in, trivial | Config files, external db |
| Rendering (V2) | **Lumen + Nanite + Niagara** | The "graphics for less effort" payoff; no bakes/LODs/bespoke VFX | Baked lighting, hand LODs |
| Assets | **Fab** (Megascans + packs), **MetaHuman** (V2) | First-party marketplace + free digital humans | Custom modelling pipeline |
| Source control | **Git + Git LFS** (lockable `.uasset`/`.umap`) | Already on git; lightweight for a small team | Perforce / Anchorpoint (scale-up option) |
| Unit testing | **Automation Spec** (DubitoCore, headless) | Pure logic, fast, CI-able | — |
| Net testing | **PIE multiplayer** (daily) → **Gauntlet** (CI, optional) | Real host+clients in-editor; no 2nd machine | Manual-only |
| Build | **UAT `BuildCookRun`** → Steam depot (`steamcmd`) | Standard UE packaging + Steamworks ContentBuilder | — |

**Plugins to enable at bootstrap:** Online Subsystem Steam · Advanced Sessions + Advanced Steam Sessions · Common UI ·
Model View Viewmodel (MVVM) · Enhanced Input (default) · Functional Testing Editor · Fab. *(Everything else off — §11.)*

### Why this stack (short)
- **No dedicated-server cost** — one player is the **listen server** (host); Valve's SDR relay handles NAT traversal for
  free via OSS Steam.
- **Native replication over third-party netcode** — Unreal's Actor replication/RPCs/`OnRep` are first-party and map
  cleanly onto host-authoritative card logic; nothing to bolt on.
- **Unreal for graphics-per-effort** — Lumen/Nanite/MetaHuman/Fab make the V2 infernal tavern a *polish task, not an art
  commission*. V1 stays greybox (see `DESIGN.md` Part E) so mechanics land first.
- **C++ core, modern-UMG shell** — logic/networking in C++ for testability + replication correctness; CommonUI/MVVM only
  where they reduce work and improve UX.

### Steam AppID
| Phase | AppID | Reason |
|---|---|---|
| Dev & test | `480` (Spacewar) | Free Valve test app; lobbies/P2P/invites/auth work |
| Release | Real AppID | Steam Direct ($100, recouped at $1000 sales) |

`steam_appid.txt` (root) holds the number; OSS Steam is pointed at it in `Config/DefaultEngine.ini`
(`[OnlineSubsystemSteam] SteamDevAppId=480`). Switch `480` → real ID (drop the dev id) before submission.

---

## 2. Engine & modules

- **Target UE 5.8** — current stable and Epic's **last planned major UE5** version, so it's a long-lived base. **Fallback
  5.7** only if a required plugin (realistically Advanced Sessions) has no 5.8 branch at bootstrap; our C++ targets stable
  framework APIs, so 5.7↔5.8 is a version bump, not a rewrite. **Confirm the exact version in `ROADMAP.md` Phase 1.**

Two C++ modules keep the rules pure and testable (Unreal's expression of "logic out of MonoBehaviours"):

| Module | Depends on | Contains | Testable headless? |
|---|---|---|---|
| **`DubitoCore`** | `Core`, `CoreUObject` **only — never `Engine`** | `FCard`, `FDeck`, `FHand`, `FAnnouncement`, `FPlayAction`, `FGameStateData`, `FTurnValidator`, `FGameConstants` | **Yes** (Automation Spec) |
| **`Dubito`** | `Engine`, `DubitoCore`, `OnlineSubsystem*`, `UMG`, `CommonUI`, `ModelViewViewModel`, `EnhancedInput` | GameMode/GameState/Player*, session subsystem, pawns, actors, UI C++ bases | No |
| **`DubitoTests`** | `DubitoCore` + automation | Spec tests for the pure logic | Yes |

> **The one rule:** *game logic never lives in an Actor or Widget.* Actors/widgets read state and send intent (on the
> server); they never decide rules. `CoreUObject` is allowed in `DubitoCore` so networked value types (`FCard`,
> `FAnnouncement`) can be `USTRUCT`s and replicate; deck/validator/state logic stays plain and deterministic (injected RNG).

UI = **C++ CommonUI/`UUserWidget` bases** with **`WBP_`** subclasses for layout/skin — logic + bindings in C++, pixels in Blueprint.

---

## 3. Networking & online (the study)

### Transport / netcode: **native UE replication**
Unreal's Actor replication + RPCs map directly onto Dubito's host-authoritative model — there is **no Mirror/Photon
equivalent to add**. Details in §8.

### Online subsystem: **OSS Steam (v1) + Advanced Sessions**, and why not the others
| Option | What it is | Verdict |
|---|---|---|
| **OSS Steam (v1) + Advanced Sessions** ✅ | Classic Steam subsystem + a free, blueprint-exposed session layer (create/find/join, **Steam invites, overlay, friends, lobby code**) | **Chosen.** Best-documented path for a Steam-only listen-server P2P game; free; exactly our feature set. |
| **Online Services (OSSv2)** ⏭ | Epic's newer online API | **Deferred.** Future-proof but thinner docs today; no benefit for a single-platform V1. |
| **EOS** ⏭ | Free cross-platform sessions/identity | **Deferred.** Cross-platform we don't need; **sessions vs Steam server-lists differ → matchmaking isn't portable**, so switching later is a rewrite (only if we go cross-store). |
| **SteamCore PRO** 💲 | Paid, self-contained Steamworks wrapper (rich achievements/stats/leaderboards) | **Deferred/optional.** Cleanest paid route if we later want deep achievements/stats. Not needed to build the game. |
| **Photon / third-party netcode** ❌ | External relay/netcode | **Rejected.** Redundant — native replication + Steam SDR already give free NAT traversal. |

Achievements/stats (if wanted) come at release via OSS Steam or SteamCore.

---

## 4. UI framework (the study) — modern, simple, input-agnostic

The biggest "modern & easy" lever. Best-practice modern UE UI is a **hybrid**:
- **CommonUI** — the frame: activatable widget stacks (menus, modals, the doubt-reveal overlay), **input routing** (input
  goes only to the top layer → no focus bugs), native **gamepad + mouse/keyboard**. This is what Lyra/Fortnite use, and it
  buys **Steam Deck / controller** support essentially for free.
- **UMG Viewmodel (MVVM)** — data-binding for the **live HUD** (turn/claim/pile/counts/timer): the view updates *only when
  data changes* (FieldNotify), not by polling. MVVM is still beta, so use it **selectively** (data-heavy widgets), keeping
  menus/navigation on CommonUI — the recommended split, not "MVVM everywhere."
- **Enhanced Input** — modern input (Mapping Contexts + Input Actions); default in 5.x; the layer CommonUI expects.

**Why over plain UMG:** plain UMG means manual polling, brittle focus, per-platform input hacks. The hybrid is *less* code
over the project's life and is why the UI can be clean, responsive, and Deck-ready with no extra work. **References to
mirror (not import wholesale):** Epic's **Lyra** (CommonUI frontend) and the community **ModernUI_UE5** project. Full HUD/
interaction/motion spec is in `DESIGN.md` Part C.

---

## 5. Other subsystems

- **Audio:** V1 = the six SFX (`card_place`, `card_flip`, `doubt_call/correct/wrong`, `win`) as SoundWave/SoundCue (simple,
  easy variation). V2 = **MetaSounds** for dynamic infernal ambience (procedural fire, layered wails, reactive stingers).
- **Data:** a **DataTable/Primary Data Asset** maps `(ESuit,Value) → face Texture2D` (no runtime scans); seat names/colours
  data-driven; tunables in `FGameConstants` (expose a designer Data Asset only if we want no-recompile tweaks).
- **Persistence:** a small **`USaveGame`** for settings (volume, reduce-motion, (V2) reduce-look) + first-run onboarding flags.
- **Animation & presentation (separate from authoritative state):** animations *present* server-confirmed state — they are
  never the simulation (`DESIGN.md` §C.4). Triggered by `OnRep_`/`Multicast_`; on a newer authoritative state they
  **finish-fast or snap** (never fight it); the **server never waits** on client animation. Tech: **code-based C++ tweens**
  for procedural/variable UI (hand fan, per-card flights, staggered deal/reveal — trivial for N widgets and cheaper than
  Sequencer for short frequent anims); **UMG timeline** animations for fixed widget transitions; **C++ timelines/curves**
  for world actors (cards, pile, glow). A shared **token set** (durations/easings) keeps it one system. Full catalog: `DESIGN.md` §C.3.
- **Camera & social:** `ASeatPawn` **free-look** (clamped, recenter + idle auto-recenter); cinematic beats drive the camera
  via a timeline, then return control. Look is client-only, never replicated as game state. **Emotes (V2):** `Server_Emote(id)`
  → `Multicast_Emote`, **cosmetic-only**, rate-limited + per-player cooldown — never affects the simulation.
- **Rendering & assets (mostly V2):** **Lumen** (GI) + **Nanite** (geometry) + **Niagara** (fire/embers) — no bakes, no
  LODs, no bespoke VFX. **Fab** (Megascans + packs; Quixel Bridge deprecated → in-editor Fab plugin) for environment + the
  V1 card pack. **MetaHuman** for V2 characters. **V1 stays greybox** (`DESIGN.md` Part E).
- **Source control:** **Git + Git LFS**. `.uasset`/`.umap` are **binary & unmergeable**, so track them (+ textures/audio/
  models) in **`.gitattributes`** with the **`lockable`** flag and lock before editing; standard UE `.gitignore`
  (`Binaries/ Intermediate/ DerivedDataCache/ Saved/ .vs/`). Scale-up option (not now): Perforce Helix Core (free ≤5 users)
  or Anchorpoint.
- **Testing:** (1) **Automation Spec** unit tests for `DubitoCore`, headless via
  `UnrealEditor-Cmd ... -ExecCmds="Automation RunTests Dubito.Core; Quit"` (deterministic RNG → reproducible); (2)
  **PIE multiplayer** (Players > 1, *Play As Listen Server*) as the daily networked driver — real host + clients in-editor,
  no 2nd machine; (3) **Gauntlet** (optional CI) to orchestrate a packaged server + N clients + functional tests; (4) the
  real 2-machine **Steam** pass (Phase 6).
- **Build & release:** UAT `BuildCookRun` (Development for testing, **Shipping** for release, Win64); Steam **ContentBuilder**/
  `steamcmd` depot upload; AppID 480 → real AppID at release.

---

## 6. Game State Machine

Every transition is authoritative on the **host** (listen server). Clients send inputs; the server validates and replicates.

```
  Lobby → Dealing → PlayerTurn ⇄ {Play→TurnEnd | Doubt→DoubtReveal | Discard→PileDiscarded} → … → GameOver
```
```
  Lobby
    │ (host starts, ≥2 players, all ready)
    ▼
  Dealing (private hands, order randomized)
    ▼
  PlayerTurn ─┬─[Play]────► TurnEnd ─(hand empty?)─► next player's PotentialWin window
              │                                        ├─[Doubt] → DoubtReveal → win stands / continues
              │                                        └─[Play]  → GameOver ✓ (win confirmed; Discard blocked)
              ├─[Doubt]───► DoubtReveal ─┬─[right] → PenaltyApplied → doubter plays on
              │                          └─[wrong] → PenaltyApplied → turn skipped
              └─[Discard]─► PileDiscarded → turn skipped (round value resets)
```
`EGamePhase` (in `DubitoCore`): `Lobby`, `Dealing`, `PlayerTurn`, `DoubtReveal`, `GameOver`. Intermediate boxes
(`TurnEnd`, `PenaltyApplied`, `PileDiscarded`) are **server-side resolution steps**, not replicated phases — they collapse
into the next `PlayerTurn`/`GameOver` broadcast. Full rule semantics: `DESIGN.md` Part A; UX per phase: `DESIGN.md` §B.4/§D.1.

---

## 7. Data Model

Pure C++ in `DubitoCore` (no `Engine`). Networked value types are `USTRUCT`s; logic types are plain and deterministic.

```cpp
UENUM() enum class ESuit : uint8 { Clubs, Diamonds, Hearts, Spades };

USTRUCT() struct FCard {
    GENERATED_BODY()
    UPROPERTY() ESuit Suit;
    UPROPERTY() uint8 Value;   // 1..13 (Ace..King), validated; NetSerialize = 2 bytes; operator==
};

class FDeck { TArray<FCard> Cards; public:
    void Shuffle(FRandomStream& Rng);      // injected RNG → reproducible tests
    TArray<FCard> Deal(int32 Count); };

class FHand { public:
    int32 OwnerId; TArray<FCard> Cards;
    void AddCards(const TArray<FCard>&);
    bool RemoveCards(const TArray<FCard>&); // all-or-nothing
    bool IsEmpty() const; };

USTRUCT() struct FAnnouncement {
    GENERATED_BODY()
    UPROPERTY() uint8 ClaimedValue;   // = the locked RoundValue (1..13)
    UPROPERTY() uint8 ClaimedCount;   // 1..4, the free lie
};

struct FPlayAction { int32 PlayerId; TArray<FCard> ActualCards /*SERVER-ONLY until reveal*/; FAnnouncement Announcement; };

struct FGameStateData {
    EGamePhase Phase;
    TArray<int32> TurnOrder; int32 ActivePlayerIndex;
    uint8 RoundValue;                 // NoRoundValue when pile empty / round closed
    TArray<FCard> CenterPile;         // contents SERVER-ONLY; only the COUNT is public
    TOptional<FPlayAction> LastPlay;  // unset at start / after a reset
    TMap<int32, FHand> Hands;         // private per player
    bool bPotentialWinPending; int32 PotentialWinnerId; // valid iff pending; else NoPlayerId
};
```

**`FTurnValidator`** (static, pure) — the single source of rule truth:
- `IsValidPlay(hand, cards, announcement, roundValue)` — 1–4 cards, count 1–4, value == round value (or opening).
- `IsClaimedValueAllowed(claimedValue, roundValue)` — enforces the locked-value rule.
- `IsDoubtCorrect(lastPlay, roundValue)` — **correct iff** `actualCount != claimedCount` **OR** any actual card's value !=
  the round value. Both checked (matches `DESIGN.md` Part A).

### Privacy model (critical for bluffing)
- A hand is **never** in the replicated `ADubitoGameState`; it is delivered to its owner only (see §8).
- Pile **count** is public (in GameState); pile **contents** are server-only until a reveal.
- No per-turn marker in the pile — identical cards; players track mentally.
- `LastPlay.ActualCards` is **server-only** until `DoubtReveal` broadcasts it.

---

## 8. Network Architecture (Unreal-native)

### Host model
- **Listen server** = one player hosts (`GetNetMode()==NM_ListenServer`, `HasAuthority()` gates all logic); no separate
  machine. Valve SDR relay (via OSS Steam) handles NAT traversal.
- **Steam Session** = up to 8, created/found/joined through OSS Steam (Advanced Sessions wraps it); joinable by Steam
  invite/overlay or a short lobby code.
- **Responsiveness model:** authoritative, **no client prediction/rollback** — a card game tolerates the round-trip. The
  client shows a light **pending** state on the initiating control and applies the result on server confirm (`DESIGN.md` §C.4).

### Framework component map
| UE class | Dubito class | Lives on | Responsibility |
|---|---|---|---|
| `AGameModeBase` | `ADubitoGameMode` | **Server only** | Owns `FGameStateData`; validates + applies every action via `DubitoCore`; deals; runs the turn timer; resolves doubts/wins. The single authority. |
| `AGameStateBase` | `ADubitoGameState` | Replicated to all | **Public** state: phase, active id, pile **count**, `RoundValue`, turn order, current public claim, pending-win flags, `TurnEndsServerTime`. `OnRep_*` → MVVM viewmodels → HUD. |
| `APlayerState` | `ADubitoPlayerState` | One per player, replicated | **Public** per-player: `PlayerId`, `DisplayName`, `SeatIndex`, `CardCount`, `bReady`. |
| `APlayerController` | `ADubitoPlayerController` | Owning client + server | Sends **Server RPCs** (input); receives **Client RPCs**; holds the **owner-only private hand**. |
| `APawn` | `ASeatPawn` | Per seat | **Free-look** first-person camera rig at the local seat (clamped yaw/pitch, recenter + idle auto-recenter); cinematic beats take/return control; screen-space HUD stays put. Look is client-only, never game state. |
| `UGameInstanceSubsystem` | `UDubitoSessionSubsystem` | Client & host | OSS Steam session lifecycle (create/find/join/destroy, invites, code); transport-agnostic menu API. |

### RPC & replication map
| Intent | Unreal mechanism |
|---|---|
| Client → Host input | **`Server_` RPC** on `ADubitoPlayerController` (Reliable, WithValidation): `Server_PlayCards(TArray<FCard>, FAnnouncement)`, `Server_Doubt()`, `Server_DiscardPile()`, `Server_SetReady(bool)`. Sender = the RPC's owning connection. |
| Host → All public state | **Replicated `ADubitoGameState` properties + `OnRep_`** (auto delta-replication). |
| Host → All events | **`NetMulticast_` RPC**: `Multicast_RevealCards(FRevealInfo)`, `Multicast_GameOver(FGameOverInfo)`, `Multicast_AutoPlayed(...)`. |
| Host → **one** (private hand) | **Owner-only replication:** the hand is `UPROPERTY(ReplicatedUsing=OnRep_Hand)` on `ADubitoPlayerController` with `DOREPLIFETIME_CONDITION(..., COND_OwnerOnly)`. Unreal ships it **only to the owning client** and **auto-resyncs on change** — no manual re-send. (A `Client_` RPC is the alternative for an event vs state.) |

> **Owner-only replication for hands** is an improvement over hand-rolled targeted sends: the engine keeps the hand in sync
> privately and automatically (correct by construction). The bluff (actual played cards) never enters a replicated property
> until a reveal.

### Authority rules
| Action | Initiator | Validator | Result visibility |
|---|---|---|---|
| Play cards | Client (`Server_PlayCards`) | Server (`FTurnValidator`) | Pile **count** + public claim → all; actual cards stay server-only |
| Call doubt | Client (`Server_Doubt`) | Server | `Multicast_RevealCards` → all; transfer replicates |
| Discard pile | Client (`Server_DiscardPile`) | Server | Pile count → 0, round value reset → all |
| Deal / update hand | Server | Server | **Owner-only** — never to others |
| Ready / start | Client / Host | Server | `bReady` replicates; host-only `StartMatch` |

### Turn timer (server-authoritative)
GameMode owns an `FTimerHandle` per turn (`TurnSeconds = 45`) and replicates the **deadline** as
`ADubitoGameState.TurnEndsServerTime` (`GetServerWorldTimeSeconds`), so clients render the same countdown without owning
it. On expiry → `ServerAutoResolveTimeout` (decline doubt → auto-play one truthful card; pending-win → confirm win). A
voluntary action resets the streak; **3 consecutive timeouts** → treated as a disconnect. Rule spec: `DESIGN.md` Part A;
edge cases: `DESIGN.md` §D.4.

### Sessions, travel & lifecycle
`UDubitoSessionSubsystem` (on the `UGameInstance`, survives travel) wraps OSS Steam (Host/Find/Join/Destroy, invite, code)
+ exposes connecting/error states. Scene flow uses **`ServerTravel`**: MainMenu → WaitingRoom → Table → PostGame → back.
V1 is built **local-first** on the null/LAN subsystem (host + join-by-IP `127.0.0.1`), fully testable by launching a
packaged build twice on one PC; the OSS Steam path (invite + code) is swapped in for the 2-machine test (`ROADMAP.md`
Phases 5–6). The menu/session API is transport-agnostic, so that's a config swap, not a rewrite.

---

## 9. Project / Folder Structure

```
DubitoUE/
├── Dubito.uproject
├── steam_appid.txt                # "480" in dev
├── Config/                        # DefaultEngine.ini (OSS Steam, maps), DefaultInput.ini (Enhanced Input)
├── Documentation/                 # DESIGN.md, ARCHITECTURE.md, ROADMAP.md  (moved here at bootstrap)
├── Source/
│   ├── DubitoCore/                # PURE logic — Core + CoreUObject only, NO Engine
│   │   ├── DubitoCore.Build.cs
│   │   ├── GameConstants.h · Card.h/.cpp · Deck.h/.cpp · Hand.h/.cpp
│   │   ├── Announcement.h · PlayAction.h · GameStateData.h/.cpp · GameOverReason.h · TurnValidator.h/.cpp
│   ├── Dubito/                     # Game module
│   │   ├── Dubito.Build.cs         # deps: Engine, DubitoCore, OnlineSubsystem, OnlineSubsystemSteam,
│   │   │                           #       UMG, CommonUI, ModelViewViewModel, EnhancedInput
│   │   ├── DubitoGameMode.h/.cpp · DubitoGameState.h/.cpp · DubitoPlayerState.h/.cpp
│   │   ├── DubitoPlayerController.h/.cpp · DubitoGameInstance.h/.cpp
│   │   ├── Session/DubitoSessionSubsystem.h/.cpp
│   │   ├── Pawns/SeatPawn.h/.cpp
│   │   ├── Actors/CardActor.h/.cpp · Actors/PileActor.h/.cpp · Actors/SeatActor.h/.cpp
│   │   └── UI/                      # CommonUI/UUserWidget bases: HUDRoot, ActionBar, AnnouncePanel,
│   │                               #   DoubtRevealPanel, ConfirmPanel, WaitingRoom, PostGame, HelpCard
│   └── DubitoTests/                # Automation Spec tests for DubitoCore
├── Content/
│   ├── Maps/                       # MainMenu, WaitingRoom, Table (.umap)
│   ├── UI/                         # WBP_ widgets (subclass the C++ bases)
│   ├── Blueprints/                 # BP_ subclasses of C++ actors (data/cosmetics only)
│   ├── Cards/                      # card mesh + materials + CardDataTable
│   ├── Characters/                 # V1 placeholder; V2 MetaHumans
│   └── Environment/                # V1 greybox; V2 Fab/Megascans
└── Plugins/
    ├── AdvancedSessions/  └── AdvancedSteamSessions/
```

`CLAUDE.md` ships at the project root at bootstrap. During conception these docs live in `docs-unreal/`.

---

## 10. Constants (`FGameConstants`)

| Constant | Value | | Constant | Value |
|---|---|---|---|---|
| `MinPlayers` | 2 | | `MaxCardValue` | 13 |
| `MaxPlayers` | 8 | | `TurnSeconds` | 45 |
| `DeckSize` | 52 | | `MaxConsecutiveTimeouts` | 3 |
| `MinCardsPerPlay` | 1 | | `NoRoundValue` | 0 (pile empty / round closed) |
| `MaxCardsPerPlay` | 4 | | `NoPlayerId` | -1 |
| `MinCardValue` | 1 | | | |

> `TurnSeconds` / `MaxConsecutiveTimeouts` back the turn timer + anti-AFK rule (`DESIGN.md` Part A). Host-authoritative;
> clients render the remaining time from the replicated deadline only. These values **must** match `DESIGN.md`.

## 11. Deliberately NOT using (restraint = simplicity)
| Not using | Why |
|---|---|
| **Gameplay Ability System (GAS)** | Built for abilities/attributes/cooldowns; a turn-based card game has none. Huge complexity, zero benefit. |
| **The full Lyra framework** (experiences, GameFeatures) | We take Lyra's *UI patterns* (CommonUI) only; the rest is AAA-shooter scaffolding. |
| **EOS / Online Services (OSSv2)** now | No cross-platform need for a Steam-only V1; adopting later is a contained swap (§3). |
| **Photon / third-party netcode** | Redundant with native replication + Steam SDR. |
| **Dedicated servers** | Listen-server (a player hosts) is the design; free NAT traversal via Steam. |
| **MVVM everywhere** | Beta; HUD data only, CommonUI for navigation (the recommended split). |
| **Gauntlet as a hard requirement** | PIE multiplayer covers daily testing; Gauntlet is opt-in CI. |
| **Client-side prediction / rollback** | A card game tolerates the round-trip; pending states are simpler and glitch-free. |

## 12. Out of Scope (V1)
Spectator mode · voice chat (players use Discord) · AI bots · rejoin after disconnect · **host migration** (host leaving
ends the session) · the Guillotine (→ V2, `DESIGN.md` Part A) · mobile / console ports.

---

## Sources (technology study)
UE 5.7/5.8 official docs (networking, Online Subsystem Steam, CommonUI input/overview, UMG Viewmodel, Gauntlet) · OSS Steam
vs EOS (maystocks.net; Epic forums) · Advanced Sessions (Epic community; Fab) · SteamCore PRO (Fab/eelDev) · Version control
for UE5 — Git LFS vs Perforce (StraySpark, Anchorpoint) · MetaSounds vs SoundCue (community wiki) · Unreal test automation
2025 (Andrew Fray) · ModernUI_UE5 reference (github kosowskie).
