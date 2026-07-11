# Dubito Agent Guide

This file is mirrored at the repository root and under `Documentation/`. Keep all copies identical.

Dubito is a Steam-focused multiplayer bluffing card game for 2 to 8 players. V1 must prove the complete playable loop with
a small, first-party Unreal stack before visual or systems expansion.

All conception documentation lives under `Documentation/`.

## Read Routing

Read only what matches the task.

| Task | Read |
|---|---|
| Any agent session | `AGENT.md`, `CLAUDE.md`, `Documentation/AGENT.md`, or `Documentation/CLAUDE.md` |
| Rules, UX, V1 scope, art direction | `Documentation/DESIGN.md` |
| Edge cases or implementation validation | `Documentation/v1/EDGE_CASES.md` |
| Unreal stack, network, data contracts | `Documentation/ARCHITECTURE.md` |
| Official UE/Steam constraints | `Documentation/reference/UE_STEAM_SOURCES.md` |
| Execution order or phase status | `Documentation/ROADMAP.md` |

Do not read every file by default. Start with the routed owner file, then open detail files only when the task needs them.
When executing roadmap work, pick the exact sub-phase ID from `Documentation/ROADMAP.md`; do not treat a full phase as a single agent task.

## Current Phase

Phase 0 conception documentation, Phase 1 Unreal bootstrap, Phase 2 core rules, and Phase 3 greybox table are complete.
Phase 4 Network Framework is complete. Phase 5.0 table HUD state binding is complete.
Current implementation work continues at Phase 5.1 Play interaction, then advances through the remaining Phase 5 sub-phases in roadmap order unless the owner explicitly changes priorities.

No gameplay code should be written during Phase 0. Documentation changes are allowed.

## Absolute V1 Constraints

- Windows PC through Steam.
- 2 to 8 players.
- Listen server, host-authoritative, no dedicated servers.
- Unreal native replication plus Online Subsystem Steam.
- CommonUI/UMG for menus and HUD; Enhanced Input for gameplay controls.
- CommonUI is accepted because it is a built-in Unreal plugin.
- No third-party runtime plugin or external asset dependency in the V1 baseline.
- No MVVM dependency in V1.
- Greybox/readable tavern and simple cards before final art.
- V2 only: MetaHuman, Fab/Megascans, Lumen polish, Nanite, Niagara, emotes, Guillotine.

## Rule Invariants

`Documentation/DESIGN.md` owns the rules.

- Round value is opened on an empty pile, then locked until the pile is emptied.
- Players secretly play 1 to 4 actual cards.
- Claimed count is a bluffable value from 1 to 4 and may differ from actual count.
- Only the immediate next player may Doubt, before taking another action.
- Doubt is correct if actual count differs from claimed count, or if any actual card is not the round value.
- Discard is legal only for the active player, only on a non-empty pile, and never while a win is pending.
- Timeout auto-plays one card; truthful if possible, forced minimal bluff only when truth is impossible.
- Timeout or disconnect during a pending-win response confirms the pending winner.

## Hidden Information Invariants

Never reveal these to non-owners before a valid reveal or transfer:

- hand contents;
- actual number of cards just played;
- exact opponent hand delta;
- actual pile size;
- pile contents.

Public counts are ledgers, not proof. Own hand is exact. Pending-win is driven by actual server state.

## Architecture Invariants

- Keep rules in `DubitoCore`, not Actors, Pawns, Widgets, or Blueprints.
- Keep `DubitoCore` independent from the `Engine` module.
- The server owns all rule decisions.
- Clients send intent and render confirmed replicated state.
- Use owner-only replication for exact private hand state.
- Use multicast events only for self-contained public reveal/game-over events.
- Treat RPC validation failure as abusive or malformed input, not ordinary illegal gameplay.

## Human Action Rules

- If a task needs a human decision, account action, purchase, license review, or manual asset import, state it explicitly in
  the relevant conception doc or roadmap phase/sub-phase.
- Assets are selected and imported by the project owner when a later phase needs them. Agents may define needed asset
  categories, license constraints, timing, and target folders, but must not assume or add external assets without explicit
  approval.
- Runtime-affecting plugins, tools, templates, or asset packs stay out of V1 unless the owner approves a documented blocker.
- Editor-only helper plugins may remain if they do not become V1 runtime dependencies.

## Documentation Rules

- Phase 0 docs contain contracts, decisions, responsibilities, and validation criteria.
- Do not add code snippets or command blocks to conception docs.
- Inline identifiers are allowed when they name a future class, module, file, property, or Unreal concept.
- Roadmap phases are milestones; roadmap sub-phases are execution units for agents.
- If a fact appears in two docs, update both or remove one duplicate.
- If this file changes, copy the exact same content to `AGENT.md`, `CLAUDE.md`, `Documentation/AGENT.md`, and `Documentation/CLAUDE.md`.

## Closeout

Before stopping after meaningful documentation work:

1. Check `Documentation/DESIGN.md`, `Documentation/ARCHITECTURE.md`, `Documentation/ROADMAP.md`, `AGENT.md`, `CLAUDE.md`, `Documentation/AGENT.md`, and `Documentation/CLAUDE.md` for contradictions.
2. Confirm all `AGENT.md` and `CLAUDE.md` copies are identical.
3. Search for stale paths, code fences, and deferred V2 items accidentally marked as V1.
