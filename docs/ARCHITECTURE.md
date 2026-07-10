# Dubito - V1 Architecture

This file defines the V1 technical contract. It avoids code snippets and focuses on decisions, responsibilities, and
validation criteria.

Official-source notes live in `docs/reference/UE_STEAM_SOURCES.md`. Game design lives in `docs/DESIGN.md`.

## V1 Stack

| Area | V1 decision | Reason |
|---|---|---|
| Engine | Unreal Engine 5.8 target; fallback 5.7 only if bootstrap blocks | Current UE5 baseline, avoids waiting for UE6 |
| Rules | Pure C++ module, unit-tested | Rules stay deterministic and independent from actors/widgets |
| Network | Native UE replication | First-party, enough for a turn-based card game |
| Host model | Listen server | No server cost for V1 |
| Online | Online Subsystem Steam through C++ session APIs | Official Steam path, no third-party dependency |
| UI | CommonUI + UMG | Stable enough, supports layered menus and controller navigation |
| Input | Enhanced Input for gameplay; CommonUI actions for UI | Clear separation between gameplay and UI input |
| HUD updates | Event-driven UMG from replicated state | Avoids MVVM beta dependency in V1 |
| Data | Data Assets / Data Tables for card faces and tunables | Simple, editor-friendly |
| Persistence | SaveGame for settings and onboarding flags | Built-in |
| Audio | SoundWave/SoundCue | Enough for V1 feedback |
| Source control | Git + Git LFS before assets | Unreal binary assets need LFS/locking discipline |

## Key V1 Decisions

This section is the compact ADR-equivalent for V1. Do not create separate ADR files unless the documentation strategy
changes.

| Decision | V1 ruling |
|---|---|
| First-party baseline | Use built-in Unreal systems first; no optional plugin during V1 bootstrap. |
| CommonUI accepted | Use built-in CommonUI/UMG for menus, modals, HUD shells, and controller-friendly UI. |
| No MVVM dependency | Epic labels UMG Viewmodel as beta; V1 uses event-driven UMG updates. |
| No Advanced Sessions baseline | Use official Online Subsystem Steam C++ session APIs first. |
| External assets | No external asset dependency in the V1 baseline; owner selects and imports assets only when a phase calls for them. |
| Listen server | No dedicated server cost or infrastructure for V1. |
| Greybox first | Fab/Megascans, MetaHuman, Lumen polish, Nanite, Niagara, emotes, and Guillotine are V2. |

Optional plugins, tools, templates, or asset packs require explicit owner approval and a documented blocker or later art
phase need.

## Modules

| Module | Responsibility | Dependency rule |
|---|---|---|
| `DubitoCore` | cards, deck, hands, claims, state transitions, rule validation, constants | no `Engine` dependency |
| `Dubito` | Unreal gameplay framework, networking, UI shells, sessions, pawns, actors | may depend on `DubitoCore` and UE runtime modules |
| `DubitoTests` | Automation tests for `DubitoCore` contracts | test-only |

`DubitoCore` may use Unreal core value types if needed for reflection/replication contracts, but rule logic must remain
plain, deterministic, and testable without a level.

## Required V1 Unreal Systems

Enable or configure during Phase 1:

- Online Subsystem Steam;
- Online Subsystem Utils;
- Common UI;
- Enhanced Input;
- UMG;
- Functional Testing / Automation support;
- Git LFS and Unreal binary asset tracking before `.uasset` or `.umap` files land.

Explicitly not required for V1:

- Model View Viewmodel;
- Advanced Sessions / Advanced Steam Sessions;
- Gameplay Ability System;
- Replication Graph or Iris-specific optimization;
- dedicated server target;
- SteamCore PRO;
- Fab plugin;
- external asset packs as a V1 dependency.

## UI Architecture

V1 UI is deliberately simple.

| Surface | Approach |
|---|---|
| Main menu, waiting room, post-game, modals | CommonUI activatable widgets |
| In-game HUD | UMG widgets refreshed by replicated state events |
| Action bar | UMG/CommonUI-compatible buttons with focus/confirm support |
| Input glyphs | CommonUI input data where practical |
| Gameplay controls | Enhanced Input actions |

Do not depend on the experimental CommonUI + Enhanced Input integration for V1. If it is stable in the installed engine,
it can be enabled later, but the design must work without it.

## Gameplay Framework Map

| Unreal role | Dubito responsibility |
|---|---|
| GameMode | server-only authority; owns actual game state, deals, validates, resolves turns |
| GameState | replicated public state: phase, active player, round value, public claim, claimed pile ledger, timer deadline |
| PlayerState | replicated public identity, seat, readiness, public hand ledger |
| PlayerController | owning client input, server RPCs, owner-only private hand state |
| Pawn | seated camera and local free-look only |
| GameInstance subsystem | session lifecycle and travel entry points |

Actors and widgets render state and send intent. They do not decide rules.

## Data Contracts

| Contract | Meaning |
|---|---|
| Card | suit and value |
| Hand | private card list owned by a player |
| Announcement | claimed value and claimed count |
| Play action | player, actual cards, announcement |
| Game state data | phase, turn order, round value, pile, last play, hands, public ledgers, pending win |
| Reveal info | claim, actual previous play, verdict, loser, transfer summary |
| Game over info | winner and reason |

Rule validator responsibilities:

- validate actual play size and claimed count independently;
- enforce locked claimed value when a round is open;
- allow actual cards/count to differ from the claim;
- resolve Doubt by comparing actual count and values to the claim;
- never read UI state.

## Hidden Information Contract

Server-only before reveal:

- actual hands for all players;
- actual cards just played;
- actual count just played;
- actual pile contents and size;
- full hidden game state.

