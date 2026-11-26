---
name: dubito-character-import
description: Importe un personnage humanoïde et ses animations dans Unity pour Dubito (Mixamo FBX ou modèle riggé de Blender) - Avatar Humanoid, clips assis en boucle sur place, Animator, placement assis sur la chaise, couleur de siège. À utiliser pour les mannequins des adversaires et tout futur personnage.
---

# Personnages dans Unity

Source phase 2 : Mixamo Y Bot + animations assises (D-43, `docs/ASSETS.md` §3). Un personnage fait dans Blender passe d'abord par `blender-to-unity`.

## Fichiers

- Bruts fournis : `art/mixamo/` (jamais modifiés). Copie de travail importée : `unity/Assets/Dubito/Art/Characters/Mannequin/` (FBX en Git LFS).
- Un FBX « personnage » (avec maillage, T-pose) et un FBX par animation (« Without Skin »).

## Import

1. Personnage : Rig → **Animation Type Humanoid**, Avatar Definition **Create From This Model** ; Configure → tous les os obligatoires verts, T-pose correcte. Échelle : vérifier la hauteur (≈ 1,8 m debout) ; ne jamais corriger en mettant le prefab à l'échelle 100.
2. Animations : Rig Humanoid. **Copy From Other Avatar** (celui du personnage) seulement si le squelette est identique (animations Mixamo téléchargées appliquées au même Y Bot) ; sinon **Create From This Model** et laisser Unity recibler (retargeting Humanoid). Onglet Animation : **Loop Time** pour les boucles (assis au repos) ; **Root Transform Rotation / Position (Y et XZ) : Bake Into Pose**, basé sur l'original, pour que le personnage reste sur place.
3. Matériaux : extraire et passer en **URP/Lit** ; un matériau de base + une teinte par siège (propriété de couleur ou `MaterialPropertyBlock`), sans dupliquer les textures.

## Animator

- Un Animator Controller `AC_SeatedOpponent` : état par défaut « Sitting Idle » en boucle ; réactions courtes (penser, rire, désapprouver) déclenchées par un paramètre puis retour au repos.
- Décaler le début de la boucle par siège (offset aléatoire) pour éviter six mannequins synchrones.
- L'Animator ne lit que la présentation (événements « a posé », « a perdu un doute »), jamais l'état de jeu caché.

## Assis à la table

- Prefab `PF_Opponent` : mannequin + point d'ancrage du bassin. Le placer pour que le bassin repose sur l'assise et les pieds au sol (`ASSETS §2`) ; ajuster par un décalage du prefab, pas en modifiant les clips.
- Mains près du bord de table : si les mains traversent la table, corriger par la position du siège d'abord ; IK seulement si nécessaire.
- Cartes de dos en éventail tenues près des mains (UX §2) : un point d'ancrage `HandFan` sur le prefab.

## Vérification

- Les 6 places occupées en mode Play : pas de traversée de la chaise ou de la table, animations décalées, couleurs de siège distinctes.
- Captures depuis un siège et vue de côté, puis présentation au propriétaire : `dubito-visual-review`.
