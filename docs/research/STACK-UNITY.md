# Recherche : stack Unity + Steam

> Recherche Codex du **2026-11-26**. Les tarifs et versions changent : revérifier avant de s'y fier.
> Décision prise : voir `DECISIONS D-14, D-20`.

## Version Unity

- **6.3 LTS** : support standard jusqu'en décembre 2027. Correctif vérifié : 6000.3.29f1 (19/11/2026). [Support Unity](https://unity.com/releases/unity-6/support)
- 6.0 LTS : support terminé en octobre 2026. 6.6 : pas LTS. 6.7 LTS : pas encore sortie.

## Réseau : comparaison

| Solution | Steam + hôte joueur | Coût | Verdict pour Dubito |
|---|---|---|---|
| **Mirror + FizzySteamworks** | Listen server natif, Fizzy utilise Steamworks.NET ; lobbies à brancher soi-même | Gratuit | **Retenu** : simple, mature, bien connu des IA |
| Netcode for GameObjects + Multiplayer Services | Mode Host ; Sessions/Relay/Lobby ; pont Steam en plus | Gratuit sous quotas UGS (50 CCU moyens/mois pour Relay) | **Repli**, surtout si on sort de Steam |
| FishNet + FishySteamworks | Host, Steamworks.NET | Gratuit (Pro optionnel) | Solide, moins connu des IA |
| Photon Fusion 2 | Host Mode, réseau Photon | 100 CCU gratuits puis payant | Trop de concepts pour un jeu de cartes |
| Photon PUN 2 | Master Client | Photon Cloud | En maintenance, à éviter |
| PurrNet | Host, transport Steam intégré | Cœur MIT gratuit | Jeune, familiarité IA incertaine |
| Messages maison sur Steam | Adapté | Gratuit | Simple à dessiner, dur à fiabiliser en solo |

Sources : [Mirror](https://github.com/MirrorNetworking/Mirror), [FizzySteamworks](https://github.com/Chykary/FizzySteamworks), [Multiplayer Services](https://docs.unity.com/en-us/mps-sdk/subservices-toc), [tarifs UGS](https://unity.com/products/gaming-services/pricing), [tarifs Photon](https://www.photonengine.com/fusion/pricing), [Valve multijoueur](https://partner.steamgames.com/doc/features/multiplayer).

**Steamworks.NET plutôt que Facepunch.Steamworks** : Fizzy et Fishy en dépendent.

## Tests locaux à plusieurs

- Mirror : éditeur + builds locaux. ParrelSync (plusieurs éditeurs) n'est garanti que jusqu'à Unity 2022 : vérifier avant de l'adopter. [ParrelSync](https://github.com/VeriorPies/ParrelSync)
- Aucun de ces outils ne crée plusieurs comptes Steam : le test Steam se fait sur 2 PC.

## Architecture conseillée

Moteur de règles en C# pur (asmdef `No Engine References`), protocole séparé, réseau et présentation fins. Tests NUnit, et les mêmes sources compilées dans un projet `dotnet test`. Garder .NET Standard 2.1 / C# 9. [asmdef](https://docs.unity.com/en-us/engine/6000.7/manual/programming-environment/script-compilation/assembly-definition-files/class-assembly-definition-importer), [tests en ligne de commande](https://docs.unity.com/en-us/engine/6000.3/manual/programming-environment/test-framework-introduction/running-tests/run-tests-from-command-line)

## Pièges Steam

- App ID 480 (Spacewar) pour prototyper ; lobbies partagés, donc filtrer par une clé Dubito + version du protocole.
- `steam_appid.txt` : développement seulement, jamais livré.
- Lobby ≠ connexion : après l'invitation, démarrer le client Mirror vers l'hôte.
- Tester sur 2 PC, 2 comptes, 2 réseaux. [Lobbies Valve](https://partner.steamgames.com/doc/features/multiplayer/matchmaking)
