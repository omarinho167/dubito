---
name: dubito-cross-review
description: Use after a significant change to Dubito docs or code (new rules, a finished roadmap step, networking or privacy code) to get an independent second review from the other AI agent (Claude reviews Codex's work, Codex reviews Claude's), then fix the real findings.
---

# Relecture croisée de Dubito

Un agent ne relit pas seul son propre travail important : on fait relire par l'autre (Claude ↔ Codex).

## Lancer la relecture

- **Depuis Claude** : lancer un agent Codex (`codex:sol`, `sandbox: read-only`) avec le prompt ci-dessous.
- **Depuis Codex** : demander au propriétaire de faire relire par Claude, ou faire soi-même la relecture si c'est le travail de Claude qui est relu.

Prompt de relecture (compléter les crochets) :

```text
sandbox: read-only
Relis [fichiers ou diff] du jeu de cartes Dubito dans [chemin du dépôt].
docs/RULES.md est la seule source de vérité des règles ; docs/CASES.md en dérive.
Contexte du changement : [une phrase].
Cherche uniquement de vrais problèmes, sous forme de liste numérotée avec fichier:section et une correction proposée :
1. contradictions avec RULES.md (ou internes à RULES) ;
2. situations non définies qu'un programmeur devrait deviner (les jouer à 2 joueurs et à 6) ;
3. pour du code : bugs, état incohérent, fuite d'information privée (RULES §9), dépendance de Dubito.Core vers Unity/Mirror/Steam ;
4. règles recopiées hors de RULES.md.
Pas de remarques de style. Ne modifie aucun fichier. Réponds en français.
```

## Après la relecture

1. **Vérifier chaque remarque** avant de l'appliquer : citer l'extrait concerné ; écarter celles qui sont fausses, en disant pourquoi.
2. Corriger les vraies, en passant par `dubito-rule-change` si une règle bouge.
3. Une correction qui change le comportement du jeu sans accord du propriétaire est notée **C** dans DECISIONS.md et lui est signalée.
