# Guide pour les agents IA (Claude, Codex…)

Dubito est un jeu de bluff de cartes multijoueur (Unity + Mirror + Steam). Le propriétaire n'est pas développeur : il valide le jeu, l'IA écrit le code.

## Où trouver quoi

| Fichier | Contenu | Autorité |
|---|---|---|
| [docs/RULES.md](docs/RULES.md) | Règles du jeu | **Seule source de vérité pour les règles** |
| [docs/CASES.md](docs/CASES.md) | Cas limites numérotés = tests | Dérivé de RULES |
| [docs/UX.md](docs/UX.md) | Ce que le joueur voit et fait | Présentation |
| [docs/ASSETS.md](docs/ASSETS.md) | Assets à l'écran : contenu, source, qui, dimensions | Contenu visuel |
| [docs/VISION.md](docs/VISION.md) | Pitch, périmètre V1, concurrence | Périmètre |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Choix techniques et découpage du code | Technique |
| [docs/ROADMAP.md](docs/ROADMAP.md) | Étapes et avancement | Ce qu'on fait maintenant |
| [docs/DECISIONS.md](docs/DECISIONS.md) | Journal des décisions et leurs raisons | Historique |
| `docs/research/` | Recherches datées | Contexte, pas des règles |

Ordre de lecture pour une tâche : ROADMAP (étape en cours), puis le fichier concerné. Lire RULES dès qu'on touche au jeu.

## Règles de travail

1. **Une information n'est écrite qu'à un seul endroit.** Ailleurs, on fait un lien (`RULES §4.2`) au lieu de recopier.
2. **Le code suit RULES.md.** S'ils se contredisent, on s'arrête et on demande au propriétaire. On ne « corrige » jamais une règle en silence.
3. **Une étape de la ROADMAP à la fois**, avec son critère de fin. On coche l'étape et on met la date quand le propriétaire l'a validée.
4. **Tests d'abord pour les règles** : chaque cas de CASES.md devient un test nommé par son identifiant (`C-DOU-03`).
5. **Core reste du C# pur** : aucune référence à Unity, Mirror ou Steam dans `Dubito.Core` (ARCHITECTURE §2).
6. **Questions au propriétaire** : sous forme de **choix multiples** (2 à 4 options, la recommandation en premier), jamais de questions ouvertes. Chaque réponse importante devient une entrée de DECISIONS.md.
7. **Vérification visuelle** : c'est le propriétaire qui valide ce qui se voit à l'écran. Ne pas déclarer « ça marche » sur la seule base d'une capture.
8. **Pas d'asset externe** (modèles, sons, packs) sans l'accord du propriétaire, avec la licence vérifiée.
9. Ne pas mettre d'avancement, de compte rendu ou d'astuce d'outil dans les docs de conception. Ça va dans les commits ou dans ROADMAP (une ligne).

## Claude et Codex

Le propriétaire parle à **Claude** (Claude Code). **Codex** est branché à Claude par le plugin Codex (agents `codex:*`), et le propriétaire a un abonnement aux deux : on l'exploite au maximum (DECISIONS D-35).

- **Claude, chef d'équipe** : dialogue avec le propriétaire, conception, découpage en tâches, intégration, docs, vérification finale.
- **Codex** : grosses tâches de code bien délimitées, **modélisation 3D dans Blender** (D-38), recherches, travail mécanique, et relecture du travail de Claude (`dubito-cross-review`).
- Une tâche confiée à Codex reçoit un brief complet (Codex ne voit pas la conversation) : fichiers, sections de RULES, critère de fin. Claude vérifie le résultat (tests, diff) avant de le déclarer fait.
- Deux agents n'écrivent jamais dans les mêmes fichiers en même temps.
- En cas de doute sur qui fait quoi, Claude pose la question au propriétaire (`dubito-ask-owner`).
- Si Codex travaille seul (sans Claude), il suit ce même fichier et demande au propriétaire de faire relire par Claude.

## Skills partagés

Une seule copie, dans `.agents/skills/` (lu par Codex). Claude Code les lit via la jonction `.claude/skills`, à créer une fois après chaque clone :

```powershell
powershell -ExecutionPolicy Bypass -File tools\setup-skills.ps1
```

**Au début de chaque tâche, lire le skill `dubito-route`** : il dit quel skill utiliser (règles, Unity, tests, réseau, Blender, 3D, relecture) et dans quel ordre. La liste des skills et leur usage n'est écrite que là.

Sources et choix des skills : [docs/research/SKILLS-ET-OUTILS.md](docs/research/SKILLS-ET-OUTILS.md).

Frontmatter portable seulement : `name` et `description`. Pas d'options propres à Claude ou à Codex. Les skills tiers gardent leur frontmatter d'origine ; ils sont listés dans [.agents/skills/THIRD_PARTY.md](.agents/skills/THIRD_PARTY.md) avec leur licence et leur version.

**Claude et Codex voient toujours les mêmes skills** (D-48) :

- **Projet** : tout skill propre à Dubito ou importé pour lui vit dans `.agents/skills/` (y compris les skills tiers). Exceptions : les skills installés et mis à jour par un outil (`unity-cli`, `unity-pipeline` par Unity CLI) et les plugins, qui restent au niveau du PC.
- **PC** : `~\.agents\skills` est le dossier commun aux deux outils pour les autres projets ; `tools\sync-skills.ps1` relie chaque skill à Claude par une jonction, sans toucher aux dossiers gérés par un outil (`~\.claude\skills\synced`, `~\.codex\skills\.system`, skills synchronisés par d'autres projets).
- Skills rarement utiles ici : rangés dans `.agents/skills-library/` (hors catalogue des deux agents, liste dans `dubito-route`), pour que la liste chargée reste courte. Descriptions : 300 caractères environ au maximum.
- Après avoir ajouté ou installé un skill, n'importe où : lancer le script ; sans option il vérifie, avec `-Apply` il corrige (sauvegarde d'abord).

```powershell
powershell -ExecutionPolicy Bypass -File tools\sync-skills.ps1
```

## Commandes

Détails et verdict fiable : skill `dubito-core-tests` ; pilotage de l'éditeur : skill `dubito-unity-editor`.

```powershell
dotnet test                       # depuis la racine : tests des règles hors Unity (secondes)
```

```powershell
unity command test run --mode editor --filter-type Assembly --filter Dubito.Tests   # depuis unity/, éditeur ouvert
```
