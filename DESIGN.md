# Dubito — Game Design (Rules · UX · Interactions · Art)

> **The complete "what to build" spec:** the rules, how it feels and reads, exactly how every interaction and
> state-change works, every edge case, and the art direction. The **"how it's engineered"** (engine, C++, networking,
> plugins, folders, constants) is in `ARCHITECTURE.md`; the **execution order** is in `ROADMAP.md`; project conventions
> and the entry point are in `CLAUDE.md`.
>
> Living document. If any two statements ever disagree, the **Rules** section (Part A) wins for game logic and this file
> wins for UX/art; fix the other place immediately.

**Contents**
- **Part A — Rules** (the game logic; the source of truth)
- **Part B — UX foundations** (simplicity north-star, pillars, mental model, the always-answers, state×action matrix)
- **Part C — HUD, interactions & motion** (hybrid HUD, per-action mechanics, motion system, network feel, onboarding, accessibility)
- **Part D — Exhaustive edge-case coverage** (every case has a defined response)
- **Part E — Art direction** (infernal tavern, camera, cards, characters, audio)
- **Part F — Open decisions** (to validate in playtest)

---
---

# PART A — RULES

> The game logic, engine-agnostic. All code must match this. Implementation notes point to `ARCHITECTURE.md`.

## Quick Reference

| Question | Answer |
|---|---|
| Players | 2 – 8 |
| Deck | 52 cards, no jokers |
| Goal | First to empty their hand |
| Round value | Chosen by whoever **opens** the pile, then **locked** for that round; resets when the pile is emptied (discard, or taken on a doubt) |
| Announce | Value is fixed to the round value; only the **count** (1–4 cards face down) is freely lied about |
| Who can doubt | Only the immediate next player, before they play |
| Wrong doubt | Doubter takes the center pile + loses their turn |
| Right doubt | Liar takes the center pile, doubter then plays normally |
| Discard pile | Active player removes the center pile from the game, skips their turn |
| Turn timer | 45 s per turn; on timeout the turn auto-resolves (see *Turn Timer*) |

## Setup
1. Shuffle a standard 52-card deck (no jokers).
2. Deal all cards as evenly as possible; leftover cards are discarded face down, out of play.
3. Randomize play order.
4. No center pile at game start.

## Turn Structure — one of three actions

### Action 1 — Play
- Place **1 to 4 cards** face down onto the center pile.
- **The announced value (rank) is fixed for the whole round:**
  - Whoever plays onto an **empty pile opens the round** and **freely chooses the announced value** (e.g. "a round of Kings").
  - **Every player after them must announce that same value.** Only the **count** stays a free lie.
- **The count is always a free bluff** — the announced count (1–4) need not match the number of cards actually placed.
- So for anyone but the opener, the lie is whether the cards they actually place are really that value (and/or how many).
- The round value **resets when the pile is emptied** (a discard, or someone taking the pile on a doubt). The next player
  to play onto the empty pile opens a new round with a new value.
- Turn passes to the next player.

> **Pile visibility:** everyone can count the total cards in the pile at any time, but there is **no** visual marker of
> which cards were just added — players track that mentally.

### Action 2 — Doubt
- Challenge the **previous player's** last announcement. Only **before** the active player places any cards; once they
  begin Action 1 or 3, the previous play is locked and can no longer be doubted.
- Reveal the previously played cards:
  - **RIGHT** (they lied) if the actual cards mismatch the announcement in **count OR value** → liar takes the whole pile;
    the doubter then plays normally (Action 1).
  - **WRONG** (they were honest — correct count AND all cards are the round value) → doubter takes the whole pile + **loses their turn**.

### Action 3 — Discard the Center Pile
- Remove all pile cards permanently (face down, out of play). Active player **skips their turn**.
- The round value resets; the next player opens a new round. Turn passes on.

## Turn Timer
Each turn is capped at **45 s** to keep the game moving and stop AFK stalling. The countdown is shown to everyone. It runs
**only during a player's own decision window** and is **paused during reveal/resolution**; it resets each turn.

**On timeout, the turn auto-resolves to the safest legal action** (never a lie, never a sacrifice):
- First **decline any available doubt** (default = believe the claim).
- Then **auto-play exactly one card, announced truthfully** (count 1, its real value if it is the round value; otherwise
  the safest legal single card / a legal open). A player always holds ≥1 card on their turn, so this is always possible.
- Auto-play beats auto-discard on purpose — discarding is a deliberate sacrifice and must never happen by accident.

**Special windows:** in a **doubt window**, timeout = decline + auto-play one truthful card. In a **pending win**, timeout =
the forced action **confirms the opponent's win** (Discard stays blocked).

**Anti-AFK:** **3 consecutive timeouts** = treated as a disconnect (hand discarded, removed from turn order). A voluntary
action resets the streak. Constants (`45`, `3`) live in `ARCHITECTURE.md` → *Constants*.

## Winning
First to empty their hand wins, but the **next player may still doubt before the win is confirmed**:
- After the last card(s) are played, the next player may call Doubt before any other action.
- **Doubt right** → the (would-be) winner takes back their cards + the pile; game continues.
- **Doubt wrong** → doubter takes the pile + loses their turn; the winner's hand is still empty → **win confirmed**.
- **No doubt** → as soon as the next player begins any other action (Play/Discard), the win is confirmed.

## Disconnections
- The player's hand is **permanently discarded** (as in Action 3); they're removed from the turn order; the pile is unaffected.
- If it was their turn, it passes on immediately. If they were the previous player and the active player hadn't acted, the
  doubt window closes (their last play is void).
