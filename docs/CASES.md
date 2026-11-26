# Dubito : cas limites (V1)

> Scénarios numérotés « situation → action → résultat attendu ». Chaque cas renvoie à la règle de [RULES.md](RULES.md).
> **Chaque cas est destiné à devenir un test automatique** du moteur de règles. Le nom du test reprend l'identifiant (ex. `C-DOU-03`).
> Si un cas contredit RULES.md, c'est RULES.md qui a raison : corriger ce fichier.

Notation : J1, J2, J3… dans l'ordre du tour. « Tas : 5 (valeur 7) » = 5 cartes, valeur de manche 7.

## Mise en place (SET)

| ID | Situation | Action | Résultat attendu | Règle |
|---|---|---|---|---|
| C-SET-01 | 2 joueurs | Distribution | 26 et 26 cartes | §3 |
| C-SET-02 | 5 joueurs, ordre J1…J5 | Distribution | J1 et J2 : 11 cartes. J3, J4, J5 : 10 cartes | §3 |
| C-SET-03 | 6 joueurs | Distribution | J1 à J4 : 9 cartes. J5, J6 : 8 cartes | §3 |
| C-SET-04 | Toute partie | Distribution | Total des mains = 52, aucune carte en double | §3 |
| C-SET-05 | Début de partie | — | Tas vide, pas de valeur de manche, J1 actif, chrono 45 s | §3 |
| C-SET-06 | 1 joueur ou 7 joueurs dans le salon | L'hôte lance | Refusé : 2 à 6 joueurs | §1 |
| C-SET-07 | L'hôte a choisi 60 s | Lancement | Chaque tour dure 60 s ; la durée ne peut plus changer | §5 |
| C-SET-08 | L'hôte propose un chrono de 50 s | — | Refusé : seulement 30, 45, 60 ou 90 s | §5 |

## Jouer (PLA)

| ID | Situation | Action | Résultat attendu | Règle |
|---|---|---|---|---|
| C-PLA-01 | J1 actif, tas vide | Pose 2 cartes, annonce « 2 × Dame » | Tas : 2 (valeur D), J2 actif | §4.1 |
| C-PLA-02 | Tas vide | Annonce une valeur sans en avoir aucune en main | Accepté (ouverture libre) | §4.1 |
| C-PLA-03 | Tas : 2 (valeur D) | J2 pose 1 carte annoncée « 1 × 5 » | Refusé : l'annonce doit être la valeur de manche | §4.1 |
| C-PLA-04 | Tas : 2 (valeur D) | J2 pose 3 cartes (un 4, un 9, une D) | Accepté, annonce « 3 × Dame ». Tas : 5 | §4.1 |
| C-PLA-05 | Joueur actif | Pose 0 carte | Refusé | §4.1 |
| C-PLA-06 | Joueur actif | Pose 5 cartes | Refusé | §4.1 |
| C-PLA-07 | Joueur actif | Pose une carte qu'il n'a pas en main | Refusé, resynchronisation | §8 |
| C-PLA-08 | Joueur actif | Pose deux fois la même carte | Refusé | §8 |
| C-PLA-09 | J3 pas actif | J3 envoie Jouer | Refusé, resynchronisation, message « ce n'est pas ton tour » | §8 |
| C-PLA-10 | Le même message Jouer arrive deux fois | Second message | Ignoré (même identifiant de commande) | §8 |
| C-PLA-11 | Joueur avec 2 cartes, 2 cartes posées | — | Victoire en attente | §6 |

## Douter (DOU)

