# Source de `blender-pipeline-automation`

- Dépôt : https://github.com/EL4CTEO/blender-skills
- Version : v1.3.3 (1a1cc6c49312f9f370aae86f52b7b907f29c94f6)
- Chemin d'origine : `skills/blender-pipeline-automation`
- Licence : MIT (fichier joint)
- Importé le : 2026-11-26, relu par Claude et audité par Codex (aucun script exécuté).

## Modifications locales

- Ajout de la section « Dans Dubito » en tête du SKILL.md : Dans Dubito, les traitements par lots tournent **uniquement dans un Blender séparé** (`blender -b`), jamais dans la session MCP ouverte du propriétaire (ils ouvrent ou remplacent le fichier courant). `pip install bpy` et le téléchargement de Blender Linux sont facultatifs et non utilisés ici.

Pour mettre à jour : comparer avec la nouvelle version amont (diff), relire, puis remplacer en gardant ces modifications.
