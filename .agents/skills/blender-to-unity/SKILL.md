---
name: blender-to-unity
description: Prépare un modèle Blender pour Dubito puis l'exporte dans Unity URP - nettoyage et diagnostic du maillage, retopologie, UV, baking, rig Rigify et animations assises, export FBX, import Humanoid et matériaux URP/Lit, contrôles d'échelle et de budget. À utiliser avant de livrer tout modèle 3D au projet Unity.
---

# Blender → Unity

## 1. Diagnostic et nettoyage

- Garder la source dense intacte dans une collection `HIGH` (sert au baking).
- Compter triangles (après modificateurs), composants, matériaux. Chercher : doublons, normales inversées, non-manifold, trous, petites îles, faces superposées. Un élément séparé (œil, bouton) peut être voulu : expliquer les exceptions.
- Merge by Distance avec une tolérance adaptée à l'échelle (trop large = doigts fusionnés).
- Objet statique : Décimer en surveillant la silhouette. Objet déformable : **retopologie** (QuadriFlow intégré ou Instant Meshes / Quad Remesher), puis corriger à la main les boucles des épaules, coudes, poignets, hanches, genoux, doigts.

## 2. UV et textures

- Coutures discrètes, dépliage, contrôle au damier, plus de résolution pour le visage et les mains, marges suffisantes.
- Baking du `HIGH` vers le modèle de jeu : normales (et AO/couleur si utile). Vérifier entre les doigts, sous les bras, aux coutures.
- Textures 1K par défaut, 2K seulement si le cadrage le justifie.

## 3. Rig et animations (personnages)

- Transformations appliquées, échelle en mètres, debout au sol, origine aux pieds. Noms stables : `CHR_<nom>`, `RIG_<nom>`, `MAT_<nom>`.
- Rigify : ajuster le métarig, générer, poids automatiques puis corrections ; 4 influences max par sommet.
- Tester les poses de Dubito : assis (bassin sur le siège, genoux pliés, pieds au sol, mains près de la table), prendre une carte, avancer le bras, tourner la tête. Vêtements sans traverser le corps.
- Animations **sur place** (pas de déplacement de la racine). Exporter un squelette de jeu propre, sans les contrôleurs Rigify.

## 4. Export

- FBX : seulement les objets nécessaires, échelle et axes vérifiés sur un premier import (pas de correction ×100 après coup), « Add Leaf Bones » désactivé, animations cuites.
- Source : `.blend` dans `art/<asset>/` (LFS), hors du projet Unity.
- Fichiers du jeu : exporter le `.fbx` et ses textures dans `unity/Assets/Dubito/Art/<asset>/` (LFS), sinon Unity ne les voit pas.

## 5. Import Unity

- Type de rig : **None** pour les objets statiques (table, chaise, carte), **Generic** pour un rig non humain, **Humanoid** seulement pour un personnage : Create From This Model → Configure, vérifier chaque os, la T-pose, les doigts.
- Matériaux recréés en **URP/Lit** : `smoothness = 1 − roughness`, normal map importée en type Normal Map. Peu de matériaux.
- Prefab créé à côté, via Unity (`dubito-unity-editor`).

## Budgets de départ (à mesurer en jeu, avec les 6 sièges occupés)

| Élément | Triangles |
|---|---:|
| Personnage stylisé complet | 10 000 – 25 000 |
| Chaise | 500 – 3 000 |
| Table | 1 000 – 5 000 |
| Carte | 2 par face (plus si épaisseur visible) |

## Contrôle final

- Rendus multi-angles et filaire dans Blender, puis capture depuis la caméra d'un siège dans Unity.
- Pas de sommet sans poids, poids normalisés, animations qui jouent dans Unity, dimensions cohérentes avec la table.
- **Le propriétaire valide** l'apparence et les gestes en jeu (`AGENTS.md` règle 7). Demander son choix si une correction change l'apparence ou impose un compromis.
