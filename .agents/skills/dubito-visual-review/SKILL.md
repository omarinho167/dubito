---
name: dubito-visual-review
description: Prépare des preuves visuelles (captures Unity depuis un siège, rendus Blender, comparaison avec une photo de référence) et les présente au propriétaire de Dubito pour validation, avec une question à choix multiples. À utiliser avant de déclarer fini tout ce qui se voit à l'écran (scène, modèle 3D, cartes, animation, interface).
---

# Faire valider le visuel par le propriétaire

Règle : c'est le propriétaire qui valide ce qui se voit (`AGENTS.md` règle 7). Une capture sert à **repérer** un défaut, pas à se valider soi-même.

## 1. Contrôler d'abord soi-même

- Unity : console sans erreur ; `unity command capture game_view` depuis la caméra d'un siège (et `capture scene_view` pour une vue d'ensemble). Varier : 2, 4 et 6 joueurs ; son siège et un siège adverse.
- Blender : `render_viewport_to_path` ou capture de la zone 3D, depuis les mêmes caméras que les références (`blender-model-from-reference`).
- Lister ce qu'on voit et ce qui cloche (objet absent, hors cadre, traversée, illisible, trop sombre). Corriger ce qui est objectivement faux avant de montrer.

## 2. Préparer ce qu'on montre

- 2 à 4 images au plus, chacune avec une légende d'une ligne (« vue depuis le siège 0, 6 joueurs »).
- Pour un modèle d'après photo : image côte à côte **référence | rendu**, même angle.
- Ranger les images dans `docs/review/<date>-<sujet>/` seulement si elles servent plus tard ; sinon dans un dossier temporaire.
- Les envoyer au propriétaire (outil d'envoi de fichiers ou chemin cliquable). Dire honnêtement ce qui n'a pas été vérifié (mouvement, ressenti, autre résolution).

## 3. Demander

- Une question à choix multiples (`dubito-ask-owner`), concrète, avec la recommandation en premier, par exemple : « Valider » / « Plus clair » / « Table plus grande » / « Autre ».
- Le mouvement et le ressenti (caméra, animations) ne se jugent pas sur une image : demander au propriétaire de lancer le mode Play, et lui dire exactement quoi regarder.

## 4. Après

- Validé : cocher l'étape dans la ROADMAP si c'est son critère de fin.
- Refusé ou « Autre » : appliquer, puis recommencer ce skill. Une préférence durable (style, taille, couleur) devient une ligne de DECISIONS ou d'ASSETS.
