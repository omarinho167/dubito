# Dubito : règles du jeu (V1)

> **Seule référence pour les règles.** Le code, les tests et les autres docs s'alignent sur ce fichier.
> Une règle n'est écrite qu'ici. Ailleurs, on renvoie vers une section (`RULES §4.2`) sans la recopier.
> Les cas limites détaillés sont dans [CASES.md](CASES.md). Ce que le joueur voit est dans [UX.md](UX.md).

## 1. Résumé

| Question | Réponse V1 |
|---|---|
| Joueurs | 2 à 6 |
| Paquet | 52 cartes, sans joker. Seule la valeur compte (A, 2…10, V, D, R), jamais la couleur. |
| Distribution | Toutes les cartes sont distribuées, certains peuvent avoir une carte de plus. |
| But | Être le premier à vider sa main et survivre au dernier doute. |
| Poser | 1 à 4 cartes face cachée. **Le nombre est vrai et visible**, seule la valeur peut être un mensonge. |
| Valeur de manche | Choisie par celui qui ouvre sur un tas vide, imposée jusqu'à ce que le tas soit vidé. |
| Douter | Seul le joueur suivant, avant de confirmer son action. |
| Défausser | Le joueur actif retire le tas du jeu et passe son tour. |
| Chrono | 45 s par tour par défaut (réglable par l'hôte, §5). Si le temps est écoulé, le tour est sauté. |
| AFK | 2 tours sautés d'affilée : le joueur est exclu. |

## 2. Vocabulaire

| Terme | Sens |
|---|---|
| **Tas** | Les cartes face cachée au centre de la table. |
| **Manche** | Période entre l'ouverture d'un tas vide et le moment où il est vidé (ramassé ou défaussé). |
| **Valeur de manche** | Valeur annoncée à l'ouverture, imposée à toutes les poses de la manche. |
| **Coup** | Une pose : N cartes face cachée, annoncées comme « N × valeur de manche ». |
| **Dernier coup** | Le coup le plus récent posé sur le tas. |
| **Répondant** | Pour un coup donné : le joueur qui était le suivant au moment de la pose. |
| **Coup doutable** | Chaque coup ouvre une **fenêtre de doute** pour son répondant. Elle se ferme **définitivement** dès que ce répondant confirme une action, que son tour est sauté ou qu'il quitte la partie. Elle ne se rouvre jamais, même si plus personne ne pose ensuite. |
| **Joueur actif** | Celui dont c'est le tour. |
| **Suivant** | Le prochain joueur encore en jeu dans l'ordre du tour. |
| **Victoire en attente** | État entre le moment où un joueur vide sa main et la résolution de la réponse du suivant (§6). |
| **Retirer du jeu** | Les cartes sont mises de côté face cachée et ne reviennent jamais. Le retrait ne révèle rien de nouveau (des cartes déjà révélées lors d'un doute restent connues de ceux qui les ont vues). |
| **Révélation** | Phase entre un doute accepté et sa résolution (§4.2). |

## 3. Mise en place

1. Mélanger les 52 cartes.
2. Tirer au hasard l'ordre du tour. Les joueurs sont placés autour de la table dans cet ordre (personne ne choisit sa place). Le premier de l'ordre est le premier joueur.
3. Distribuer **toutes** les cartes une par une, en commençant par le premier joueur et dans l'ordre du tour.
   Avec 52 cartes et N joueurs, les `52 mod N` premiers joueurs ont une carte de plus.
   *(Exemple : 5 joueurs, les 2 premiers ont 11 cartes, les 3 autres 10.)*
4. Le tas est vide, il n'y a pas de valeur de manche.
5. Le premier joueur commence.

Il n'y a pas de pioche : aucune carte n'est tirée pendant la partie.

## 4. Le tour

Le joueur actif fait **une** des actions possibles :

| Situation du joueur actif | Jouer | Douter | Défausser |
|---|---|---|---|
| Tas vide | oui (ouverture) | non | non |
| Il est répondant d'un coup doutable | oui | oui | oui |
| Pas de coup doutable, tas non vide | oui | non | oui |
| Il répond à une victoire en attente | oui | oui | non |

Les autres joueurs ne peuvent rien faire. Pendant une révélation ou une fois la partie finie, personne ne peut agir.

### 4.1 Jouer

- Choisir 1 à 4 cartes de sa main et les poser face cachée sur le tas.
- **Tas vide (ouverture)** : le joueur annonce une valeur, n'importe laquelle de A à R, même s'il n'en a aucune.
  Elle devient la valeur de manche.
- **Tas non vide** : l'annonce porte obligatoirement sur la valeur de manche.
- Le nombre de cartes annoncé est **toujours** le nombre réellement posé : tout le monde le voit.
- Les cartes posées peuvent avoir n'importe quelle valeur : c'est le bluff.
- Après la pose, le tour passe au suivant.
- Si la main du joueur est vide après la pose : victoire en attente (§6).

### 4.2 Douter

- Possible seulement si le joueur actif est le répondant d'un **coup doutable** (§2) : il conteste ce coup.
- On peut douter **tant qu'on n'a pas confirmé** Jouer ou Défausser. Sélectionner des cartes ou ouvrir un menu ne compte pas.
- Le doute accepté ouvre la **révélation** : les cartes du coup contesté (et elles seules) sont **retournées face visible pour toute la table**. Pendant la révélation, le chrono est en pause et toute action de jeu est refusée.
- **Mensonge** : au moins une des cartes révélées n'a pas la valeur de manche.
- **Doute juste** (mensonge) :
  1. le menteur ramasse tout le tas (cartes révélées comprises) dans sa main ;
  2. le tas est vide, la valeur de manche est effacée ;
  3. le douteur reste joueur actif, avec un **nouveau chrono complet**, et doit jouer (il ouvre une nouvelle manche).
- **Doute faux** (toutes les cartes ont la valeur de manche) :
  1. le douteur ramasse tout le tas ;
  2. le tas est vide, la valeur de manche est effacée ;
  3. le douteur perd son tour : le suivant du douteur devient joueur actif et ouvre une nouvelle manche.

### 4.3 Défausser

- Possible seulement si le tas n'est pas vide et qu'il n'y a **pas de victoire en attente**.
- Tout le tas est retiré du jeu, **sans révélation**.
- La valeur de manche est effacée. Le coup doutable, s'il y en avait un, ne l'est plus.
- Le joueur ne pose aucune carte : le tour passe au suivant, qui ouvre une nouvelle manche.

## 5. Chrono et inactivité

- Chaque tour dure **45 secondes** par défaut. L'hôte peut choisir **30, 45, 60 ou 90 s** dans le salon, avant le lancement. La durée ne change plus pendant la partie.
- Le chrono est en pause pendant une révélation.
- Il repart à la durée complète à chaque nouveau joueur actif, et pour le douteur après un doute juste.
- **Temps écoulé** : le tour est **sauté**. Aucune carte n'est posée, aucun doute n'est fait, rien n'est défaussé.
  - Le tas et la valeur de manche ne changent pas.
  - Si le joueur sauté était le répondant du dernier coup, ce coup n'est plus doutable (§2).
  - Si le joueur sauté devait répondre à une victoire en attente, la victoire est confirmée (§6).
- **Inactivité** : au **2e tour sauté d'affilée**, le joueur est exclu (traité comme une déconnexion, §7).
  Toute action confirmée par le joueur remet ce compteur à zéro.

## 6. Victoire

- Un joueur qui vide sa main en jouant crée une **victoire en attente**. La partie n'est pas encore finie.
- Le suivant doit répondre. Il ne peut que **Douter** ou **Jouer** (Défausser est interdit).
  - **Doute juste** : la victoire est annulée. Le joueur ramasse le tas et la partie continue (§4.2).
  - **Doute faux** : la victoire est **confirmée**.
  - **Il confirme Jouer** : la victoire est confirmée avant même que sa pose soit appliquée.
  - **Temps écoulé** ou **déconnexion** du répondant : la victoire est confirmée.
- La partie s'arrête au premier vainqueur confirmé. En V1, il n'y a qu'un gagnant et pas de classement.
- Si un seul joueur reste en jeu (les autres déconnectés ou exclus), il gagne immédiatement.
- Il n'y a pas de durée maximale : une partie peut être longue.

## 7. Déconnexion et exclusion

Un joueur qui se déconnecte ou est exclu (§5) :

- voit sa main **retirée du jeu** ;
- est retiré de l'ordre du tour ; le tas et la valeur de manche ne changent pas ;
- si c'était le joueur actif, le tour passe au suivant, sans coup doutable ;
- si son dernier coup était doutable, il ne l'est plus ;
- s'il était en **victoire en attente**, il ne gagne pas : la victoire est annulée et la partie continue sans coup doutable ;
- s'il devait **répondre** à une victoire en attente, elle est confirmée.

Pendant une révélation, **le doute est toujours résolu d'abord** (verdict, y compris pour une victoire en attente), puis on applique le départ :

| Qui part | Doute juste | Doute faux |
|---|---|---|
| **Le poseur** (accusé) | Personne ne ramasse : le tas est retiré du jeu. Le douteur joue avec 45 s. | Si c'était une victoire en attente, elle est confirmée. Sinon le douteur ramasse et le tour passe à son suivant. |
| **Le douteur** | Le poseur ramasse (victoire en attente annulée). Le tour passe au suivant du douteur, avec 45 s. | Personne ne ramasse : le tas est retiré du jeu. Si c'était une victoire en attente, elle est confirmée. Sinon le tour passe au suivant du douteur. |
| Un autre joueur | Résolution normale (§4.2), puis départ normal. | Idem. |

Session :

- **L'hôte part** : la partie s'arrête pour tout le monde, retour au menu. Pas de migration d'hôte.
- Pas de reconnexion à une partie en cours.
- On ne peut pas rejoindre une partie déjà commencée.

## 8. Ordre de résolution

L'hôte est l'arbitre : il traite les messages **un par un, dans l'ordre où il les reçoit**.

1. Une action reçue avant la fin du chrono (horloge de l'hôte) est valide, même si le client voyait 0 s.
2. Une action reçue quand ce n'est plus possible (mauvais tour, fenêtre fermée, état périmé) est refusée.
   Le client est resynchronisé et un message explique pourquoi.
3. Une déconnexion est traitée au moment où l'hôte la détecte, comme un événement entre deux messages.

## 9. Information visible

| Information | Qui la connaît |
|---|---|
| Ses propres cartes | Soi-même (et l'hôte) |
| Cartes des adversaires | Personne (seul l'hôte les a en mémoire) |
| **Nombre** de cartes de chaque joueur | Tout le monde (vrai nombre, visible en 3D) |
| **Nombre** de cartes du tas | Tout le monde |
| Valeur de manche, annonce de chaque coup | Tout le monde |
| Contenu du tas | Personne, sauf les cartes du coup contesté lors d'un doute (toute la table) |
| Cartes ramassées | Celui qui les ramasse. Les cartes révélées restent connues de ceux qui les ont vues. |
| Cartes défaussées ou retirées du jeu | Aucune nouvelle information : rien n'est révélé au retrait |

## 10. Hors V1 (idées à garder)

Ces idées ne s'appliquent **pas** en V1, elles sont notées pour plus tard :

- **Bluff sur le nombre** : annoncer un autre nombre que celui posé. Demande de cacher le vrai nombre de cartes des mains et du tas.
- **Guillotine** : une lame descend chaque fois qu'on perd un doute. Design à faire, attention à la ressemblance avec Liar's Bar.
- Bots, vocal intégré, manette, 7 et 8 joueurs, classement complet.
- Variantes : valeur montante (A→2→3…), valeur ±1, jokers.

## 11. À vérifier en playtest

- 45 s par tour, est-ce trop ou pas assez ?
- La Défausse est-elle trop forte (si tout le monde brûle les gros tas, il n'y a plus de tension) ?
- La carte en plus des premiers joueurs à la distribution est-elle gênante ?
- Le duel à 2 joueurs est-il intéressant ?
- L'exclusion au 2e tour sauté est-elle trop sévère ?
