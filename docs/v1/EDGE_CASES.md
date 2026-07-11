# Dubito V1 Edge Cases

Read this file only when implementing, testing, or reviewing rules/UX behavior.

`docs/DESIGN.md` owns the rule contract. This file expands expected responses.

## Turn And Phase

| Case | Expected response |
|---|---|
| Player acts out of turn | Reject, resync from server, show reason |
| Player acts during reveal | Reject, controls disabled |
| Player starts Play while Doubt is available | Doubt window is forfeited |
| Player starts Discard while Doubt is available | Doubt window is forfeited |
| Pile empty on active turn | Play only; no Doubt, no Discard |
| Pile non-empty with no doubtable claim | Play and Discard only |
| Pending win | Doubt emphasized, Discard blocked |
| Game over | All gameplay inputs disabled |

## Play And Claim

| Case | Expected response |
|---|---|
| Select 0 cards | Play disabled |
| Select more than 4 cards | Refuse extra selection with reason |
| Claim count below 1 or above 4 | Confirm disabled |
| Open empty pile | Player chooses value and count |
| Follow open round | Value fixed to round value; only count editable |
| Claim count differs from actual selected count | Legal |
| Actual card value differs from round value | Legal play, but vulnerable to correct Doubt |
| Player has no card matching locked value | Player may still play a bluff |
| Public hand ledger would go below zero | Clamp visible ledger at zero and keep unverified marker |

## Hidden Count Presentation

| Case | Expected response |
|---|---|
| Opponent plays 1 card claiming 4 | Others see neutral packet and claimed ledger change only |
| Opponent plays 4 cards claiming 1 | Others see neutral packet and claimed ledger change only |
| Pile mesh grows | Growth must reflect claimed stake or abstract state, never exact hidden count |
| Opponent hand badge changes | Badge is public ledger, visually unverified |
| Own hand changes | Owner sees exact cards/count |
| Pending win appears | It is authoritative actual server state |
| Public ledger says zero without pending win | UI must imply "claimed zero", not confirmed empty |

## Doubt

| Case | Expected response |
|---|---|
| Doubt previous honest play | Doubter takes pile and loses turn |
| Doubt previous count lie | Liar takes pile; doubter plays |
| Doubt previous value lie | Liar takes pile; doubter plays |
| Doubt both count and value lie | Liar takes pile; doubter plays |
| Doubt after already playing/discarding | Reject; window closed |
| Non-immediate player doubts | Reject |
| Previous claimant disconnected | Claim is no longer doubtable |
| Doubt reveal interrupted by newer state | Presentation snaps to authoritative result |

## Discard

| Case | Expected response |
|---|---|
| Discard empty pile | Disabled |
| Discard during pending win | Disabled |
| Discard after choosing not to Doubt | Legal if pile non-empty; forfeits Doubt |
| Discard succeeds | Hidden pile removed, claimed pile ledger reset, round value cleared, turn skipped |
| Discard while request pending | Controls stay pending/disabled until server response |

## Timer

| Case | Expected response |
|---|---|
| Timeout with Doubt available | Decline Doubt, then auto-play |
| Timeout on empty pile | Auto-play one card and truthfully open round |
| Timeout with matching locked-value card | Auto-play one matching card, claim count 1 |
| Timeout without matching locked-value card | Auto-play one card as forced minimal bluff, claim count 1 |
| Timeout during pending win response | Confirms opponent win |
| Timeout during reveal | Timer paused; no auto-action |
| 3 consecutive timeouts | Treat as disconnect |
| Voluntary action after timeouts | Reset timeout streak |

## Winning

| Case | Expected response |
|---|---|
| Player plays last actual card | Pending win, next player can Doubt |
| Next player doubts correctly | Would-be winner takes pile; game continues |
| Next player doubts wrongly | Winner confirmed |
| Next player plays or times out | Winner confirmed |
| Next player disconnects during pending win | Winner confirmed |
| Last remaining player | Wins automatically |

## Connection And Session

| Case | Expected response |
|---|---|
| Player disconnects before match start | Remove from lobby |
| Player disconnects during match | Remove hand from game and turn order |
| Active player disconnects | Advance turn |
| Host disconnects | End session for everyone |
| Join lobby full | Show clear error |
| Join match already started | Show clear error |
| Steam unavailable | Show offline/local test path if available |

## Input And UX Robustness

| Case | Expected response |
|---|---|
| Modal open | Underlying gameplay controls blocked |
| Confirm panel canceled | Return to previous selection/action state |
| Controller focus lost | Restore focus to primary legal action |
| Window resized | Critical text and action controls remain visible |
| Reduce motion enabled | No cinematic camera push; reveal remains readable |
| User spams action button | One pending server request at a time |

## Test Priority

Must be covered before V1 release:

- count bluff hidden from non-owners;
- owner-only exact hand state;
- right and wrong Doubt;
- pending-win Doubt window;
- timeout forced minimal bluff;
- host leave;
- packaged two-instance full game;
- Steam two-machine full game.
