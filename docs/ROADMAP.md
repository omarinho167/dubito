# Dubito : feuille de route

> Petites étapes, chacune vérifiable. On ne passe à l'étape suivante que quand le **critère de fin** est rempli **et validé par le propriétaire**.
> L'avancement est noté ici (case cochée + date). Le détail de ce qui a été fait va dans les commits, pas dans ce fichier.

## Phase 0 : conception

- [x] 0.1 Récupérer les anciennes docs, choisir la version des règles (juillet 2026). *(2026-11-26)*
- [x] 0.2 Trancher les trous de règles et le périmètre V1 avec le propriétaire. *(2026-11-26)*
- [x] 0.3 Recherches : stack Unity/Steam, concurrence. *(2026-11-26)*
- [ ] 0.4 Relecture de la nouvelle doc par le propriétaire.

**Fin de phase** : le propriétaire valide RULES, CASES, UX, VISION et ARCHITECTURE.

## Phase 1 : projet Unity et moteur de règles

- [ ] 1.1 Créer le projet Unity 6.3 LTS (URP) dans `unity/`, les asmdefs et le `.gitignore` ; s'assurer qu'il compile.
- [ ] 1.2 Core : cartes, paquet, distribution, ordre du tour (`RULES §3`). Tests `C-SET-*`.
- [ ] 1.3 Core : Jouer, Douter, Défausser (`RULES §4`). Tests `C-PLA-*`, `C-DOU-*`, `C-DEF-*`.
- [ ] 1.4 Core : chrono, tours sautés, exclusion (`RULES §5`). Tests `C-CHR-*`.
- [ ] 1.5 Core : victoire, déconnexion, ordre de résolution (`RULES §6–8`). Tests `C-VIC-*`, `C-DEC-*`.
- [ ] 1.6 Core : vues filtrées par joueur (`RULES §9`). Tests `C-PRV-*` + invariants.
- [ ] 1.7 Projet `dotnet test` hors Unity sur les mêmes sources.

**Fin de phase** : tous les cas de CASES.md passent en test automatique. Aucun visuel nécessaire.

## Phase 2 : table 3D basique, en local sur une seule machine

- [ ] 2.1 Scène Table : table ronde, 2 à 6 sièges, caméra 1re personne avec regard limité (`UX §2`).
- [ ] 2.2 Cartes 3D : sa main en éventail (face visible), mains adverses de dos, tas au centre.
- [ ] 2.3 Sélection des cartes au clic, panneau d'action, Douter maintenu, Défausser confirmé (`UX §4`).
- [ ] 2.4 Mode **debug « hot-seat »** : une seule instance, on contrôle chaque siège à tour de rôle (sans réseau) pour tester les règles en 3D.
- [ ] 2.5 Révélation du doute, victoire en attente, fin de partie, tour sauté (`UX §5`).

**Fin de phase** : le propriétaire joue une partie complète à 3 sièges en hot-seat.

## Phase 3 : réseau local (Mirror, sans Steam)

- [ ] 3.1 Protocol : commandes, vues, erreurs, version.
- [ ] 3.2 Hôte + clients en `localhost` ; association connexion → siège.
- [ ] 3.3 Chaque client ne reçoit que sa vue (vérification `C-PRV-*` sur les vrais messages).
- [ ] 3.4 Salon local : Prêt, réglages de l'hôte (chrono, places max), lancer, revanche, retour au menu (`UX §6`).
- [ ] 3.5 Déconnexions et départ de l'hôte (`RULES §7`).

**Fin de phase** : partie complète à 3 instances sur un PC (éditeur + 2 builds).

## Phase 4 : Steam

- [ ] 4.1 Steamworks.NET + FizzySteamworks, App ID 480, filtre de lobby Dubito.
- [ ] 4.2 Créer un salon, inviter un ami via l'overlay, rejoindre, noms Steam.
- [ ] 4.3 Test sur deux PC, deux comptes, deux réseaux différents.

**Fin de phase** : partie complète entre deux PC via invitation Steam.

## Phase 5 : V1 jouable

- [ ] 5.1 Menus, écran Règles, options (volume, sensibilité, réduction des animations), messages d'erreur (`UX §6`).
- [ ] 5.2 Sons simples (`UX §7`).
- [ ] 5.3 Personnages stylisés simples (placeholders) aux places.
- [ ] 5.4 Playtests avec des amis, réglage des points `RULES §11`.

**Fin de phase** : 3 parties complètes à 4+ joueurs sans bug bloquant.

## Plus tard

Direction artistique, puis les idées de `RULES §10` et `VISION` (« Pour plus tard »).
