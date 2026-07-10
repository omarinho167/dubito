# Dubito - V1 Roadmap

This is the execution plan. It stays at phase/task/validation level and intentionally avoids code snippets or command blocks.

## Phase Overview

| Phase | Name | Status | Goal |
|---|---|---|---|
| 0 | Conception | Done | compact, coherent V1 documentation |
| 1 | Unreal Bootstrap | Next | clean UE project, first-party V1 stack, source-control foundation |
| 2 | Core Rules | Locked | pure rule model with tests |
| 3 | Greybox Table | Locked | readable local table, cards, camera, basic UI shell |
| 4 | Network Framework | Locked | host-authoritative replicated match state |
| 5 | Full Gameplay Loop | Locked | Play, Doubt, Discard, timer, win, post-game |
| 6 | Steam Multiplayer | Locked | Steam lobbies/invites tested on real machines |
| 7 | V1 Polish | Locked | clarity, accessibility, UX pass, packaging |
| 8 | Release Prep | Locked | real AppID, depot, store/build checklist |

## Phase 0 - Conception

Goal: make V1 coherent before any gameplay code.

Tasks:

- [x] Define rules and hidden-count model.
- [x] Define V1 scope versus V2.
- [x] Define first-party V1 Unreal stack.
- [x] Remove code snippets from conception docs.
- [x] Split long reference material into routed detail docs.
- [x] Mirror `docs/AGENT.md` and `docs/CLAUDE.md` exactly.
- [x] Record CommonUI as accepted and keep other plugins/assets out of the V1 baseline.
- [x] Record where human actions are required instead of letting agents assume them.
- [x] Final contradiction pass after this refinement.

Done when:

- docs under `docs/` are concise enough for agents to route by task;
- V1 stack has no avoidable beta or third-party dependency;
- no conception doc contains code fences or command blocks;
- `docs/AGENT.md` and `docs/CLAUDE.md` are identical and explain what to read.

## Phase 1 - Unreal Bootstrap

Goal: create the project foundation without gameplay complexity.

Tasks:

- [ ] Confirm installed Unreal version: 5.8 target, 5.7 fallback only if bootstrap blocks.
- [ ] Create `DubitoUE` as a blank C++ Unreal project.
- [ ] Copy the repository `docs/` folder into `DubitoUE/Documentation/`.
- [ ] Keep all concept Markdown under `docs/`; do not scatter documentation outside that folder.
- [ ] Add modules for rules, game runtime, and tests.
- [ ] Enable required V1 systems: Online Subsystem Steam, Online Subsystem Utils, Common UI, UMG, Enhanced Input, Automation support.
- [ ] Configure Git ignore rules before generated Unreal files accumulate.
- [ ] Configure Git LFS before binary assets land.
- [ ] Add local development AppID 480 setup for Steam testing.
- [ ] Human: confirm installed Unreal version and local Steam testing account availability.
- [ ] Human: confirm Git LFS is acceptable before any binary asset enters the repository.

Done when:

- project opens with zero startup errors;
- modules compile;
- required plugins/systems are enabled;
- source control ignores generated folders;
- no binary assets are tracked outside LFS rules.

## Phase 2 - Core Rules

Goal: implement the engine-independent rule model first.

Contracts to cover:

- card and deck integrity;
- deterministic shuffle/deal;
- hand add/remove semantics;
- announcement validity;
- locked round value;
- claimed count bluff;
- Doubt correctness for value and count lies;
- discard legality;
- timeout behavior;
- pending-win window;
- disconnection behavior;
- public ledger updates versus hidden actual counts.

Done when:

- rule tests pass;
- rules do not depend on actors, widgets, maps, or live networking;
- hidden-count invariants are tested.

## Phase 3 - Greybox Table

Goal: make the game readable locally before multiplayer complexity.

Tasks:

- [ ] Build Main Menu, Waiting Room, Table, and Post Game maps as greybox.
- [ ] Add a seated camera with limited free-look.
- [ ] Add table, 8 seats, center pile anchor, and simple player placeholders.
- [ ] Add simple card visuals sufficient to identify own hand.
- [ ] Add CommonUI/UMG shell for menu, HUD, action bar, confirm modal, reveal panel.
- [ ] Prove gamepad focus/confirm works for menu and action bar.
- [ ] Human, only if needed: select or approve external card/table placeholder assets after the agent specifies category,
      license constraint, and target folder.

Done when:

- one local player can see the full table layout;
- own hand and center pile areas are readable;
- no UI surface requires mouse-only interaction;
- opponent packet/pile presentation does not imply actual hidden counts.

## Phase 4 - Network Framework

