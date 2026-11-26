# Source de `blender-image-to-3d`

- Dépôt : https://github.com/majidmanzarpour/blender-game-skills
- Version : f0ef29385a03de139957e6f700b801cdc00b7e29
- Chemin d'origine : `skills/blender-image-to-3d`
- Licence : MIT (fichier joint)
- Importé le : 2026-11-26, relu par Claude et audité par Codex (aucun script exécuté).

## Modifications locales

- Ajout de la section « Dans Dubito » en tête du SKILL.md : Dans Dubito : `scripts/init_master.py` et `scripts/roundtrip.py` **réinitialisent Blender** ; ne les lancer que dans un Blender séparé (`blender -b`), jamais dans la session MCP ouverte du propriétaire. Pillow est un prérequis (déjà installé sur le PC) ; ne rien installer automatiquement. Le flux Dubito reste `blender-model-from-reference` (références, ASSETS.md) ; ce skill apporte la comparaison mesurée aux photos (silhouettes, mêmes caméras).

Pour mettre à jour : comparer avec la nouvelle version amont (diff), relire, puis remplacer en gardant ces modifications.
