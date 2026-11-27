---
name: dubito-route
description: Point d'entrée de toute tâche Dubito (règles, docs, code Unity, scène 3D, cartes, personnages, réseau, Blender, validation visuelle). Indique quel skill spécialisé charger selon la tâche, et dans quel ordre. À lire en premier dès qu'on commence une tâche dans ce dépôt ou qu'on hésite entre plusieurs skills.
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
| 5 | pilote l'éditeur Unity (Unity CLI, scènes, prefabs, console, captures) | `dubito-unity-editor` |
| 6 | construit la scène 3D : pièce, table, sièges, lumière, matériaux, prefabs | `dubito-unity-scene` |
| 7 | touche à la table jouable : caméra assise, sélection des cartes, entrées, interface en jeu | `dubito-table-interaction` |
| 8 | crée ou modifie l'apparence des cartes (faces, dos) | `dubito-card-faces` |
| 9 | importe un personnage ou des animations (Mixamo, Humanoid, assis) | `dubito-character-import` |
| 10 | touche au réseau : Mirror, protocole, vues privées, lobby ou invitations Steam | `dubito-mirror-steam` |
| 11 | crée un modèle 3D dans Blender, d'après photos ou non | `blender-model-from-reference` |
| 12 | nettoie, rigge ou exporte un modèle de Blender vers Unity | `blender-to-unity` |
| 13 | quelque chose qui se voit à l'écran est prêt à être validé | `dubito-visual-review` |
| 15 | génère, itère, classe ou fait valider des images de référence (style, personnages, objets, interface) avec Nano Banana Pro | `dubito-art-direction` |
| 14 | un changement important est fini (règles, étape de ROADMAP, réseau, confidentialité) | `dubito-cross-review` |

Ordre quand plusieurs lignes s'appliquent : décision manquante (1) → pour une règle, écrire d'abord le test qui échoue (4) → changement (2, 3, 5 à 12, 15) → lancer les tests (4) → validation visuelle (13) → relecture (14).
## Skills d'outil (en complément des lignes ci-dessus)

Charger en plus, seulement si la tâche le demande. Les notes « Dans Dubito » de ces skills et les skills `dubito-*` priment.

| Domaine | Skills |
|---|---|
| Blender (toute tâche) | commencer par `blender-core`, qui oriente vers le `blender-*` spécialisé (modélisation, UV, rig, matériaux, éclairage, export…) ; `blender-asset-qa` avant de livrer un modèle |
| Modèle d'après photos | `blender-model-from-reference` (méthode Dubito) + `blender-image-to-3d` (comparaison mesurée aux photos) |
| Export Blender → Unity | `blender-to-unity` + `blender-export-pipelines`, `blender-game-optimization` |
| Unity CLI officiel | `unity-cli`, `unity-pipeline` (et l'index `using-unity` du plugin Unity) |
| Effets, sensations (poses, révélation) | `game-feel` |
| Sons (phase 5) | `audio-design` |
| Assets 2D, icônes, licences d'assets | `create-game-assets` (les cartes restent `dubito-card-faces`) |
| Images Nano Banana Pro (MCP `google-flow`, skill du PC) | `google-flow-image` ; pour Dubito, `dubito-art-direction` prime |
| Bug ou test qui échoue sans cause évidente | `systematic-debugging` |
| Conception, playtests (phase 5) | `game-designer` |
| Publication Steam (plus tard) | `steam-publish` |

**Rangés à part** (dans `.agents/skills-library/`, non chargés automatiquement pour garder la liste des skills courte) : `blender-addon-development`, `blender-clothing`, `blender-facial-shapekeys`, `blender-geometry-nodes`, `blender-hair-grooming`, `blender-sculpting`, `blender-simulation`, `blender-time-of-day`, `blender-video-editing`, `blender-weather-fx`. Si une tâche en a vraiment besoin, lire directement `.agents/skills-library/<nom>/SKILL.md`.

Une validation déjà donnée par le propriétaire dans la conversation vaut pour l'étape 1.

Répartition Claude / Codex : voir `AGENTS.md`, section « Claude et Codex ».
