# Recherche : skills d'agents et outils Unity

> Recherche Codex du **2026-11-26**, vérifiée en partie par Claude. Les dépôts évoluent vite : épingler un commit avant de reprendre quoi que ce soit.
> Les skills du projet sont écrits par nous dans `.agents/skills/` à partir de ces idées ; rien n'est copié tel quel.

## Format des skills (Claude Code et Codex)

- Format commun [Agent Skills](https://agentskills.io/specification) : dossier + `SKILL.md`, frontmatter `name` (≤ 64 caractères, = nom du dossier) et `description` (≤ 1 024). Corps conseillé < 500 lignes. Dossiers optionnels `references/`, `scripts/`, `assets/`.
- Le modèle choisit un skill **sur sa description** (divulgation progressive) : elle doit dire quoi et **quand**. Le corps n'est lu qu'après.
- Budget du catalogue : ~1 % du contexte chez Claude Code, ~2 % chez Codex ; trop de skills = descriptions raccourcies ou omises. Garder des descriptions courtes et distinctes.
- Un « skill routeur » est un skill ordinaire : rien ne garantit qu'il soit lu en premier. D'où l'instruction permanente dans `AGENTS.md` (chargé à chaque session par les deux outils) de le lire d'abord.
- Sources : [Claude Code skills](https://code.claude.com/docs/en/skills), [OpenAI build skills](https://learn.chatgpt.com/docs/build-skills), [bonnes pratiques Anthropic](https://platform.claude.com/docs/en/agents-and-tools/agent-skills/best-practices), [évaluer des skills (OpenAI)](https://developers.openai.com/blog/eval-skills), [étude Vercel AGENTS.md vs skills](https://vercel.com/blog/agents-md-outperforms-skills-in-our-agent-evals).

## Collections utiles

| Collection | Licence | Intérêt pour Dubito |
|---|---|---|
| [Unity-Technologies/skills](https://github.com/Unity-Technologies/skills) + [unity-agent-plugin](https://github.com/Unity-Technologies/unity-agent-plugin) | Unity Companion | Officiel, 33 skills (CLI, pipeline, packages, URP, audio, TMP…). **Vise souvent Unity 6.7+ et Netcode/UGS** : à filtrer pour 6.3 + Mirror. |
| [gamedev-skills/awesome-gamedev-agent-skills](https://github.com/gamedev-skills/awesome-gamedev-agent-skills) | Apache-2.0 | 9 skills Unity 6.3, routeur par tâche. |
| [scenario-labs/skills](https://github.com/scenario-labs/skills) | MIT | Skills Unity testés sur 6000.3 / URP 17.3 (surtout Mac). |
| [batihandev/unity-mcp-skills](https://github.com/batihandev/unity-mcp-skills) | MIT | Recettes pour le MCP officiel Unity. |
| [anthropics/skills](https://github.com/anthropics/skills) | Apache-2.0 | `skill-creator` : écrire et tester des skills. |
| [obra/superpowers](https://github.com/obra/superpowers) | MIT | Méthode (TDD, debug, revue). Attention : peut contredire notre répartition Claude/Codex. |
| [EL4CTEO/blender-skills](https://github.com/EL4CTEO/blender-skills), [oh-ashen-one/blender-agent-skills](https://github.com/oh-ashen-one/blender-agent-skills) | MIT (le 2e) | Modélisation, UV, rig, comparaison à une référence. |
| [openai/skills](https://github.com/openai/skills) | par skill | **Déprécié**, remplacé par [openai/plugins](https://github.com/openai/plugins). |

## Unity : MCP intégré déprécié

- Unity a **déprécié le serveur MCP** du paquet `com.unity.ai.assistant` (installé chez nous en `2.22.0-pre.1`). Remplaçant : **Unity CLI** (`unity`, `unity eval`, `unity mcp`) + paquet `com.unity.pipeline`, Unity 6.0 ou plus. Gratuit, connexions locales seulement. Pas de date de fin annoncée. [Annonce Unity](https://docs.unity.com/en-us/unity-cli/replace-mcp-server-unity-cli), [installer la CLI](https://docs.unity.com/en-us/unity-cli/use-unity-cli), [Unity Pipeline](https://docs.unity.com/en-us/unity-cli/unity-pipeline/unity-pipeline-package).
- Installation Windows : `winget install Unity.CLI` (ou via Unity Hub), puis dans le projet ouvert `unity auth login`, `unity pipeline install`. Le guide officiel de la CLI demande aux agents qui ont un terminal d'utiliser `unity command …` plutôt que `unity mcp`.
- **Fait le 2026-11-26** (D-37) : CLI 1.1.0, `com.unity.pipeline` 1.0.1, plugin `unity@unity-agent-plugin` installé pour Claude Code et Codex, ancien MCP débranché.

## Points techniques retenus (détaillés dans les skills)

- Tests Unity en batchmode et verdict fiable : skill `dubito-core-tests`. [Arguments Unity 6.3](https://docs.unity3d.com/6000.3/Documentation/Manual/test-framework/reference-command-line.html)
- UI V1 : uGUI + TextMesh Pro (recommandé par Unity 6.3 pour le jeu). [Comparatif officiel](https://docs.unity.com/en-us/engine/6000.3/manual/uitoolkits/ui-system-compare)
- Plusieurs instances sur un PC : ParrelSync garanti seulement jusqu'à Unity 2022 ; Multiplayer Play Mode d'Unity 6 à évaluer en phase 3.
