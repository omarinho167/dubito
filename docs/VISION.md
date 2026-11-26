# Dubito : vision et périmètre

## Le jeu en une phrase

Un jeu de bluff multijoueur sur Steam : de 2 à 6 amis assis autour d'une table en 3D posent des cartes face cachée, mentent sur leur valeur et se demandent sans arrêt s'il faut **croire, douter ou brûler le tas**.

## Ce qui rend Dubito différent

- **Le dilemme à trois choix** : Jouer, Douter ou Défausser. Le tas est un risque commun qui grossit, et chacun peut le supprimer au prix de son tour. C'est la signature du jeu (`RULES §4.3`).
- **Un vrai jeu de défausse** : on garde sa main d'un tour à l'autre et on gagne en la vidant. Perdre un doute ne fait éliminer personne : tout le monde reste impliqué jusqu'à la fin (seule l'inactivité exclut, `RULES §5`).
- **Le dernier doute** : vider sa main ne suffit pas, il faut survivre à la dernière contestation (`RULES §6`).

## Concurrence (à garder en tête)

**Liar's Bar** (Curve Animation, 2024, environ 57 000 avis Steam, 6,99 $) est déjà un jeu de bluff de cartes à la 1re personne, assis à une table, avec une punition mortelle (roulette russe). D'autres petits jeux l'ont imité depuis, sans grand succès.

Conséquences pour Dubito :

- **Ne pas cloner Liar's Bar**, ni sa présentation (bar sombre, personnages animaux, roulette) ni ses éléments propres.
- Les règles du jeu traditionnel (Cheat, Doute, Menteur…) sont libres, mais la direction artistique, les personnages et la mise en scène doivent être originaux.
- La différence doit venir d'abord des **mécaniques** (ci-dessus), puis d'une direction artistique propre (plus tard).

Détails et sources : [research/CONCURRENCE.md](research/CONCURRENCE.md).

## Périmètre V1

| Dans la V1 | Pas dans la V1 |
|---|---|
| 2 à 6 joueurs en ligne via Steam (amis, invitations) | Matchmaking public, classement |
| Un joueur héberge la partie (hôte) | Serveurs dédiés, migration d'hôte, reconnexion |
| Règles complètes de [RULES.md](RULES.md) | Bluff sur le nombre, Guillotine, variantes |
| 3D basique : table, sièges, cartes, personnages simples | Direction artistique finale, décor détaillé |
| 1re personne assis, regard limité | Déplacement, emotes |
| Souris + clavier | Manette, Steam Deck |
| Sons simples | Musique, ambiance dynamique, vocal intégré (Discord suffit) |
| Revanche dans le même salon | Bots, mode solo |
| Un écran de Règles illustré | Tutoriel, bulles d'aide, historique des coups |
| Réglages de l'hôte : chrono, places max | Variantes de règles activables |
| Interface en français (textes prêts à traduire) | Autres langues |

## Direction artistique

**Pas décidée volontairement.** On fait d'abord un jeu qui marche avec des formes simples (style « greybox »). La direction artistique sera choisie après les premiers playtests, en tenant compte de la section Concurrence.

Pistes déjà évoquées, aucune n'est retenue : taverne infernale (trop proche de Liar's Bar), sci-fi désert (piste Unreal de juillet), tribunal infernal des serments, salon clandestin années 20.

Seule contrainte posée : **personnages stylisés simples** (un squelette commun, des variantes), pas de réalisme type MetaHuman.

## Pour plus tard (V2 et après)

Bluff sur le nombre de cartes, Guillotine (ou autre enjeu visuel), bots, vocal intégré, manette, 7–8 joueurs, variantes de règles, cosmétiques (dos de cartes, titres, sans avantage de jeu).