| ID | Situation | Action | Résultat attendu | Règle |
|---|---|---|---|---|
| C-DOU-01 | J1 a ouvert « 2 × 7 » avec deux 7. J2 actif | J2 doute | Révélation : doute faux. J2 ramasse le tas (2 cartes), tas vide, valeur effacée. J3 actif, ouvre | §4.2 |
| C-DOU-02 | J1 a ouvert « 2 × 7 » avec un 7 et un 3. J2 actif | J2 doute | Doute juste. J1 ramasse le tas. J2 reste actif, chrono 45 s, doit ouvrir | §4.2 |
| C-DOU-03 | Tas : 6 (valeur R), dernier coup de J2 = 2 cartes dont un 8 | J3 doute | Seules les 2 cartes du dernier coup sont révélées. Doute juste : J2 ramasse les 6 cartes | §4.2 |
| C-DOU-04 | 2 joueurs, J1 a posé honnêtement | J2 doute | Doute faux : J2 ramasse, J2 perd son tour, J1 actif et ouvre | §4.2 |
| C-DOU-05 | Tas vide | Le joueur actif doute | Refusé : rien à douter | §4.2 |
| C-DOU-06 | J1 a posé, J2 actif | J3 doute | Refusé : seul le joueur actif peut douter | §4.2 |
| C-DOU-07 | J2 a sélectionné des cartes sans confirmer | J2 doute | Accepté : sélectionner n'est pas confirmer | §4.2 |
| C-DOU-08 | J2 a confirmé Jouer | J2 envoie Douter (même tour) | Refusé : le tour est déjà joué | §4.2 |
| C-DOU-09 | J1 a posé, J2 a été sauté (temps écoulé), J3 actif | J3 doute | Refusé : le dernier coup n'est pas celui du joueur juste avant J3 | §5 |
| C-DOU-10 | Doute en cours (révélation) | N'importe quel joueur envoie une action | Refusé. Chrono en pause | §5, §8 |
| C-DOU-11 | Doute juste, le douteur rouvre | — | Il peut choisir n'importe quelle valeur, y compris la précédente | §4.1 |
| C-DOU-12 | 2 joueurs. J1 pose, J2 sauté, J1 sauté, J2 de nouveau actif | J2 doute | Refusé : la fenêtre du coup de J1 s'est fermée quand J2 a été sauté | §2 |
| C-DOU-13 | 3 joueurs. J1 pose, J2 (répondant) se déconnecte avant d'agir, J3 actif | J3 doute | Refusé : le répondant est parti, la fenêtre est fermée | §2, §7 |

## Défausser (DEF)

| ID | Situation | Action | Résultat attendu | Règle |
|---|---|---|---|---|
| C-DEF-01 | Tas : 7 (valeur 9), J3 actif | J3 défausse | Tas retiré du jeu sans révélation, valeur effacée, J4 actif et ouvre | §4.3 |
| C-DEF-02 | Tas vide | Défausser | Refusé | §4.3 |
| C-DEF-03 | Coup doutable de J2, J3 actif | J3 défausse | Accepté. Le coup de J2 n'est plus doutable | §4.3 |
| C-DEF-04 | Victoire en attente de J2, J3 répond | J3 défausse | Refusé : seuls Douter ou Jouer sont possibles | §6 |
| C-DEF-05 | Après une défausse | — | Nombre total de cartes en main + retirées du jeu = 52 | §4.3 |

## Chrono et inactivité (CHR)

| ID | Situation | Action | Résultat attendu | Règle |
|---|---|---|---|---|
| C-CHR-01 | J2 actif, coup doutable de J1 | 45 s sans action | Tour de J2 sauté. Tas inchangé. J3 actif, pas de coup doutable | §5 |
| C-CHR-02 | J1 actif, tas vide | 45 s sans action | Tour sauté, J2 actif, tas toujours vide | §5 |
| C-CHR-03 | J2 a déjà été sauté au tour précédent | Encore 45 s sans action | J2 exclu (main retirée du jeu), suite comme §7 | §5 |
| C-CHR-04 | J2 sauté, puis joue normalement, puis sauté | — | Pas exclu : le compteur a été remis à zéro | §5 |
| C-CHR-05 | Révélation en cours | Le temps passe | Chrono en pause | §5 |
| C-CHR-06 | Doute juste de J2 avec 10 s restantes | Fin de la révélation | J2 repart à 45 s | §5 |
| C-CHR-07 | L'action de J2 arrive à l'hôte à 44,9 s | — | Acceptée | §8 |
| C-CHR-08 | L'action de J2 arrive après expiration côté hôte | — | Refusée, le tour a déjà été sauté | §8 |

