---
name: dubito-unity-editor
description: Pilote l'éditeur Unity ouvert de Dubito (MCP Unity, Unity CLI) pour créer ou inspecter scènes, GameObjects, prefabs et matériaux, lire la console, attendre la recompilation et faire des captures. À utiliser dès qu'une tâche agit dans l'éditeur plutôt que sur des fichiers texte.
---

# Piloter l'éditeur Unity

## Quel canal

| Canal | Pour quoi | Limite |
|---|---|---|
| **Unity CLI** (`unity`, `unity command …`) — **canal principal** (D-37) | Piloter l'éditeur ouvert : console, GameObjects, scènes, assets, `script eval`, mode Play, tests, captures | Paquet `com.unity.pipeline` installé dans le projet ; l'éditeur doit être ouvert (`unity status` → `ready`). Guide officiel : `unity skill show` et le skill `using-unity` du plugin Unity. |
| Batchmode `Unity.exe` | Tests et vérifications reproductibles | Impossible si l'éditeur a déjà le projet ouvert (`dubito-core-tests`). |

L'ancien MCP intégré (`com.unity.ai.assistant`, outils `Unity_*`) est **déprécié** et débranché pour Claude et Codex. Ne pas le rebrancher sans accord du propriétaire.

Commandes de base (depuis `unity/`) :

```powershell
unity status                          # éditeur connecté ? (state = ready)
unity command console status          # garder le cursor renvoyé
unity command console get -- --since <cursor> --level error
unity command --query <mot>           # trouver une commande de l'éditeur
unity command <commande> --help       # ses paramètres
```

Si `unity` n'est pas reconnu (terminal ouvert avant l'installation de la CLI), utiliser le chemin complet `& "$env:LOCALAPPDATA\Unity\bin\unity.exe"` ou relancer l'outil.

Toujours chercher une commande avec `unity commands --grep` / `unity command --query`, jamais en filtrant `--help`.

## Boucle de travail

1. Le projet doit être sauvegardé dans git avant une série d'actions (les outils exécutent du code dans l'éditeur).
2. Après un changement de script : **attendre la fin de la recompilation**, puis lire la console (erreurs d'abord). Ne rien conclure avant.
3. Modifier scènes et prefabs par l'API Unity (jamais le YAML à la main, `dubito-unity-foundation`), puis **sauvegarder** la scène ou le prefab explicitement.
4. Noms d'objets clairs et stables (`Table`, `Seat_0`…`Seat_5`, `Pile`), hiérarchie rangée sous des parents.
5. Ne pas lancer le mode Play, ni changer de scène ouverte, ni fermer l'éditeur sans raison : le propriétaire peut être en train de regarder.

## Captures

- Une capture sert à **repérer** un problème (objet absent, hors cadre, mauvaise taille), pas à valider : c'est le propriétaire qui valide ce qui se voit (`AGENTS.md` règle 7).
- Capturer depuis la caméra du joueur (siège), plusieurs angles si besoin. Dire ce qu'on a vérifié et ce qu'on n'a pas pu vérifier.

## Pièges

- Erreurs de compilation ⇒ l'éditeur garde les anciens assemblages : un test « vert » ne prouve rien.
- Une opération du Package Manager est asynchrone : attendre sa fin avant d'enchaîner.
- Le plugin officiel de skills Unity est installé en plus (D-37). Ses guides visent souvent Unity 6.7+ et Netcode/UGS : **les skills Dubito et ARCHITECTURE priment**. Vérifier la compatibilité 6.3 de toute API et ne jamais adopter leurs choix de services (comptes, cloud, Netcode).
