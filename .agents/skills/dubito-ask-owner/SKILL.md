---
name: dubito-ask-owner
description: Use when a Dubito decision belongs to the owner (game rules, scope, UX, art, tech choices with trade-offs, publishing, assets) or when docs and code disagree. Asks multiple-choice questions only, then records the answers in docs/DECISIONS.md.
---

# Poser une question au propriétaire de Dubito

Le propriétaire n'est pas développeur. Il veut beaucoup de questions, mais **toujours à choix multiples, jamais ouvertes**.

## Quand demander

- Une règle manque, est ambiguë ou contredit le code.
- Un choix change ce que le joueur vit (UX, périmètre, difficulté) ou coûte cher (asset, service payant, outil).
- Une action est difficile à annuler ou publique (push, suppression, publication).

Ne pas demander ce qui se décide seul par la doc, le code ou un choix technique sans impact visible. Dans ce cas, décider, l'écrire dans DECISIONS avec **C** et le signaler.

## Comment demander

- Utiliser l'outil de question interactive (Claude : `AskUserQuestion`). Sans cet outil, présenter des options numérotées et demander de répondre par un numéro.
- Jusqu'à 4 questions par lot, regroupées par thème.
- 2 à 4 options par question, **la recommandation en premier**, suivie de « (Recommandé) ».
- Chaque option : un libellé court + une phrase sur sa conséquence concrète pour le jeu ou pour le projet.
- La question doit se comprendre seule : rappeler en une phrase le contexte nécessaire.
- En français, sans jargon. Expliquer un terme technique s'il est inévitable.

## Après la réponse

1. Lire la réponse en entier : le propriétaire répond souvent « Autre » avec une idée différente, qui peut changer une règle (exemple : la suppression du bluff sur le nombre).
2. Ajouter chaque décision importante en bas de `docs/DECISIONS.md` (`D-NN`, **P**).
3. Si c'est une règle : appliquer le skill `dubito-rule-change`.
