---
name: blender-model-from-reference
description: Modélise un objet ou un personnage stylisé dans Blender (piloté par le MCP Blender et des scripts bpy) à partir d'images de référence fournies par le propriétaire, en comparant à chaque itération des rendus à la même vue que la référence. À utiliser quand le propriétaire donne des images et demande un modèle 3D, ou pour reprendre un modèle existant.
---

# Modéliser depuis des références

Pour Dubito : décor (table, chaises, cartes) et personnages **stylisés simples** (DECISIONS D-12). La 3D est faite par **Codex** dans Blender, sans générateur IA (D-38) ; Claude prépare le brief et vérifie. Pas d'asset externe sans accord (`AGENTS.md` règle 8).

## 1. Lire les références

- Lister les vues fournies (face, profil, dos, trois-quarts), les dimensions connues, ce qui est essentiel (silhouette, tête, mains, couleurs) et ce qui est **deviné** (faces cachées).
- Une photo en perspective n'est pas un plan orthographique. Une vue de profil inventée ou générée par IA = proposition à faire valider si elle change le design.
- S'il manque une vue indispensable, demander au propriétaire (choix multiples : « je devine », « tu fournis l'image », « je génère une proposition »).

## 2. Préparer la scène

- Inspecter d'abord la scène (collections, objets, unités) ; ne rien supprimer sans accord. Sauvegarder le `.blend` dans le dépôt (`art/<asset>/`), en Git LFS.
- Unités en **mètres**. Collection `REF` avec des Empty image `REF_Front`, `REF_Side`, `REF_Back`, alignés sur les mêmes repères (sol, bassin, épaules, sommet du crâne), exclus du rendu et de l'export.
- Une caméra nommée par vue de référence (`CAM_Front`…), même cadrage et même projection que l'image.

## 3. Blockout puis détails

- Ordre : silhouette → proportions → volumes → articulations → détails → matériaux. Quelques corrections par itération, garder la meilleure version précédente.
- Primitives + modificateurs (Mirror, Bevel, Subdivision, Solidify, Array). `bpy.data`/`bmesh` pour la précision, opérateurs avec mode et sélection vérifiés. En mode édition : bmesh, puis écrire dans le mesh.
- Paramètres explicites et nommés (hauteur de table, largeur d'épaules, taille de tête) pour corriger sans tout refaire. Ne pas reconstruire la scène à chaque essai.
- Personnage : pose neutre (T-pose ou A-pose), mains et doigts séparés, pour pouvoir le rigger (`blender-to-unity`).

## 4. Comparer à chaque itération

- Rendre depuis chaque `CAM_*` : silhouette noire, gris neutre, filaire. Comparer côte à côte avec la référence et **écrire** les écarts constatés (trop large, tête trop petite…).
- Pour Dubito, juger surtout depuis les sièges adverses (tête, mains, haut du corps) et vérifier l'échelle à côté de la table et d'une chaise.
- Ce contrôle aide l'agent à corriger. **Le propriétaire valide l'apparence** : lui montrer 2 ou 3 rendus aux étapes clés (silhouette, détails, couleurs), avec une question à choix.

## 5. Avant de livrer

Contrôles techniques et export : `blender-to-unity`.