Replicated to everyone:

- phase;
- active player;
- round value;
- previous public claim;
- claimed pile ledger;
- public hand ledgers;
- pending-win flag;
- timer deadline.

Replicated only to owner:

- exact private hand.

Important: `PublicClaimedPileCount` and public hand ledgers are display/stake ledgers. They are not authoritative proof of
actual hidden counts.

## RPC And Replication Rules

- Client actions use server RPCs on the owning PlayerController.
- Core actions are Play, Doubt, Discard, Ready, and Start Match.
- Use reliable RPCs for these low-frequency turn actions.
- RPC validation failure should be reserved for malformed or abusive payloads that should disconnect a caller.
- Ordinary illegal gameplay, stale turns, and expired windows must produce controlled rejection and state resync.
- Use replicated properties for persistent public state.
- Use multicast only for self-contained public events such as reveal and game over.
- Use owner-only replication for exact private hand state.

## Sessions And Steam

V1 uses Online Subsystem Steam directly through Unreal's C++ session interfaces.

Steam/lobby contract:

- AppID 480 for development.
- Real AppID for release.
- Use Steam lobbies for listen-server matchmaking when available.
- Use presence/lobby session settings for host/join through Steam.
- Use `bInitServerOnClient` when using Steam sessions.
- Keep `steam_appid.txt` for local development only and do not ship it in Steam depots.

Build order:

1. local null/LAN flow first;
2. packaged two-instance test on one PC;
3. Steam two-machine test with two Steam accounts;
4. release AppID and depot setup.

## Travel

Use server-driven travel for multiplayer flow:

- Main Menu;
- Waiting Room;
- Table;
- Post Game;
- back to Waiting Room or Main Menu.

Prefer seamless travel after initial connection. Non-seamless travel is acceptable for first connect, leaving a session, or
resetting to a new main-menu context.

## State Machine

Replicated phases:

- Lobby;
- Dealing;
- Player Turn;
- Doubt Reveal;
- Game Over.

Server-only resolution steps should not become public phases unless the UI needs them. Keep public state small.

## Constants

| Constant | V1 value |
|---|---|
| Min players | 2 |
| Max players | 8 |
| Deck size | 52 |
| Min cards per play | 1 |
| Max cards per play | 4 |
| Card values | 1 to 13 |
| Turn seconds | 45 |
| Max consecutive timeouts | 3 |
| No round value | 0 |
| No player id | -1 |

Constants must match `docs/DESIGN.md`.

## Project Layout Contract

| Path | Purpose |
|---|---|
| `DubitoUE/` | Unreal project root after Phase 1 |
| `DubitoUE/Documentation/` | copy of this `docs/` folder after bootstrap |
| `DubitoUE/Source/DubitoCore/` | pure rules |
| `DubitoUE/Source/Dubito/` | Unreal gameplay, network, UI shells |
| `DubitoUE/Source/DubitoTests/` | automation tests |
| `DubitoUE/Content/Maps/` | MainMenu, WaitingRoom, Table, PostGame |
| `DubitoUE/Content/UI/` | widgets and styles |
| `DubitoUE/Content/Cards/` | simple card meshes/materials/textures |
| `DubitoUE/Content/Art/Prototype/` | owner-approved temporary visual assets, only if greybox primitives are not enough |
| `DubitoUE/Content/Art/Marketing/` | optional project copy of approved release marketing assets |
| `DubitoUE/Content/Audio/SFX/` | owner-approved V1 action and UI sounds, only after gameplay loop works |

During conception, keep all documentation under the repository `docs/` folder. During Unreal bootstrap, copy this folder
into `DubitoUE/Documentation/` so the Unreal project can be opened as a standalone workspace.

## Asset Intake Policy

V1 starts with Unreal primitives, simple materials, and internally created placeholders. External assets are not selected
by default during conception or bootstrap.

When a phase needs assets, the agent must tell the owner:

- what asset category is needed;
- whether it is temporary V1 placeholder, V1 shipping candidate, or V2/final art;
- what license or ownership constraint must be checked;
- where the asset should be imported in the Unreal project;
- whether Git LFS/locking must be confirmed first.

Likely asset asks:

| Phase | Need | Human action | Target folder |
|---|---|---|---|
| 3 | readable card faces, backs, and simple card materials | approve internally created placeholders or import an owned/free card set | `DubitoUE/Content/Cards/` |
| 3 | table, seats, and neutral player placeholders | use Unreal primitives first; import external props only if readability blocks | `DubitoUE/Content/Art/Prototype/` |
| 7 | place, flip, doubt, win, menu confirm/cancel sounds | choose or approve owned/free audio once the full loop works | `DubitoUE/Content/Audio/SFX/` |
| 8 | store capsule, screenshots, and release marketing art | owner provides or approves final release assets | Steamworks and `DubitoUE/Content/Art/Marketing/` if a project copy is needed |

## Testing Strategy

| Layer | Validation |
|---|---|
| Core rules | Automation tests for deck, hand, validator, turn state, public ledgers |
| Network | PIE multiplayer listen-server tests |
| Packaged local | two instances on one PC |
| Steam | two machines, two Steam accounts |
| UX | screenshot/viewport checks for action matrix and hidden-count presentation |

## Deliberately Not Using In V1

- dedicated servers;
- host migration;
- rejoin into active match;
- GAS;
- MVVM;
- Advanced Sessions;
- SteamCore PRO;
- Replication Graph optimization;
- final asset pipeline;
- complex economy, progression, achievements, or cosmetics.

## Sources

Official source notes checked July 11, 2026 are summarized in `docs/reference/UE_STEAM_SOURCES.md`.
