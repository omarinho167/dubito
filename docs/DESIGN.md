# Dubito - V1 Design Spec

This file defines what V1 must be. It is intentionally concise: rules, UX contract, V1 scope, and design constraints.

Detailed edge cases live in `docs/v1/EDGE_CASES.md`. Architecture lives in `docs/ARCHITECTURE.md`.

No code snippets belong in this conception file.

## V1 Promise

Dubito V1 is a complete playable Steam card game with readable bluffing, stable multiplayer, and a restrained greybox
presentation. It should feel clear before it feels expensive.

The V1 player fantasy:

- sit at a table with other players;
- play hidden cards face down;
- claim a locked round value and a bluffable count;
- decide whether to believe or Doubt the previous player;
- win by emptying the actual hand and surviving the final Doubt window.

## Scope

| In V1 | Out of V1 |
|---|---|
| 2 to 8 players | dedicated servers |
| Steam-focused online play | cross-platform accounts |
| listen-server host | host migration |
| full rules loop | reconnect into a live match |
| greybox infernal tavern | final tavern art pass |
| simple seated placeholders | MetaHuman characters |
| clear card/table UX | emotes and cosmetics |
| simple audio feedback | dynamic ambience system |
| keyboard/mouse plus gamepad-ready UI | mobile/console ports |
| complete testable rules | Guillotine mechanic |
| owner-approved assets only when a phase asks for them | agent-selected external asset packs |

## Glossary

| Term | Meaning |
|---|---|
| Actual cards | Cards really selected/played by a player. Hidden until reveal. |
| Actual count | Number of actual cards played. Hidden until reveal. |
| Claimed count | Public number claimed by the player, from 1 to 4. It may lie. |
| Round value | Public value opened on an empty pile and locked until the pile empties. |
| Public ledger | Display count visible to players. It tracks claims/stakes, not guaranteed reality. |
| Claimed pile ledger | Public pile count based on claimed counts, not actual pile size. |
| Actual pile size | Real number of hidden cards in the pile. Server-only before reveal/transfer. |
| Pending win | State after a player empties actual hand, before the next player's final Doubt window resolves. |

## Rules

### Quick Contract

| Question | V1 answer |
|---|---|
| Players | 2 to 8 |
| Deck | Standard 52-card deck, no jokers |
| Deal | Even as possible; leftovers are removed face down |
| Goal | First to empty actual hand, after final Doubt window |
| Play size | 1 to 4 actual cards |
| Round value | Opener chooses on empty pile; locked until pile empties |
| Claim | Value must be the round value; claimed count from 1 to 4 can lie |
| Doubt | Only immediate next player, before any other action |
| Discard | Active player may discard a non-empty pile, then skips turn |
| Timer | 45 seconds per decision turn |
| Anti-AFK | 3 consecutive timeouts counts as disconnect |

### Setup

1. Shuffle a 52-card deck.
2. Deal cards as evenly as possible.
3. Remove leftovers from the game.
4. Randomize turn order.
5. Start with an empty center pile and no round value.

### Play

- The active player secretly plays 1 to 4 actual cards face down.
- If the pile is empty, this play opens the round and chooses the round value.
- If the pile is not empty, the claimed value must match the locked round value.
- The claimed count is always allowed to differ from the actual count.
- Turn passes to the next player.

### Doubt

- Only the immediate next player can Doubt the previous play.
- Doubt must happen before that player plays or discards.
- The previous play is a lie if actual count differs from claimed count, or any actual card is not the round value.
- Correct Doubt: liar takes the pile; doubter then plays normally.
- Wrong Doubt: doubter takes the pile and loses the turn.

### Discard

- Legal only for the active player.
- Legal only when the center pile is non-empty.
- Blocked while a win is pending.
- Removes the pile from the game, clears the round value, and skips the active player's turn.

### Timer

On timeout:

1. decline any available Doubt;
2. auto-play exactly one actual card;
3. claim count 1;
4. claim truthfully if possible;
5. if the locked round value makes truth impossible, perform a forced minimal bluff.

Timeout never auto-discards.

### Winning