- If only **1 player remains**, they win automatically (last player standing).
- **Host (listen-server) leaves** → the session ends for everyone (clear message → Main Menu). Host migration is out of V1
  scope.

## Rules-level Edge Cases
- **Uneven deal:** leftovers are removed from the game, not kept in a pile.
- **Pile after a win:** irrelevant — the game ends as soon as the win is confirmed.
- **1-card hands:** you may play your last card; the next player may doubt; if none, the win confirms when they act.
- **Wrong doubt on an empty hand:** doubter takes the pile + loses their turn; winner's hand still empty → win confirmed.
- **Discard after a last play:** you **cannot** discard while a previous player's empty hand awaits confirmation — you must
  doubt or play (which confirms the win).
- **Opening with a value you don't hold:** always legal — opening declares the round value; the bluff is whether your
  placed cards are actually that value.

> The full *interaction-level* and *network-level* edge cases (timeouts during modals, disconnects mid-reveal, etc.) are
> in **Part D**.

## V2 — The Guillotine *(not implemented in V1)*
Each lost doubt (liar or wrong doubter) drops the guillotine blade above that player's seat one notch; notches escalate
in-game penalties. Full design TBD (visual treatment in Part E).

---
---

# PART B — UX FOUNDATIONS

> Dubito is hidden-information bluffing. Players quit bluffing games when they don't understand *what just happened* or
> *what they may do*. Everything here optimizes for **instant legibility** + **simplicity**.

## B.0 — Design for simplicity (the north star)
Benchmarks: **Liar's Bar** (5M+ sold, 88% positive — "simple rules, deep mind games") and **Balatro** (minimal surface,
ruthless clarity). Both pair *trivial-to-learn* with *hard-to-master* and let the **feel** teach. Six ranked principles
every screen obeys:

1. **Contextual minimalism** — show only what's needed, when it's needed. The only permanent things are the four
   always-answers (§B.3). Everything else appears on demand and leaves. A calm screen is a readable screen.
2. **Zone discipline (Balatro model)** — fixed regions so the eye never hunts: **status = top**, **information = left**,
   **interaction = bottom**, **stakes = center**.
3. **Colour-coded, label-free reading** — key values parse without reading text (each seat a fixed colour, round value a
   colour + token, big pile count). Never colour *alone* — pair with icon/word (accessibility, §C.6).
4. **Juice is the design, not polish** — feedback ships *with* each mechanic (the card slam, the count tick, the reveal
   flip+verdict *are* the payload). Budget it from the start (fidelity scales V1→V2).
5. **One decision at a time** — the loop is always the same two-step (§B.2); the UI never surfaces more than the current
   decision; the primary action is emphasised, the rest recede.
6. **Progressive onboarding, skippable** — teach as they play, one idea at a time, first game only (§C.5); veterans skip.

**The one test that matters:** a first-timer, with *no* explanation, plays a correct turn — decides whether to doubt,
plays cards, reads the reveal — from on-screen cues alone. If a screen fails that, it's too complex.

## B.1 — UX Pillars (non-negotiable, ranked)
1. **Always answer the four questions** (§B.3): whose turn, what was claimed, what can *I* do, how many cards (incl. pile).
2. **Affordance + signifier for every action** — possible actions *look* possible; impossible ones *look* impossible **and
   say why** on hover/press. Never a dead button with no explanation.
3. **Every input gets immediate feedback** — within one frame; no silent actions.
4. **Teach by showing, one idea at a time** — no rulebook dump; the reveal panel re-teaches the count-and-value rule every time.
5. **The doubt moment is the show** — dramatic, unambiguous, self-explaining (§C.3).
6. **Fail safe, not sorry** — prevent illegal/mis-clicks up front (disable + explain); confirm only irreversible things.

## B.2 — The player's mental model
On **your turn**, always the same two-step, in order:
```
   1. Do I BELIEVE the previous claim?  → Yes/don't care: go to 2   → No: DOUBT (reveal now)
   2. Make my move:  PLAY 1–4 cards claiming a count of the round's value (count may be a lie)
                     or DISCARD the pile (skip my play)
```
**Starting step 2 (Play or Discard) permanently forfeits your right to doubt** — so during the doubt window, Doubt is the
primary, time-sensitive choice. Only the **immediate next player** may doubt, only **before acting**; everyone else is a
spectator ("Waiting for {name}…"). **Opening vs following:** empty pile → *you* pick the value; open round → value is fixed,
you only choose (and may lie about) the count.

## B.3 — Information Architecture (the always-answers)
| # | Info | Delivered via (see §C.1) |
|---|---|---|
| Q-a | **Whose turn** | In-world active-seat glow + a compact turn chip |
| Q-b | **Previous claim** (doubtable) | Pinned top-left, **only** in your doubt window ("◀ 2 × King") |
| Q-c | **My actions** | Action bar (bottom), only on your turn; only legal ones lit, with reasons |
| Q-d | **Card counts** | Per-seat badges + the pile's large count (in-world) |
| + | Round value | In-world token on the table + pile tint (while a round is open) |
| + | My hand | In-world 3D cards, owner-only |

> "Always answerable" ≠ "always a 2D banner." Most of these live **in the 3D world** or appear **contextually**; only the
> claim-to-judge and your action bar are pinned. See the hybrid HUD, §C.1.

## B.4 — State × Action matrix (the interaction logic)
A pure function of *phase, my turn?, doubtable play?, pending win?, pile empty (opening)?*:

