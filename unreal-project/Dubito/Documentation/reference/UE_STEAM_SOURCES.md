# UE And Steam Source Notes

Checked July 11, 2026. Prefer official sources when implementing V1.

This file is reference material. Agents should read it only for Unreal/Steam decisions or verification.

## Unreal UI

- [Common UI](https://dev.epicgames.com/documentation/en-us/unreal-engine/common-ui-plugin-for-advanced-user-interfaces-in-unreal-engine):
  supports layered UI, input routing, controller icons, and gamepad navigation. This supports the V1 menu/modal choice.
- [UMG Viewmodel](https://dev.epicgames.com/documentation/en-us/unreal-engine/umg-viewmodel-for-unreal-engine):
  documented as beta. V1 therefore does not depend on MVVM.
- CommonUI plus Enhanced Input has been documented by Epic as experimental in UE 5.x release notes. V1 separates CommonUI
  UI actions from Enhanced Input gameplay actions unless the installed engine proves the integration stable.

## Unreal Networking

- [Remote Procedure Calls](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-procedure-calls-in-unreal-engine):
  server RPC validation can disconnect clients on failed validation. Use it for malformed/abusive payloads, not normal
  rule rejection.
- [Property replication](https://dev.epicgames.com/documentation/en-us/unreal-engine/replicate-actor-properties-in-unreal-engine):
  replication conditions include owner-only replication. Use this for exact private hands.
- [Travelling in multiplayer](https://dev.epicgames.com/documentation/en-us/unreal-engine/travelling-in-multiplayer-in-unreal-engine):
  Epic recommends seamless travel when possible after initial connection; server travel moves connected clients.
- [Play In Editor Multiplayer Options](https://dev.epicgames.com/documentation/unreal-engine/play-in-editor-multiplayer-options-in-unreal-engine):
  PIE can spawn multiple networked worlds for fast local multiplayer iteration. Play As Listen Server lets the editor act
  as both server and client, and additional clients can be opened from the player count.
- [Testing Multiplayer](https://dev.epicgames.com/documentation/unreal-engine/testing-multiplayer-in-unreal-engine):
  Unreal can run multiple multiplayer windows in one editor process for speed, or launch separate instances when the
  single-process shortcut is disabled. Separate instances are slower but more representative of runtime process behavior.
- [Testing and Debugging Networked Games](https://dev.epicgames.com/documentation/unreal-engine/testing-and-debugging-networked-games-in-unreal-engine):
  PIE listen-server/client modes can auto-connect clients locally by IP. This is useful for gameplay testing, but does not
  validate session discovery, lobby search, or invite behavior by itself.
- [Command-Line Arguments](https://dev.epicgames.com/documentation/unreal-engine/command-line-arguments-in-unreal-engine):
  standalone and packaged instances can receive launch parameters, which is useful for local two-window testing on one PC.
- PIE multiplayer supports local listen-server testing and remains the daily network validation path before packaged local
  and real Steam tests.

## Steam And OSS

- [Online Subsystem Steam](https://dev.epicgames.com/documentation/en-us/unreal-engine/online-subsystem-steam-interface-in-unreal-engine):
  official Unreal route for Steam features. AppID 480 is the shared test AppID; real AppID is required for shipping.
- The same OSS Steam documentation notes Steam lobbies for listen-server games and the session flags needed for lobby use.
- It also notes that `bInitServerOnClient` is needed when using Steam sessions.
- `steam_appid.txt` is for local development/testing and should not be included in Steam images/depots.
- [Steam Datagram Relay](https://partner.steamgames.com/doc/features/multiplayer/steamdatagramrelay):
  Steam P2P can use Steam relay/routing. V1 relies on OSS Steam rather than a separate third-party networking provider.
- [Steam Direct fee](https://partner.steamgames.com/doc/gettingstarted/appfee):
  the fee is recoupable after the product reaches at least $1,000 adjusted gross revenue.

## V1 Implications

- Keep V1 first-party by default.
- CommonUI is accepted for V1 because it is built into Unreal and directly supports layered, controller-friendly UI.
- Do not add Advanced Sessions for V1 unless the owner explicitly approves a documented OSS Steam blocker.
- Defer MVVM until after V1, or adopt only if the installed engine version and project needs justify it.
- Use the local multiplayer test ladder before Steam: PIE listen server, separate-process or standalone checks when useful,
  packaged two-instance test on one PC, then Steam two-machine validation.
- Do not select external asset sources in this reference file; assets are owner-selected/imported later when a phase needs
  them.
- Avoid designing around dedicated servers, cross-platform identity, or backend matchmaking in V1.
