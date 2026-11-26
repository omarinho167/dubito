---
name: dubito-unity-scene
description: Construit ou modifie la scène 3D de Dubito dans Unity URP - pièce greybox, table, sièges, lumière, matériaux URP/Lit, prefabs, organisation de la hiérarchie et performances. À utiliser pour créer la scène Table, placer des objets 3D ou régler l'éclairage et l'ambiance visuelle.
---

# Scène 3D dans Unity

Contenu et dimensions : `docs/ASSETS.md` (§1 liste, §2 dimensions). Ce que le joueur voit : `docs/UX.md` §2. Pilotage de l'éditeur : `dubito-unity-editor`. Caméra et clics : `dubito-table-interaction`.

## Organisation

- Scènes dans `unity/Assets/Dubito/Scenes/` (`Table.unity` en phase 2). Assets du jeu dans `unity/Assets/Dubito/Art/<catégorie>/`.
- Hiérarchie rangée sous des parents vides : `Room`, `Lighting`, `Table`, `Seats` (`Seat_0`…`Seat_5`), `Pile`, `UI`. Noms stables en anglais : le code les retrouve.
- Tout objet répété (siège, carte, mannequin) est un **prefab** ; la scène ne contient que des instances.
- Les sièges sont placés **par code** à partir du nombre de joueurs (cercle régulier, rayon `ASSETS §2`), jamais à la main pour 2, 3… 6.
- Le visuel n'a aucune logique de jeu : il affiche une `PlayerView` (ARCHITECTURE §2).

## Greybox

- Formes simples Unity (Cube, Cylinder, Plane) ou modèles Blender simples (`blender-model-from-reference`, `blender-to-unity`).
- Un matériau **URP/Lit** par rôle, couleurs unies, peu nombreux : `M_Floor`, `M_Wall`, `M_TableFelt`, `M_TableWood`, `M_Seat`, `M_SeatColor_<n>`. Couleurs de siège bien distinctes (6), jamais l'information par la couleur seule (UX §8).
- Échelle réelle en mètres, objets posés au sol (y = 0), pivots logiques (centre de la table au sol).

## Lumière (D-45)

- Une lumière principale chaude au-dessus de la table (Spot ou Point dans l'abat-jour), ombres douces activées ; la table est la zone la plus claire, les murs restent sombres.
- Lumière ambiante faible (Environment Lighting), pas de soleil.
- Commencer en temps réel. Ne cuire l'éclairage (`lighting bake`) que si les mesures le justifient.
- Post-traitement léger dans le Volume global (tonemapping, un peu de vignette) ; pas d'effets lourds tant que la lisibilité n'est pas validée.
- Les guides du plugin Unity (`using-unity`, URP/post-processing) peuvent aider : vérifier qu'ils s'appliquent à Unity 6.3 et à URP 17.3.

## Performance (cible : PC moyen, 1080p, 60 i/s)

- Mesurer avec les commandes `diagnostics` de la CLI avant d'optimiser. Peu de lumières qui projettent des ombres (1 ou 2).
- Les cartes 3D partagent un seul matériau par face (texture d'atlas ou matériaux partagés), pas un matériau unique par carte instanciée.

## Vérification

- Console sans erreur, scène sauvegardée.
- Captures depuis la caméra d'un siège (2, 4 et 6 joueurs), puis présentation au propriétaire : `dubito-visual-review`.