| Situation | PLAY | DOUBT | DISCARD | Primary prompt |
|---|---|---|---|---|
| Not my turn | off | off | off | "{Name}'s turn"; if a doubt window is open elsewhere: "Waiting for {name} to respond…" |
| My turn, **pile empty** (I open) | on (1–4 sel.) | **off** | on | "Open a round: play 1–4 cards and name the value." |
| My turn, round open, doubtable play | on | **on, emphasised** | on | "Believe {prev}'s claim of {n} × {value}, or doubt it." |
| My turn, **win pending** | on | **on, emphasised** | **off** (blocked) | "{prev} is about to WIN — doubt now or they win!" |
| Play selected, count not 1–4 | greyed | — | — | "Select 1–4 cards." |
| Reveal / resolving | off | off | off | World dims; reveal panel owns the screen. |
| Game over | — | — | — | Post-game overlay. |

Rule encoded: **choosing Play or Discard forfeits Doubt** — the instant the announce panel opens or a discard confirms,
Doubt is gone for that claim.

---
---

# PART C — HUD, INTERACTIONS & MOTION

> The exact "how". Decisions locked: **hybrid HUD**, **sober motion + one cinematic doubt reveal**, **reduce-motion always
> available**, **authoritative networking with light pending states (no client prediction)**.

## C.1 — HUD model: hybrid, with a "resting screen"
The anti-heaviness idea: **at rest the screen is almost pure 3D world.** Info lives **in the world** where it has a natural
place; the 2D overlay is trimmed to the two things you must never lose. Chrome earns its place or it isn't drawn.

**Where each piece lives:**
| Information | Home | Shown when |
|---|---|---|
| Whose turn | **In-world:** active seat glows/lifts + soft spotlight | Always (ambient) |
| Turn + timer (self) | **Pinned, compact chip** top-center ("YOUR TURN · ⏱35") | Only your turn / last-5s warning |
| **Claim to judge** | **Pinned, minimal** top-left ("◀ 2 × King") | **Only** during *your* doubt window |
| A fresh claim | **In-world** speech bubble over the claimant ("2 × King?") | ~2.5 s after they play, then fades |
| Round value | **In-world** suited token on the table + pile tint | While a round is open |
| Pile size | **In-world** large number over the pile | Always (public stakes) |
| Per-player counts | **In-world** badge over each seat | Always (ambient) |
| **Your actions** | **Pinned** action bar bottom (Play · Doubt · Discard) | **Only on your turn**; fades otherwise |
| Selection helper ("n/4") | **Contextual**, near the hand | Only while cards are selected |
| Prompt / spectator line | **Contextual**, under the turn chip | Only when it adds something |
| Help / settings | **Corner button** (`?` / ⚙) | On demand |
| Your hand | **In-world** 3D cards, fanned, bottom | Always (owner-only) |

```
   RESTING (someone else's turn)                    ACTIVE (your doubt window)
   ┌────────────────────────────────┐               ┌────────────────────────────────┐
   │            Vesper's turn        │ ← tiny chip   │  ◀ 2×King?      YOUR TURN ⏱35   │
   │   ✦(glow) seat   seat   seat    │               │   seat   seat   ✦Mortem"2×K?"   │
   │         [ pile · 14 ]           │               │         [ pile · 14 ]           │
   │   🂠 🂠 🂠 🂠 🂠  (your hand)      │               │   🂠 🂠 🂠 🂠 🂠                   │
   │                                 │               │      [Play] [ DOUBT ] [Discard] │
   └────────────────────────────────┘               └────────────────────────────────┘
     pure world + 1 chip                               world + claim + action bar only
```
**Why hybrid (not full diegetic):** full-diegetic reads beautifully but is *risky where clarity is critical* (the
"minimal-HUD paradox" — diegetic dreams become cluttered nightmares). A bluffing game lives on "whose turn / what was
claimed," so those get a guaranteed, legible **pinned** home; everything spatial stays in-world where it reads naturally.
This is the AAA hybrid pattern. The action bar keeps a fixed order (Play · Doubt · Discard); the primary action for the
state is emphasised; Play carries its count ("PLAY (2)").

## C.2 — Interaction model per action
All interactions are **input-agnostic** (CommonUI + Enhanced Input): mouse, keyboard, and **gamepad / Steam Deck**. Button
prompts show the **active input's glyphs** (`LMB` vs `Ⓐ`); verbs stay neutral. **No mouse-only interaction** — everything
is reachable by focus + confirm.

**Select cards (reversible, no friction):** point/focus → lift + outline; confirm → raise + coloured outline + check + soft
click + "Selected: n/4"; cap **4** (a 5th is refused with a shake + "Max 4"); deselect returns to the fan. Stays
**select-then-Play** (not drag-to-pile) because you may claim a *different* count than you hold, and it works identically on a controller.

**Play → announce → confirm (where the locked-value rule surfaces):**
1. Play (enabled with 1–4 selected, your turn) → announce panel (world dims).
2. **Opening** (pile empty): **Value** + **Count** steppers (count pre-filled). Preview: "You open a round of **Kings** — claiming **2**."
   **Following** (round open): **no value control** (shown fixed, "Round: Kings"); only **Count** is editable (pre-filled,
   editable to lie). Preview: "You claim: **2 × King**." Reminder: *"Only the count can be a lie."*
3. Confirm → `Server_PlayCards`; button enters a **pending** state (§C.4); on confirm the panel closes and the play animates.
   Cancel → back to the hand, selection intact.

**Doubt (irreversible, deliberate friction):** offered only to the immediate next player, before acting, when a doubtable
claim exists. **Hold-to-doubt (~0.4 s):** hold fills a radial ring; **release early = cancel**; a tap does nothing (flashes
"hold to doubt"). On commit → the cinematic reveal (§C.3).

