---
name: dubito-mirror-steam
description: Implémente et vérifie le réseau de Dubito (Mirror, messages du protocole, association connexion → siège, vues privées par joueur, lobby et invitations Steam avec Steamworks.NET et FizzySteamworks). À utiliser pour les phases 3 et 4 de la ROADMAP et pour tout code qui envoie ou reçoit des données de partie.
---

# Réseau Mirror + Steam

Architecture et flux : `docs/ARCHITECTURE.md` §3 et §5. Règles de départ et d'ordre : `RULES §7–8`. Confidentialité : `RULES §9` et cas `C-PRV-*`.

## Principes non négociables

Les principes (hôte arbitre, siège déduit de la connexion, vue filtrée par joueur, pas d'objet réseau par carte, identifiant + révision par commande) sont dans ARCHITECTURE §3. En pratique avec Mirror :

- Échanges en `NetworkMessage` (pas de `SyncVar` d'état de partie) ; siège retrouvé depuis la `NetworkConnectionToClient` qui envoie.
- La vue de chaque destinataire est construite **avant la sérialisation**, y compris pour le client local du joueur hôte.
- Valider toute donnée reçue (taille, valeurs, siège) avant de la passer à Core.

## Versions

- Mirror, FizzySteamworks et Steamworks.NET sont **figés** et notés dans ARCHITECTURE §1 à l'installation. Vérifier qu'aucune dépendance n'est installée deux fois (Steamworks.NET est requis par Fizzy).
- Installation d'un package = accord du propriétaire (`AGENTS.md` règle 8).

## Ordre de travail (ROADMAP)

1. **Phase 3, sans Steam** : transport KCP sur `localhost`. Plusieurs instances sur un PC : éditeur + builds, ou Multiplayer Play Mode d'Unity 6 si validé. ParrelSync n'est garanti que jusqu'à Unity 2022.
2. **Phase 4, Steam** : App ID 480 au début, lobbies filtrés par une clé Dubito + version du protocole. Lobby ≠ connexion : après l'invitation, le client démarre Mirror vers l'identité Steam de l'hôte. Steam initialisé une fois, callbacks à chaque image, arrêt propre. `steam_appid.txt` jamais livré.

## Vérification

- Tests `C-PRV-*` sur les **vrais messages sérialisés** : un client ne reçoit aucune valeur hors de sa main et des révélations.
- Tests de départ (`C-DEC-*`) avec plusieurs instances.
- Steam : deux PC, deux comptes, deux réseaux (le propriétaire fait le test).
- Puis `dubito-cross-review` (réseau et confidentialité sont toujours « importants »).
