# Dubito

Multiplayer online bluffing card game for Steam (2–8 players). Damned souls sit around a table in an infernal tavern,
take turns placing cards face down and bluffing about them. First to empty their hand wins.

Reference: Liar's Bar (Steam, 2024) — same social tension, different mechanics and visual identity.

> **The docs are just three, by concern — read the one that matches what you're doing:**
> - Game design (rules · UX · interactions · edge cases · art): **@Documentation/DESIGN.md**
> - Architecture & technology (stack · plugins · networking · data model · folders · constants): **@Documentation/ARCHITECTURE.md**
> - Roadmap (phased execution plan): **@Documentation/ROADMAP.md**
>
> **Note:** during conception these four files live in `docs-unreal/`. At Phase 1 bootstrap, `DESIGN.md` /
> `ARCHITECTURE.md` / `ROADMAP.md` move to `DubitoUE/Documentation/` and this `CLAUDE.md` becomes the project-root file.

---

## Commands

```bash
# Build (adjust the engine path to the local UE 5.8 install)
"<UE>/Engine/Build/BatchFiles/Build.bat" DubitoEditor Win64 Development -Project="<path>/DubitoUE.uproject"

# Run DubitoCore unit tests headless (Editor must be closed)
UnrealEditor-Cmd.exe DubitoUE.uproject -ExecCmds="Automation RunTests Dubito.Core; Quit" -unattended -nop4 -nosplash -log

# Package (Development, Win64)
"<UE>/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun -project="<path>/DubitoUE.uproject" ^
  -noP4 -platform=Win64 -clientconfig=Development -cook -build -stage -pak -archive -archivedirectory="<out>"
```

> Networking is tested in-editor via **PIE multiplayer** (Number of Players > 1, Net Mode = *Play As Listen Server*).
> Only the real Steam transport (Phase 6) needs two machines + two Steam accounts.

---

## Stack (at a glance — full study in ARCHITECTURE.md)

| Layer | Technology |
|---|---|
| Engine | Unreal Engine **5.8** (latest UE5 stable; last major UE5 — long-lived) |
| Gameplay code | C++ (logic/networking/replication) |
| UI | CommonUI + UMG Viewmodel (MVVM) + Enhanced Input — input-agnostic (KBM/gamepad/Steam Deck) |
| Networking | Native UE replication, listen-server, host-authoritative — **no client prediction** |
| Online / Steam | Online Subsystem Steam (v1) + Advanced Sessions — AppID 480 (dev) → real AppID |
| Audio | SoundCue/SoundWave (V1) → MetaSounds (V2) |
| Source control | Git + Git LFS (lockable `.uasset`/`.umap`) |
| 3D chars / env (V2) | MetaHuman · Fab (Megascans) · Lumen / Nanite / Niagara |

> Full architecture, networking, plugin study, data model, folders, constants: **@Documentation/ARCHITECTURE.md**

## Game rules

The authoritative rules are **@Documentation/DESIGN.md** Part A. Announcement rule = **round value is locked** (opener
chooses, others must match); only the **count** is a free lie. 2–8 players, 52 cards, 1–4 per play, 45 s turn timer.

---

## Project Structure

```
DubitoUE/
├── CLAUDE.md
├── Dubito.uproject
├── steam_appid.txt              # "480" in dev
├── Config/                      # DefaultEngine.ini (OSS Steam, maps), DefaultInput.ini
├── Documentation/               # DESIGN.md · ARCHITECTURE.md · ROADMAP.md
├── Source/
│   ├── DubitoCore/              # PURE logic — Core + CoreUObject only, NO Engine. Unit-tested.
│   ├── Dubito/                  # Game: GameMode/GameState/Player*, session, pawns, actors, UI (C++ bases)
│   └── DubitoTests/             # Automation Spec tests for DubitoCore
├── Content/                     # Maps · UI (WBP_) · Blueprints (BP_) · Cards · Characters · Environment
└── Plugins/                     # AdvancedSessions · AdvancedSteamSessions
```

---

## Conventions

- All code, comments, files, and commits in **English**.
- **Unreal C++ naming:** type prefixes `A`/`U`/`F`/`E`/`I`; PascalCase types/functions/members, camelCase locals, `b`
  prefix for bools. One primary class per file; filename matches the class (minus prefix).