- Playing the last actual card creates a pending win, not an immediate win.
- The next player may still Doubt.
- Correct Doubt cancels the win because the would-be winner takes cards back.
- Wrong Doubt confirms the win.
- No Doubt confirms the win when the next player plays or times out.
- Discard is blocked during pending win.

### Disconnection

- A disconnected player's hand is removed from the game.
- The center pile remains.
- If the disconnected player was the previous claimant, their claim is no longer doubtable, but the pile and round value stay.
- If only one player remains, that player wins.
- If the host leaves, the match ends for everyone.

## Hidden Information Model

The count bluff is a core mechanic. V1 must preserve the glossary distinction between actual information and public
ledgers.

### Exact Information

| Information | Who may know before reveal |
|---|---|
| Own hand cards and exact count | Owner and server |
| Opponent hand contents | Server only |
| Actual cards/count just played | Server, then all only on valid reveal |
| Actual pile size | Server, then all only on reveal/transfer if needed |
| Claimed value/count | Everyone |

### Public Ledgers

Public counts are ledgers, not proof. Opponent badges must be visually unverified after hidden play starts. Own hand count
is exact. Pending-win uses actual server state, not the public ledger.

Presentation rule: opponents see a neutral face-down packet when someone plays, never the actual selected count.

## UX Contract

V1 must always answer four questions:

1. whose turn is it;
2. what claim can I judge;
3. what actions can I take now;
4. what public stake/ledger does the table show.

### Action Matrix

| Situation | Play | Doubt | Discard |
|---|---|---|---|
| Not my turn | off | off | off |
| My turn, empty pile | on | off | off |
| My turn, round open, previous claim doubtable | on | on, emphasized | on |
| My turn, no doubtable claim | on | off | on if pile non-empty |
| My turn, pending win | on | on, emphasized | off |
| Reveal/resolution | off | off | off |

Disabled actions must explain why on hover/focus/press.

### Interaction Rules

- Play is select cards, choose/confirm claim, then wait for server confirmation.
- Opening a round exposes value and count controls.
- Following a round exposes count control only; value is fixed.
- Doubt is hold-to-confirm, not a one-tap destructive action.
- Discard uses a light confirmation.
- All actions must work with mouse, keyboard, and gamepad focus/confirm.
- No mouse-only interaction.

### Motion Rules

- Motion presents server-confirmed state; it never drives rules.
- If newer authoritative state arrives, animation finishes fast or snaps.
- Opponents never see a card-by-card play animation that reveals actual count.
- The only cinematic beat in V1 is the Doubt reveal.
- Reduce-motion mode is a V1 baseline.

## V1 Presentation

The V1 look is a readable greybox infernal tavern:

- one table;
- up to 8 seats;
- simple seated placeholders;
- first-person seated camera with limited free-look;
- clear center pile area;
- simple card faces/back;
- seat colors and names;
- simple SFX for place, flip, doubt, win.

The art direction is infernal tavern, but V1 must not depend on final assets or agent-selected external packs.

Asset rule: V1 uses Unreal primitives, simple materials, and internally created placeholders first. If a later phase needs
external visual or audio assets, the agent must ask the owner to select or approve them, state the license/ownership check,
and specify the target Unreal folder before import.

Expected asset categories, when needed:

| Category | Timing | Target |
|---|---|---|
| Card faces, backs, and simple card materials | Phase 3 if internal placeholders are not readable enough | `DubitoUE/Content/Cards/` |
| Tavern table, seats, and neutral player placeholders | Phase 3 only if greybox primitives block readability | `DubitoUE/Content/Art/Prototype/` |
| Action and UI SFX | Phase 7 after full gameplay loop validation | `DubitoUE/Content/Audio/SFX/` |
| Store and marketing art | Phase 8 release prep | Steamworks and `DubitoUE/Content/Art/Marketing/` if a project copy is needed |

## Open Playtest Knobs

Validate these with playtests before final balancing:

- 45 second timer;
- 3 timeout disconnect threshold;
- whether public hand ledger needs stronger uncertainty labeling;
- whether Discard is too strong or too weak;
- whether 2-player games need a special pacing tweak.