**Discard (irreversible, light confirm):** your turn, not during a pending win → light confirm ("Discard the pile? You'll
skip your play.") → `Server_DiscardPile`; pile clears, round resets.

**Look around (free-look, always available):** move the camera (mouse / right-stick) to pan across the table within
clamped yaw/pitch — read faces, **stare down**, glance at a seat; **recenter** (button + idle auto-recenter) returns to the
rest framing. Look is **purely local and cosmetic** — it never touches game state, is available on anyone's turn, and is
suspended only while a cinematic beat (the reveal) owns the camera. Card selection/hover still works while looking (the
hand and buttons are screen-anchored). *(V2 layers emote/gestures on top — see Part E "Social expression".)*

**Ready / menus:** Waiting Room **Ready** toggles instantly (it's not game state) and broadcasts. Menus/modals are CommonUI
activatable widgets — input routes to the top layer only (no focus theft); Back/Cancel always dismisses one layer (no dead ends).

## C.3 — Motion & micro-interaction system
Timing: micro-interactions **100–200 ms**, transitions **200–500 ms**; **ease-out** entering, **ease-in** leaving,
**ease-in-out** for moves; **never linear**; motion *means* something or it's cut. One token set:

| Token | Duration | Easing | Used for |
|---|---|---|---|
| `instant` | 0 ms | — | Values that must read as immediate (count text) |
| `quick` | 120 ms | ease-out | Hover/focus lift, select, button press, chip in/out |
| `base` | 220 ms | ease-in-out | Panel open/close, action-bar fade, card→pile flight |
| `slow` | 400 ms | ease-in-out | Pile transfer slide, seat-glow move, scene crossfade |
| `beat` | see below | choreographed | **The doubt reveal only** |

**Event → motion + sound:**
| Event | Motion | Token | Sound |
|---|---|---|---|
| Turn becomes yours | glow slides to you; chip fades in; action bar rises | `slow`/`base` | rising chime |
| Turn (someone else) | glow slides to their seat; your action bar fades out | `slow` | — |
| Card select/deselect | lift + outline / lower | `quick` | soft click |
| Play → pile | cards arc face-down onto the pile + settle; count ticks +n; claim bubble pops | `base` | `card_place` |
| Discard | pile slides off / fades; count → 0 | `slow` | whoosh |
| Pile transfer (post-doubt) | pile slides to the loser; their badge bumps | `slow` | whoosh |
| Timer last 5 s | chip turns red, one pulse/second | `quick` | soft tick |
| Illegal/refused | 1-frame shake + reason tooltip | `quick` | tiny deny blip |
| Auto-play (timeout) | one card plays itself + "Auto-played {card}" toast | `base` | `card_place` |
| Pending (in-flight) | control dims + small spinner | `quick` | — |

**The default is quiet** — no shake, no particles, no bounce on ordinary actions. That's the "clean & modern, not heavy"
contract: the eye is only pulled when something matters.

### Animation catalog (V1 = functional tweens · V2 = + physicality, character, VFX)
Every animated element, its trigger, and how it behaves if interrupted. Timings use the tokens above.

**Cards**
| Element | V1 | V2 adds | Token · easing | Trigger | If interrupted → |
|---|---|---|---|---|---|
| Deal (game start) | cards fly from a center stack to each seat, staggered | riffle-shuffle flourish | `base`, ~60 ms/seat · ease-out | Dealing phase (state) | snap all hands to dealt |
| Hover / focus | lift + subtle tilt toward camera | — | `quick` · ease-out | pointer/focus enter | revert instantly |
| Select / deselect | raise + outline + check / lower | — | `quick` · ease-out | click / confirm | reflect current selection |
| Play → pile | selected cards arc face-down onto the pile + settle (slight overshoot) | real toss w/ spin + table impact | `base` · ease-in-out | play confirmed (state) | snap onto pile, count correct |
| Flip (reveal) | each card **lifts off the pile then rotates** (never flat), staggered | thickness, shadow, weightier flip | `base`, ~150 ms/card · ease-in-out | reveal multicast | jump to face-up |
| Pile transfer (post-doubt) | pile slides to the loser + badge bump | cards scoop toward loser | `slow` · ease-in-out | resolution (state) | snap pile to owner, counts correct |
| Discard | pile slides off / fades; count → 0 | cards burn / fire whoosh | `slow` · ease-in | discard confirmed (state) | snap to empty pile |

**Table & seats**
| Element | V1 | V2 adds | Token |
|---|---|---|---|
| Turn glow move | active-seat glow slides to the new seat | warm rim-light + faint spotlight | `slow` |
| Card-count badge | tick / bump on change | — | `quick` |
| Claim bubble | pops over the claimant, holds ~2.5 s, fades | speech-styled, small bob | `base` in/out |
| Round-value token | fade in on open, out on reset | carved / emissive token | `base` |
| Pile stack | one card added per play (height capped) | wax / scorch, subtle settle | `base` |

**HUD & panels (UMG)**
| Element | Motion | Token |
|---|---|---|
| Turn / timer chip | fade + tiny slide in/out; red pulse in the last 5 s | `quick` |
| Action bar | rises on your turn / drops otherwise | `base` |
| Announce / confirm / reveal panels | scale-fade in (from ~0.98), world dims behind | `base` · ease-out |
| Toast ("Auto-played…", "{name} left") | slide-fade in, hold, out | `quick` in / `base` out |
| First-run hint | dim the rest, spotlight the target, fade | `base` |

**Camera**
| Beat | Motion | Token |
|---|---|---|
| Free-look | 1:1 to input within clamped yaw/pitch, light smoothing | `instant`/`quick` |
| Recenter | eased return to the rest framing | `slow` |
| Doubt push (reveal) | dolly toward the pile, then eased return | `beat` |
| Win spotlight | subtle push to the winner's seat behind the overlay | `slow` |

**Characters** — V1: one **static seated idle**. V2: breathing idle + reactions (caught lying → recoil / look away;
successful doubt → triumphant point; penalty → slump; win → stand, arms raised) + emote gestures (Part E).

> **Implementation (tech: `ARCHITECTURE.md` §5):** procedural/variable animations (hand fan, per-card flights, staggered
> deals/reveals) use **code-based C++ tweens** (duration/easing in code, trivial for N widgets); fixed widget transitions
> use **UMG timeline** animations; world actors (cards, pile, glow) use **C++ timelines/curves**. All durations/easings
> come from the shared **token set** so the whole game reads as one system.

**The one cinematic beat — the doubt reveal (`beat`):**
1. **Commit** (~150 ms): world dims, low sting, camera pushes toward the pile.
2. **Flip** (staggered ~150 ms/card): the accused's cards **lift off the pile and rotate** face-up one by one — never
   flipped flat, so they read as physical cards — center screen.
3. **Verdict** (hold ≥1 s): claim-vs-actual with per-card ✓/✗ + count check + plain verdict ("LIE! Mortem was bluffing." /
   "TRUTH! Mortem was honest.") — icon **and** word, never colour alone.
   ```
        CLAIM          vs        ACTUAL
      2 × King                King · King · 7♠   ✓ ✓ ✗ (not a King)   count 3 ≠ 2 ✗
      → LIE! Mortem was bluffing.  → Mortem takes the pile (14).
   ```
4. **Transfer** (`slow`): pile slides to the loser; their count bumps.
5. **Return**: dismiss on tap / short auto-timeout; camera eases back; play resumes. Total ≈ 1.5–2.5 s. It teaches the
   count-and-value rule every time.

**Reduce-motion mode (accessibility, always available):** movement → crossfade/cut; camera push → none; shake → none; the
verdict shows instantly and is held slightly longer; all timings ≤ `base`. No strobe, no rapid scaling.

## C.4 — Network interaction feel (responsive without prediction)
A turn-based card game has small payloads and high latency tolerance → **server is the single source of truth; the client
renders only what the server confirms. No client prediction, no rollback** (which would teleport/pop cards).

**Presentation layer ≠ authoritative state.** Animations are a *presentation* of state the server has **already confirmed**
— never part of the simulation. Rules the code must follow:
- The **server never waits** for a client animation; it resolves and replicates immediately. Animations catch up.
- Each element **animates *toward* the latest authoritative state**. If a **newer** state arrives mid-animation (a fast
  opponent, a disconnect, a timeout auto-resolve), the running animation **finishes fast or snaps** to the new state —
  it never fights it or plays a now-stale outcome (per each row's *If interrupted →* in §C.3).
- Animations are **cosmetic and skippable** — tap-to-skip the reveal jumps to its end state; skipping changes nothing but
  the presentation.
- **No ordering assumptions:** each cosmetic event carries a **self-contained payload** (e.g. `FRevealInfo` = claim +
  actual + verdict + loser + pile count) so the presentation never depends on RPC/replication arrival order.

- **Pending:** on send, the control **disables + shows a spinner**; the world doesn't change yet.
- **Confirm:** on the replicated state/RPC, the animation plays and controls re-enable (spinner only visible past ~250 ms).
- **Reject (rare):** the UI already prevents illegal moves; a stale action (turn passed during lag) is ignored server-side
  and the client reverts to authoritative state with a tiny "Too late" toast — no desync.
- **Timer:** driven by the replicated server deadline; display clamps at 0 → "resolving…".
- Reconnection is out of V1 scope — see Part D for the graceful (non-crashing) handling shipped.

## C.5 — Onboarding & teaching (no text walls)
Contextual, one idea at a time, **first game only**, persisted (`USaveGame`); veterans skip; a `?` help card is always
available. Each hint dims the rest, points once, dismisses on any input:
1. First turn **& pile empty** → hand highlight: *"Open a round — pick 1–4 cards and name their value."*
2. First time **following** → announce panel: *"The value is set (Kings). Just claim how many — that number can be a lie."*
3. First **doubt window** → Doubt + pinned claim: *"They claim 2 Kings. Hold to doubt — only now, before you play."*

**The `?` help card** (one screen, diagrammatic): *Goal — empty your hand first.* · *On your turn pick one:* **Play** 1–4
cards claiming a count of the round's value (count can lie; the opener sets the value) · **Doubt** the previous player (only
you, only before you act) · **Discard** the pile & skip your turn. · *Doubt is right if* the cards don't match the claim —
**wrong count OR any card isn't the round value**; wrong doubt → *you* take the pile + lose your turn. · *45 s per turn or
it auto-plays.*

## C.6 — Accessibility (baseline, shipped in V1)
- **Colourblind:** suits = symbol + colour; verdicts = icon (✓/✗) + word; seats = colour **and** name — never colour alone.
- **Reduce motion:** the mode in §C.3.
- **Readability:** functional text in a clean legible font (gothic display for headers only); sized for a TV/Steam Deck read.
- **Input:** fully input-agnostic; no timed-precision input except the deliberate 0.4 s doubt-hold (no accuracy needed).

## C.7 — Player-action catalog (every input → result)
> The complete list of what a player can *do*, per context, and exactly what happens. Legality gating is §B.4; edge cases
> are Part D. All inputs are device-neutral (mouse / keyboard / gamepad / Steam Deck; prompts show the active glyphs).

**Main Menu**
| Action | Input | Immediate feedback | Result |
|---|---|---|---|
| Create game | Create | "Creating lobby…" + Cancel | host a session → Waiting Room |
| Join (invite) | Steam overlay invite | "Joining…" | → Waiting Room (or error) |
| Join (code) | paste code → Join | "Joining…" | → Waiting Room, or "Not found / full / already started" |
| Join (local) | address (default `127.0.0.1`) → Join | "Joining…" | → Waiting Room |
| Settings | Settings | settings layer opens | change volume / reduce-motion / (V2) reduce-look → `USaveGame` |
| Quit | Quit | confirm | app exits |

**Waiting Room**
| Action | Input | Feedback | Result |
|---|---|---|---|
| Ready ⇄ Not ready | toggle | row colour + chip flip, instant | broadcast; host Start gating recomputes |
| Start (host only) | Start | enabled only if ≥2 present & all ready (else the reason) | ServerTravel → Table; "Starting…" |
| Invite friends | Invite | Steam overlay | friend joins |
| Copy lobby code | Copy | "Copied" toast | — |
| Leave | Leave | host → "Leaving ends the lobby" confirm | → Main Menu (host-leave ends the session) |

**Table (in-game)**
| Action | Input | Feedback | Result |
|---|---|---|---|
| Look around | mouse / R-stick | camera pans (clamped) | cosmetic; social read / stare-down |
| Recenter | bind (e.g. stick-click) or idle | eased return | rest framing |
| Hover a card | point / focus | lift + tilt | — |
| Select / deselect | click / confirm | raise + outline + check / lower; "n/4" | builds the 1–4 selection |
| Open Play | Play (1–4 selected, your turn) | announce panel opens | choose value (if opening) + count |
| Adjust value / count | steppers | live "You claim…" preview | — |
| Confirm play | Confirm | pending spinner → cards fly | `Server_PlayCards`; turn passes |
| Cancel play | Cancel / Back | panel closes | selection kept |
| Doubt | **hold** Doubt (~0.4 s) | radial fill; release-early cancels | `Server_Doubt` → reveal |
| Discard | Discard → confirm | light confirm | `Server_DiscardPile`; turn skipped |
| Skip reveal | tap during the reveal | jumps to end state | cosmetic only |
| Open help | `?` | one-screen help card | — |
| Open settings | ⚙ | settings layer (**timer keeps running** — reminder shown) | — |
| Emote *(V2)* | emote-wheel bind | gesture plays on your character | cosmetic; broadcast; rate-limited |

**Post-game**
| Action | Input | Result |
|---|---|---|
| Play Again / Ready | button | re-ready in the **same** session; all ready → back to Waiting Room |
| Quit | button | → Main Menu |

**Global (any screen):** Back / Cancel (Esc / B) dismisses the top CommonUI layer — never a dead end.

---
---

# PART D — EXHAUSTIVE EDGE-CASE COVERAGE

> Every case has a **defined response**. This is the build checklist that prevents "unforeseen situations." (Core turn
> logic = §B.4; this extends it to the messy reality.)

## D.1 — Turn / phase
| Situation | Response |
|---|---|
| Your turn, **pile empty** | You **open**: Play (needs 1–4 selected) + pick the value; Doubt off; Discard off (nothing to discard). |
| Your turn, round open, doubtable | Play + Doubt (emphasised) + Discard; pinned claim shows. |
| Your turn, **pending win** | Doubt (emphasised) + Play; **Discard blocked** with reason; "about to WIN" banner. |
| Not your turn | No action bar; "{Name}'s turn" chip; doubt open elsewhere → "Waiting for {name}…". |
| Reveal / resolving | **All input locked**; the reveal owns the screen. |
| Game over | Post-game overlay; table frozen behind. |

## D.2 — Boundary & counting
| Situation | Response |
|---|---|
| **2 players (min)** | "Previous" and "next" are the same opponent; doubt targets them; turn ping-pongs. No self-reference. |
| **8 players (max)** | 8 seats ring the table; badges/bubbles sized to fit; camera frames all. |
| Hand = **1 card**, you play it | Triggers the pending-win window for the next player. |
| Select > 4 | Refused (shake + "Max 4"). |
| Play with 0 selected | Play disabled ("Select 1–4 cards"). |
| **Huge pile** (40+) | Count formats cleanly ("«44»"); the pile mesh caps its visual height (never grows forever). |
| **Large hand** after taking a pile | Fan overlaps / gently scrolls; the seat count badge is the source of truth. |
| Opening with a value you don't hold | Legal — the opener declares the round value; UI never restricts the opener's choice. |
| "Deck runs out" | **Cannot happen** — all cards dealt at start; cards only move hand↔pile; no draw pile. |

## D.3 — Doubt resolutions
| Situation | Response |
|---|---|
| Doubt **right** (lie) | Liar takes the pile; **doubter's turn continues** → pile empty → they open a new round. |
| Doubt **wrong** (truth) | Doubter takes the pile + **loses their turn**; round resets. |
| Doubt **right during pending win** | Winner takes card(s) + pile back; **pending win cleared**; game continues; no post-game. |
| Doubt **wrong during pending win** | Doubter takes pile + loses turn; winner still empty → **win confirmed** → post-game. |
| Hold-to-doubt **released early** | Cancel; nothing sent; you may still act. |
| **Tap** doubt (no hold) | Hint "hold to doubt"; nothing sent. |
| Doubt sent but claim no longer valid (lag) | Server ignores; client reverts + "Too late" toast. |

## D.4 — Turn timer / timeout
| Situation | Response |
|---|---|
| Timeout, normal turn | Decline any doubt → auto-play **one truthful card** → "Auto-played {card}" toast → turn passes. |
| Timeout, **pending win** | Forced action **confirms the opponent's win**. |
| **Announce/confirm panel open at timeout** | The timer **does not pause for modals** (else players stall). On expiry: panel auto-cancels, then the timeout auto-resolves; a toast explains. |
| **Help/settings open at timeout** | Same — the shared multiplayer timer never pauses; show a subtle "your turn — timer running" reminder while such overlays are open. |
| Timer during **reveal/resolution** | **Paused** (not a decision window); re-arms next turn. |
| **3 consecutive timeouts** | Treated as a disconnect (D.5); a voluntary action resets the streak. |
| Clock skew / display | Server-synced deadline; never shows negative (clamped → "resolving…"). |

## D.5 — Connection & lifecycle (no rejoin in V1 — but always graceful)
| Situation | Response |
|---|---|
| **You** lose connection | Non-blocking "Connection lost — reconnecting…" overlay + short countdown; on timeout → "Disconnected" → Main Menu (server already dropped you). |
| **Another player** drops | Seat dims / figure slumps, badge → "left"; toast "{Name} left the table"; removed from turn order; if it was their turn, play advances. |
| Drop **while you're mid-hold-to-doubt on them** | Your hold cancels; doubt window closes; claim clears; "{Name} left — their claim is void." |
| The player who must **take the pile** drops mid-resolution | The pile is discarded (consistent with their hand being discarded); resolution completes. |
| Drop reduces the table to **1 player** | That player wins (last player standing) → post-game. |
| **Host** drops | Session ends for everyone → "Host left — session ended" → Main Menu (no host migration in V1). |
| Join fails | Specific, actionable message: "Lobby full" / "Not found" / "Game already started — ask them to finish" / "Version mismatch". Never silent. |
| Window loses focus (alt-tab) | Game keeps simulating (Run In Background on; server-authoritative). On refocus the UI already matches replicated state — no desync. |

## D.6 — Input robustness
| Situation | Response |
|---|---|
| Rapid clicks / double-submit | Pending state disables the control on first send; extra inputs ignored. |
| Animation interrupted by a state change (e.g. a drop mid-reveal) | Cancel/skip the animation, **snap to authoritative state**, continue. |
| Resize / ultrawide / **Steam Deck** (1280×800) | UMG anchors + CommonUI scaling; TV-safe margins; controller-sized hit-targets. |
| Gamepad-only (no mouse) | Full focus navigation; every action reachable; gamepad glyphs shown. |
| **Free-look while acting** | Look never blocks input — hand/buttons are screen-anchored; you can select cards while looking. |
| **Look during a cinematic beat** (reveal) | Player look is **suspended**; the camera plays the scripted push, then returns control + recenters. |
| **Motion sickness from look** | A **reduce-look** setting (V2) lowers sensitivity / offers a static option; reduce-motion covers scripted moves. |

## D.7 — Menu, lobby & cosmetic
| Situation | Response |
|---|---|
| Join a game **already in progress** | Blocked: "Game already started — ask them to finish" (no mid-game join in V1). |
| **Lobby full** (8/8) on join | "Lobby full." |
| Host **leaves the Waiting Room** | Lobby ends for everyone → "Host left — lobby closed" → Main Menu. |
| A non-host **leaves the Waiting Room** | Their slot frees; Start gating recomputes; others unaffected. |
| Host presses **Start** as someone un-readies (race) | The server re-checks "all ready & ≥2" at Start; if not, Start is rejected with the reason (never a half-started game). |
| **Version mismatch** on join | "Version mismatch — update to play together." |
| Settings/help open on your turn | The shared **timer keeps running**; a subtle "your turn — timer running" reminder shows; closing returns you in-context. |
| **Emote spam** *(V2)* | Per-player cooldown + server rate-limit; extra presses are ignored (no flooding). |
| Emote from a **disconnected** player *(V2)* | Ignored server-side; never plays. |

---
---

# PART E — ART DIRECTION

> **V1 = greyboxed prototype** (mechanics-first, minimal art). **V2 = full visual identity** on Unreal's strengths. The
> engine choice exists for this: *more graphics for less effort.* The rendering-tech rationale (Lumen/Nanite/Niagara,
> Fab, MetaHuman) lives in `ARCHITECTURE.md`; here is the *direction*.

## Concept & setting — The Infernal Tavern
Damned souls gamble with cards in Hell, surrounded by fire — the setting is the moral verdict for a game of liars.

| Element | Description |
|---|---|
| Walls | Dark cracked stone, glowing lava veins |
| Floor | Black rock, ember patches |
| Lighting | Fire torches, candles, lava glow — no daylight ever |
| Table | Heavy scorched wood, cards, wax drippings |
| Atmosphere | Dense smoke, floating embers, distant screams (ambient) |
| Palette | Deep blacks, burnt oranges, blood reds, sulfur yellows |

- **V1:** a **greybox** — primitive/boxed walls/floor, a simple table, a few warm point lights (Lumen already sells a dim
  fire-lit room). Layout & readable seating only, no dressing.
- **V2:** Fab/Megascans modular stone + props (barrels, banners, wax, tankards), torch/brazier fire sources, Niagara
  fire/embers, a warm post-process grade. Same layout — dressed.

## Camera — free-look first-person, seated
Seated at the table, eye-level. The **rest framing** shows the whole table (pile + opposite players) with your hand at
the bottom — but you can **freely look around** (mouse / right-stick) within clamped yaw/pitch: pan across the table,
**stare down a suspected liar**, glance at a specific opponent. As in our reference *Liar's Bar*, this look-around is a
real **social/bluff layer** (a tell, a taunt, a mind-game), not just flavour. You cannot stand, walk, or leave the seat.
**Recenter** returns to the rest framing (a bindable button + a gentle auto-recenter after a few idle seconds). Cinematic
beats (the doubt reveal) temporarily take camera control (§C.3) and then hand it back. The **HUD is screen-space**, so it
stays put and fully reachable while you look. Own hand: 3D cards, face-up, lower screen (owner-only). Pile: center,
face-down. Rest framing:
```
     [P3]   [P4]   [P5]            other players across the table
 [P2]      INFERNAL TAVERN   [P6]
 [P1]      [ CENTER PILE ]   [P7]
 ─────────────────────────────────
   ♠7  ♥K  ♣3  ♦A  ♠2   ← YOUR HAND (face up)
```
HUD layered on top per the **hybrid** model (§C.1): world carries context (seat glow, pile+count, badges, claim bubble,
round token); a tiny pinned overlay adds only the claim-to-judge + your action bar (+ turn/timer chip). At rest ≈ pure world.

## Characters
- **V1:** one simple seated placeholder per occupied seat (Unreal mannequin/basic figure) + a floating name label. Readable,
  not pretty. One shared model, differentiated by name + count + seat colour.
- **V2:** damned souls as **MetaHumans** — distinct silhouettes (hooded figure, corrupt noble, fallen angel…), seated,
  reacting to events: caught lying → recoil/look away; successful doubt → triumphant point; penalty → slump; win → stand, arms raised.

### Social expression — emotes & gestures *(designed now, built in V2)*
A bluffing game is a social game — our reference (Liar's Bar) leans hard on pointing, laughing, and staring. We **design
the system now** (so it's foreseen and the rig accounts for it) and **ship it in V2**; V1 stays mechanics-first, where the
only social channel is **free-look / staring**.
- **Emote wheel** (a bindable radial), a small curated set that fits the infernal table and the mind-game: **Point /
  Accuse**, **Laugh / Mock**, **Knock the table** (impatience), **Stare** (a slow lean-in). 4–6 max; **no free chat**.
- **How it works:** pick from the wheel → a short **character gesture** plays on your V2 MetaHuman + a light sound, visible
  to all. **Purely cosmetic** — it never touches game state (a taunt is a bluff tool, not a mechanic).
- **Networking (V2):** `Server_Emote(id)` → `Multicast_Emote` (cosmetic-only), **rate-limited + per-player cooldown**
  server-side to stop spam (Part D.7); ignored for absent players.
- **Why it earns its place:** cheap on top of the MetaHuman reaction rig, and it deepens the psychology that makes bluffing
  fun — a false accusation or a smug laugh *is* gameplay, socially.
- **Accessibility:** sending is optional; a receiver can **mute/hide** others' emotes via a settings toggle.

### Player names & colours
V1 assigns a predefined name **and a fixed seat colour** (colour used for the seat glow/turn highlight and the lobby row —
accessibility, so "who" is unambiguous before Steam avatars in V2).

| Seat | Name | | Seat | Name |
|---|---|---|---|---|
| 1 | Mortem | | 5 | Malachar |
| 2 | Lucian | | 6 | Seraph |
| 3 | Vesper | | 7 | Dante |
| 4 | Cain | | 8 | Abyssia |

V2: Steam display name + avatar.

## Cards
- **V1:** a 3D card pack (realistic standard cards) — the base for all card mechanics. One `ACardActor`: thin mesh, a
  **face** material set at runtime (Material Instance Dynamic) + a shared **uniform back**. A **Data Table** maps
  `(ESuit, Value 1–13) → face texture` (no runtime scans). All 52 faces distinct; identical back (critical for bluffing).
  Source a free/cheap pack from **Fab**.
- **V2:** custom illustrated demonic face cards (Ace of Spades = death, King = demon lord…); same uniform back.

## Audio direction (V1 placeholders → V2 full)
| Event | V1 | V2 |
|---|---|---|
| Card placed | Generic "card place" | Heavy thud + distant fire crackle |
| Doubt called | UI sting | Dramatic sting, crowd murmur |
| Doubt correct (liar caught) | Short negative tone | Demonic laugh, fire burst |
| Doubt wrong | Short positive tone | Groan, pile slide |
| Win | Simple jingle | Triumphant fanfare, crowd cheer |
| Pile discarded | Whoosh | Cards burning, fire roar |
| Turn timeout / auto-play | Soft tick + neutral place | Ticking-clock ramp + resigned sigh |
| Background | Silence | Ambient hell — low fire roar, distant wails, glass clinks |

**V1 SFX list (6 clips):** `card_place`, `card_flip`, `doubt_call`, `doubt_correct`, `doubt_wrong`, `win` (`.wav`
preferred; SoundCue/SoundWave in V1, MetaSounds for V2 dynamic ambience — see `ARCHITECTURE.md`).

---
---

# PART F — OPEN DECISIONS (validate in playtest)
- **Claim bubble vs pinned claim only** — is the in-world bubble worth it, or noise? Cut it if it adds clutter.
- **Timer length (45 s)** — tune after real games; may differ for 2 vs 8 players.
- **Reveal auto-continue delay** — long enough to read, short enough to keep pace.
- **Resting-screen richness** — keep the round token in-world always, or only on hover?
- **First-run hint depth** — light stub first, fuller polish later (Phase 7).

## References
Liar's Bar (simple rules + deep bluff; fixed rank per round) · Balatro (zone discipline, colour-coded reading, juice-as-
design) · Norman (affordances/signifiers) · Portal-2 (teach one idea at a time) · modern game UI/UX & motion-design 2025
(contextual minimalism, timing/easing) · diegetic-vs-non-diegetic UI (the minimal-HUD paradox).