Goal: connect Unreal gameplay framework to the pure rule model.

Tasks:

- [ ] Add server-only authority in GameMode.
- [ ] Replicate public match state through GameState.
- [ ] Replicate public player identity/readiness/ledger through PlayerState.
- [ ] Replicate exact private hand only to the owning PlayerController.
- [ ] Add server actions for Play, Doubt, Discard, Ready, and Start Match.
- [ ] Add controlled rejection/resync for stale or illegal actions.
- [ ] Add reveal and game-over public events.
- [ ] Add server-authoritative turn timer.

Done when:

- PIE listen-server test runs with 2 to 3 players;
- deal, turn advance, public claim, public ledgers, and owner-only hands replicate correctly;
- non-owners cannot observe actual played counts before reveal;
- illegal actions reject without disconnecting ordinary players.

## Phase 5 - Full Gameplay Loop

Goal: make one complete local/networked game playable.

Tasks:

- [ ] Implement Play interaction: selection, claim, server confirmation, turn advance.
- [ ] Implement Doubt interaction: hold-to-confirm, reveal, verdict, pile transfer.
- [ ] Implement Discard interaction: confirmation, pile clear, skipped turn.
- [ ] Implement timeout auto-play and anti-AFK disconnect behavior.
- [ ] Implement pending-win and post-game flow.
- [ ] Implement local session flow from menu to waiting room to table to post-game.
- [ ] Add first-run help card and persistent help access.
- [ ] Verify all edge cases in `docs/v1/EDGE_CASES.md`.

Done when:

- two packaged local instances can complete a full game;
- winner, wrong Doubt, right Doubt, discard, timeout, and disconnect paths are all validated;
- public ledger UI remains understandable and never pretends hidden counts are exact.

## Phase 6 - Steam Multiplayer

Goal: prove the real Steam transport.

Tasks:

- [ ] Configure Steam lobbies/sessions through Online Subsystem Steam.
- [ ] Test AppID 480 locally with Steam running.
- [ ] Test host, find, join, invite, and lobby full/error states.
- [ ] Test two machines with two Steam accounts.
- [ ] Verify travel from waiting room to table and back.

Done when:

- Steam-hosted match works outside PIE;
- invite/join flow is reliable enough for V1;
- host leave ends the session gracefully;
- no Steam-only path bypasses rule validation or hidden information rules.

## Phase 7 - V1 Polish

Goal: make the playable game clear, robust, and shippable enough for first release testing.

Tasks:

- [ ] Pass through every disabled-action reason.
- [ ] Pass through gamepad/keyboard/mouse navigation.
- [ ] Add reduce-motion support.
- [ ] Add simple SFX for key actions.
- [ ] Add readable timing and feedback for pending state.
- [ ] Fix text overflow at 1280x800 and common desktop resolutions.
- [ ] Package a Development build for external playtest.
- [ ] Human, only if needed: select or approve external SFX after the agent specifies exact event list, license constraint,
      and target folder.

Done when:

- new players understand turn, claim, legal actions, and public stake without explanation;
- no V1 flow requires developer tools;
- packaged build survives repeated full-game playtests.

## Phase 8 - Release Prep

Goal: prepare the Steam release path after V1 is proven.

Tasks:

- [ ] Purchase/activate the real Steam app credit.
- [ ] Replace development AppID with real AppID in release configuration.
- [ ] Remove local `steam_appid.txt` from shipped depot.
- [ ] Prepare store capsule, screenshots, description, and build depot.
- [ ] Run release packaging validation.
- [ ] Human: provide or approve final Steamworks assets and any required account/payment actions.

Done when:

- Steamworks release checklist has no blocking technical item;
- Shipping build launches through Steam;
- V1 scope is frozen unless a release blocker appears.

## Human Actions By Phase

Agents must call these out when the phase is reached instead of silently assuming them.

| Phase | Human action | Trigger |
|---|---|---|
| 1 | Confirm installed Unreal version and Steam account availability for local testing | before bootstrap validation |
| 1 | Confirm Git LFS/locking policy | before any `.uasset`, `.umap`, audio, or texture file is tracked |
| 3 | Select or approve external visual assets only if placeholders are not readable enough | before import into `DubitoUE/Content/Cards/` or `DubitoUE/Content/Art/Prototype/` |
| 6 | Provide two Steam accounts and two test machines, or explicitly accept a reduced test pass | before Steam multiplayer validation |
| 7 | Select or approve external SFX only after exact event needs are known | before import into `DubitoUE/Content/Audio/SFX/` |
| 8 | Purchase/activate Steam app credit, provide real AppID, approve store assets | before release packaging |
