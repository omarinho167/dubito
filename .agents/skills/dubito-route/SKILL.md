---
name: dubito-route
description: Point d'entrée de toute tâche Dubito (règles, docs, code Unity, réseau, 3D, Blender). Indique quel skill spécialisé charger selon la tâche, et dans quel ordre. À lire en premier dès qu'on commence une tâche dans ce dépôt ou qu'on hésite entre plusieurs skills.
---

# Choisir le bon skill Dubito

Ce skill ne fait qu'orienter. Il ne remplace ni `AGENTS.md` ni les docs.

1. Lire l'étape en cours dans `docs/ROADMAP.md`.
2. Repérer la ou les lignes du tableau qui correspondent à la tâche.
3. Charger **seulement** les skills de ces lignes (invocation native, sinon lire leur `SKILL.md`), dans l'ordre du tableau.
4. Si aucune ligne ne correspond (question simple, explication, petite correction), continuer sans autre skill.

| # | La tâche… | Skill |
|---|---|---|
| 1 | demande une décision du propriétaire, ou docs et code se contredisent | `dubito-ask-owner` |
| 2 | ajoute, change ou clarifie une règle du jeu | `dubito-rule-change` |
| 3 | crée ou modifie la structure Unity : asmdefs, dossiers, `.meta`, packages, `.gitignore`, LFS | `dubito-unity-foundation` |
| 4 | écrit ou corrige du code de Core ou des tests, ou s'apprête à déclarer une étape de code finie | `dubito-core-tests` |
| 5 | pilote l'éditeur Unity (MCP, Unity CLI, scènes, prefabs, console, captures) | `dubito-unity-editor` |
| 6 | touche à la table 3D : caméra assise, sélection des cartes, entrées, interface en jeu | `dubito-table-interaction` |
| 7 | touche au réseau : Mirror, protocole, vues privées, lobby ou invitations Steam | `dubito-mirror-steam` |
| 8 | crée un modèle 3D à partir d'images de référence | `blender-model-from-reference` |
| 9 | nettoie, rigge ou exporte un modèle de Blender vers Unity | `blender-to-unity` |
| 10 | un changement important est fini (règles, étape de ROADMAP, réseau, confidentialité) | `dubito-cross-review` |

Ordre quand plusieurs lignes s'appliquent : décision manquante (1) → changement (2, 3, 5 à 9) → tests (4) → relecture (10).
Une validation déjà donnée par le propriétaire dans la conversation vaut pour l'étape 1.

Répartition Claude / Codex : voir `AGENTS.md`, section « Claude et Codex ».
