---
name: dubito-table-interaction
description: Construit l'interaction à la table 3D de Dubito (caméra 1re personne assise, regard limité, sélection des cartes au clic par raycast, panneau d'action, Douter maintenu, Input System, interface uGUI). À utiliser pour la phase 2 de la ROADMAP et toute modification de ce que le joueur fait à la table.
---

# Interaction à la table

Ce que le joueur voit et fait est dans `docs/UX.md` (§2 caméra, §4 actions, §5 révélation). Ce skill dit comment le coder. Si UX ne tranche pas un geste ou une sensation, demander (`dubito-ask-owner`).

## Séparation

- `Dubito.Presentation` **affiche une vue** reçue (de Core en hot-seat, du réseau ensuite) et **envoie des intentions** (« jouer ces cartes »). Elle ne décide jamais d'une règle et ne lit jamais l'état d'un adversaire (ARCHITECTURE §2).
- Les cartes 3D sont créées à partir de la vue ; une carte adverse n'a pas de valeur côté client.

## Caméra assise

- Un ancrage par siège ; un pivot horizontal (lacet) et un pivot vertical (tangage), bornés relativement au siège selon `UX §2`. Pas de déplacement.
- Une seule caméra et un seul `AudioListener` actifs par instance.
- Après un zoom de révélation, rendre l'orientation d'avant. Respecter l'option de réduction des animations.

## Entrées

- **Input System** (package `com.unity.inputsystem` déjà présent) : actions nommées, une carte d'actions Jeu et une Menus.
- Une action = une seule source : éviter qu'un clic ou Entrée confirme deux fois.
- Douter maintenu : annuler le maintien si la fenêtre perd le focus ou si l'action devient impossible.
- Regard et sélection utilisent la même souris : ne pas verrouiller le curseur en permanence ; le geste exact est une décision du propriétaire.

## Sélection des cartes

- `Camera.ScreenPointToRay` depuis la caméra du joueur, distance limitée, **masque de couche** « cartes sélectionnables » (seulement sa main).
- L'interface passe d'abord : un clic sur un bouton ne sélectionne pas une carte derrière. Avec uGUI + Input System, `InputSystemUIInputModule` dans l'`EventSystem` ; ne pas se fier à `IsPointerOverGameObject` lu dans un callback d'`InputAction` (état de l'image précédente).
- Sélection visible (carte levée, contour), refus expliqué (UX).

## Interface

- **uGUI + TextMesh Pro** en V1 (recommandé par Unity 6.3 pour l'UI en jeu). UI Toolkit possible pour menus et outils d'éditeur.
- Boutons activés selon les actions permises par la vue, jamais selon une règle recalculée dans l'UI.
- Pendant l'attente de réponse de l'hôte : une seule demande en cours, bouton désactivé.

## Vérification

Tests EditMode pour la logique sans rendu (bornes de caméra, état des boutons selon une vue). Le ressenti et le cadrage : capture pour repérer, **validation par le propriétaire**.
