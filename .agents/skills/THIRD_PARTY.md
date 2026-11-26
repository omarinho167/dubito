# Skills tiers importés

Copies relues (Claude) et auditées (Codex) le 2026-11-26, épinglées à une version. Chaque dossier contient sa licence et un `SOURCE.md` (dépôt, version, modifications locales). Les notes « **Dans Dubito** » en tête des SKILL.md priment sur le texte d'origine ; `dubito-route` fait autorité.

| Source | Version | Licence | Skills |
|---|---|---|---|
| [EL4CTEO/blender-skills](https://github.com/EL4CTEO/blender-skills) | v1.3.3 (`1a1cc6c`) | MIT | 28 skills `blender-*` : 18 dans `.agents/skills/`, 10 rangés dans `.agents/skills-library/` (hors catalogue, liste dans `dubito-route`) |
| [majidmanzarpour/blender-game-skills](https://github.com/majidmanzarpour/blender-game-skills) | `f0ef293` | MIT | `blender-image-to-3d` |
| [gamedev-skills/awesome-gamedev-agent-skills](https://github.com/gamedev-skills/awesome-gamedev-agent-skills) | `c6ca6c4` | Apache-2.0 | `game-feel`, `audio-design`, `create-game-assets`, `steam-publish` |
| [obra/superpowers](https://github.com/obra/superpowers) | `22f0b46` | MIT | `systematic-debugging` (sans `find-polluter.sh`) |
| [setcodes/gamedev-skills](https://github.com/setcodes/gamedev-skills) | `096c224` | MIT | `game-designer` |
| [scenario-labs/skills](https://github.com/scenario-labs/skills) | `3c4d18c` | MIT | documentation seulement, dans `dubito-unity-scene/references/`, `dubito-character-import/references/`, `dubito-unity-editor/references/` |

Descriptions trop longues raccourcies (noté dans leur `SOURCE.md`) pour tenir dans le catalogue des deux agents.

Non importés (audit) : les scripts et le socle `scenario-unity-expert` (pont d'exécution permissif, incompatible Windows), les installateurs EL4CTEO.

Mettre à jour un skill tiers : télécharger la nouvelle version dans un dossier isolé, comparer (diff), relire et auditer, remplacer en gardant les notes « Dans Dubito », mettre à jour ce tableau et `SOURCE.md`.