- **No magic numbers** — every gameplay constant lives in `FGameConstants` (`DubitoCore`); values must match `DESIGN.md`.
- **Game logic lives in `DubitoCore`, never in an Actor or Widget.** Actors/widgets read state and send intent; they never
  decide rules. `DubitoCore` depends on `Core`/`CoreUObject` only — **never `Engine`** (keeps it unit-testable).
- **Host is authoritative.** All rule decisions run on the server (`ADubitoGameMode`, gated by `HasAuthority()`). Clients
  send inputs and render replicated state — never validate or mutate authoritative state locally, and **no client
  prediction** (pending states only; a card game tolerates the round-trip).
- **All networked actions go through the framework:** client→host via `Server_` RPCs (Reliable, WithValidation); host→all
  via replicated `ADubitoGameState` + `OnRep_` (or `NetMulticast_` for one-shot events); host→one via **owner-only
  replication** (`COND_OwnerOnly`).
- **The bluff stays hidden.** Hands and pile contents never enter a property that replicates to non-owners. Hands are
  **owner-only**; pile *count* is public, pile *cards* reveal only on a doubt.
- **UI stack:** CommonUI (navigation, modals, input routing) + MVVM Viewmodel (HUD data-binding — data-heavy widgets only)
  + Enhanced Input. Keep it **input-agnostic** (mouse/keyboard/gamepad/Steam Deck); no mouse-only interactions. HUD is
  **hybrid** (in-world context + a minimal pinned overlay); motion is **sober + one cinematic doubt reveal** with a
  reduce-motion baseline. Full spec: `DESIGN.md` Part C; rationale: `ARCHITECTURE.md` §4.
- **Animations = presentation, not simulation.** They play **server-confirmed** state and **finish-fast/snap** on a newer
  state; the host **never waits** on a client animation. Procedural card UI = code-based C++ tweens; fixed widget anims =
  UMG timelines; one shared duration/easing token set. **Camera = free-look** (client-only, never game state). **Emotes =
  V2, cosmetic-only** (rate-limited). Full spec: `DESIGN.md` §C.3–C.4, Part E.
- **Simplicity first:** the game must stay trivial to learn (benchmarks: Liar's Bar / Balatro). Prefer built-in/first-party
  tech; don't add plugins, systems, or rules that don't earn their complexity (see `ARCHITECTURE.md` §11 "NOT using").

---

## What NOT to Do

- Do not put game logic inside Actors/Pawns/Widgets — keep it in pure `DubitoCore` C++.
- Do not let `DubitoCore` depend on the `Engine` module.
- Do not mutate authoritative game state on a client, and do not add client-side prediction — the host validates everything.
- Do not replicate hand contents (or pile card contents) to all clients — hands are **owner-only**; pile cards reveal only on a doubt.
- Do not hardcode player counts, card values, timer durations, or any gameplay constant inline — use `FGameConstants`.
- Do not add a new level without registering it in `DefaultEngine.ini` / the travel flow.
- Do not add mouse-only UI — every action must be reachable by focus + confirm (gamepad / Steam Deck).
- Do not change the announcement rule (locked value) or a constant without updating `DESIGN.md` **and** `ARCHITECTURE.md` **and** `FTurnValidator`/`FGameConstants` together.

---

## Self-Update Protocol

At the end of every session, before stopping:
1. Update `## Current Phase` below (mark done, set next).
2. Update `Documentation/DESIGN.md` if any rule, UX, interaction, edge case, or art decision changed.
3. Update `Documentation/ARCHITECTURE.md` if any tech/plugin/networking/structure/constant decision changed.
4. Update `Documentation/ROADMAP.md` phase statuses and task boxes.
5. Add any new pattern or convention discovered during the session to `## Conventions`.
6. Keep the four files **consistent** — a fact stated in two files must not contradict. Commit the updates with a clear message.

This file is the single source of truth for this project. Keep it accurate.

---

## Current Phase

**Phase 0 — Conception ✅ complete** (Unreal re-conception; docs in `docs-unreal/`)
**Phase 1 — Unreal Bootstrap ⏳ next** → see `Documentation/ROADMAP.md` for exact steps

Full phase details and validation criteria: **@Documentation/ROADMAP.md**
