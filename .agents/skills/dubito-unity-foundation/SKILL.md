---
name: dubito-unity-foundation
description: Crée ou vérifie la structure du projet Unity de Dubito (asmdefs, dossiers, fichiers .meta et GUID, packages, .gitignore, Git LFS). À utiliser avant d'ajouter un assemblage, un dossier, un package ou un gros fichier binaire, et à l'étape ROADMAP 1.1.
---

# Structure du projet Unity

Le découpage et ses interdits sont dans `docs/ARCHITECTURE.md` §2 et §6. Ce skill dit **comment** le respecter.

## Avant toute modification

1. Lire `unity/ProjectSettings/ProjectVersion.txt`, `unity/Packages/manifest.json`, `packages-lock.json` et les asmdefs concernés.
2. **Ne jamais mettre à jour Unity ni un package** pendant une tâche qui n'a pas ce but. Une mise à jour = décision du propriétaire.

## Assemblages (asmdef)

- `Dubito.Core` : `"noEngineReferences": true`, aucune référence à Mirror, Steam ou un autre asmdef du jeu.
- Dépendances seulement vers le bas (ARCHITECTURE §2), sans cycle. Le nom d'un dossier ne crée aucune frontière : seul l'asmdef compte.
- Tests dans un asmdef de test (Editor seulement, `UNITY_INCLUDE_TESTS`), jamais embarqué dans le jeu.
- Outils qui utilisent `UnityEditor` : asmdef réservé à l'éditeur.
- Code compatible **C# 9 / .NET Standard 2.1** (ARCHITECTURE §1).

## Fichiers .meta, scènes et prefabs

- Un asset et son `.meta` vont ensemble. Le GUID d'un asset existant ne change jamais.
- Déplacer ou renommer dans Unity (éditeur, MCP, `AssetDatabase.MoveAsset`). Si on déplace un fichier hors Unity, déplacer aussi son `.meta`.
- Nouveaux assets : laisser Unity créer le `.meta`. Ne jamais supprimer un `.meta` pour « réparer ».
- YAML des scènes et prefabs : **lecture autorisée pour diagnostiquer, modification interdite à la main** (fileID, GUID, références de scripts, overrides). Passer par Unity (`SerializedObject`, `PrefabUtility`, MCP) puis sauvegarder.
- Éditables à la main : scripts `.cs`, `.asmdef`, `.uss`, `.uxml`, `manifest.json` (avec accord si ça change une version).

## Git

- Versionner `Assets/` (avec les `.meta`), `Packages/manifest.json`, `Packages/packages-lock.json`, `ProjectSettings/`.
- Ignorer `Library/`, `Temp/`, `Logs/`, `obj/`, `UserSettings/`, builds et `.csproj`/`.sln*` générés **dans `unity/` seulement** (les projets `tests/` sont écrits à la main).
- Gros binaires en Git LFS (`*.blend`, `*.fbx`, `*.psd`, `*.wav`, grandes textures), déclarés dans `.gitattributes`. `git lfs track` ne migre pas les fichiers déjà commités.

## Vérification

- Unity compile sans erreur (console vide d'erreurs, voir `dubito-unity-editor`).
- `grep -rn "UnityEngine\|Mirror\|Steamworks" unity/Assets/Dubito/Core` ne trouve rien.
- `git status` ne montre ni `Library/` ni un `.meta` orphelin.
