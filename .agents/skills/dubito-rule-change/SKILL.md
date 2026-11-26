---
name: dubito-rule-change
description: Use whenever a Dubito game rule is added, changed or clarified (turn flow, doubt, discard, timer, victory, disconnection, dealing, visibility), including when code or a test reveals a rule gap. Keeps RULES.md, CASES.md, DECISIONS.md and tests in sync, in that order.
---

# Changer une règle de Dubito

`docs/RULES.md` est la seule source de vérité. Une règle qui change sans passer par ces étapes recrée le chaos de la version Unreal.

## Avant de toucher quoi que ce soit

1. Identifier la ou les sections de `docs/RULES.md` concernées (`§4.2`…).
2. Si le changement n'a **pas** été demandé ou validé par le propriétaire, ne pas l'appliquer : utiliser le skill `dubito-ask-owner` pour lui poser la question (choix multiple). Exception : une simple reformulation qui ne change pas le comportement.

## Étapes, dans cet ordre

1. **RULES.md** : modifier la règle à son unique endroit. Ne pas la recopier ailleurs ; ailleurs, renvoyer à la section.
   - Vérifier le vocabulaire de `RULES §2` (répondant, coup doutable, révélation, retirer du jeu…).
   - Vérifier la matrice d'actions de `RULES §4` et la table d'information de `RULES §9`.
2. **CASES.md** : ajouter, modifier ou supprimer les cas concernés (`C-XXX-NN`, nouvel identifiant = numéro suivant, ne jamais réutiliser un numéro supprimé). Chaque cas cite sa section de RULES.
3. **DECISIONS.md** : ajouter une ligne en bas (`D-NN`, date, décision, pourquoi, P ou C). Si elle remplace une décision : « remplace D-xx ». Ne jamais réécrire une ancienne ligne.
4. **Autres docs** : chercher les renvois (`grep -rn "§4.2" docs/`) et les textes qui décrivent ce comportement (UX, VISION, ROADMAP). Les ajuster sans y recopier la règle.
5. **Code et tests** (dès que la phase 1 existe) : mettre à jour les tests nommés par l'identifiant du cas, puis le code de `Dubito.Core`. Lancer les tests.

## Vérification finale

- Relire RULES en entier sur les sections touchées : pas de contradiction avec §5 (chrono), §6 (victoire), §7 (déconnexion), §8 (ordre de résolution).
- Les invariants de `CASES.md` (52 cartes, un seul joueur actif, tas vide ⇔ pas de valeur) tiennent toujours.
- Pour un changement important : lancer le skill `dubito-cross-review`.