## Victoire (VIC)

| ID | Situation | Action | Résultat attendu | Règle |
|---|---|---|---|---|
| C-VIC-01 | J2 pose ses dernières cartes honnêtement | J3 doute | Doute faux : J2 gagne | §6 |
| C-VIC-02 | J2 pose ses dernières cartes en mentant | J3 doute | Doute juste : J2 ramasse le tas, la partie continue, J3 joue | §6 |
| C-VIC-03 | Victoire en attente de J2 | J3 confirme Jouer | J2 gagne. La pose de J3 n'est pas appliquée | §6 |
| C-VIC-04 | Victoire en attente de J2 | J3 laisse passer le temps | J2 gagne | §6 |
| C-VIC-05 | Victoire en attente de J2 | J3 se déconnecte | J2 gagne | §6, §7 |
| C-VIC-06 | Victoire en attente de J2 | J2 se déconnecte | Victoire annulée, J2 retiré, J3 actif sans coup doutable | §7 |
| C-VIC-07 | 3 joueurs, deux partent | — | Le dernier gagne immédiatement | §6 |
| C-VIC-08 | Partie terminée | N'importe quelle action de jeu | Refusée | §6 |

## Déconnexion (DEC)

| ID | Situation | Action | Résultat attendu | Règle |
|---|---|---|---|---|
| C-DEC-01 | J2 actif | J2 se déconnecte | Main de J2 retirée du jeu, J3 actif, pas de coup doutable, tas inchangé | §7 |
| C-DEC-02 | J1 a posé, J2 actif | J1 se déconnecte | Le coup de J1 n'est plus doutable. J2 peut Jouer ou Défausser | §7 |
| C-DEC-03 | J3 pas actif, pas concerné | J3 se déconnecte | Main retirée, ordre du tour mis à jour, rien d'autre ne change | §7 |
| C-DEC-04 | Révélation, doute juste, le menteur part | — | Tas retiré du jeu, le douteur joue avec 45 s | §7 |
| C-DEC-05 | Révélation, doute faux, le douteur part | — | Tas retiré du jeu, le suivant du douteur actif | §7 |
| C-DEC-08 | Révélation, doute juste, le douteur part | — | Le menteur ramasse, le suivant du douteur actif avec 45 s | §7 |
| C-DEC-09 | Victoire en attente de J1 (il a menti), J2 doute puis part pendant la révélation | — | Doute juste résolu d'abord : victoire annulée, J1 ramasse, J3 actif | §7 |
| C-DEC-10 | Victoire en attente de J1 (honnête), J2 doute puis part pendant la révélation | — | Doute faux : J1 gagne | §7 |
| C-DEC-06 | L'hôte part | — | Partie terminée pour tous, retour au menu avec un message | §7 |
| C-DEC-07 | Partie en cours | Quelqu'un tente de rejoindre | Refusé avec un message clair | §7 |

## Confidentialité (PRV)

À tester sur les **messages réseau réels**, pas seulement sur l'affichage.

| ID | Situation | Résultat attendu | Règle |
|---|---|---|---|
| C-PRV-01 | N'importe quel moment | Un client distant ne reçoit aucune valeur de carte, sauf celles de sa propre main et celles révélées par un doute | §9 |
| C-PRV-02 | Pose d'un coup | Les autres clients reçoivent le nombre de cartes et l'annonce, pas les valeurs | §9 |
| C-PRV-03 | Doute | Seules les cartes du dernier coup sont envoyées, à tous | §9 |
| C-PRV-04 | Ramassage du tas | Seul le joueur qui ramasse reçoit les valeurs des cartes ramassées | §9 |
| C-PRV-05 | Défausse ou déconnexion | Les cartes retirées du jeu ne sont jamais envoyées | §9 |

## Invariants (à vérifier après chaque action)

- Cartes en main + tas + retirées du jeu = 52, chaque carte présente une seule fois.
- Il y a exactement un joueur actif tant que la partie n'est pas finie.
- Tas vide ⇔ pas de valeur de manche.
- Au plus une victoire en attente.
