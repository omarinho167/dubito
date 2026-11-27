---
name: dubito-art-direction
description: Pipeline de direction artistique de Dubito avec Nano Banana Pro (MCP google-flow) - générer, itérer, analyser, classer et faire valider les images de référence (style, personnages, objets, interface). À utiliser pour la phase DA, une planche de comparaison ou toute image de référence.
---

# Direction artistique de Dubito (Nano Banana Pro)

Outil : MCP `google-flow`, mode d'emploi général dans le skill `google-flow-image` (règles dures : images seulement, jamais de vidéo, limites quotidiennes de Google, ne jamais se connecter à la place du propriétaire). Ce skill dit **quoi** générer pour Dubito, **où** ranger et **comment** juger.

## Sources (ne rien recopier ici)

- Étape en cours : `docs/ROADMAP.md`, phase DA.
- Prompts, bloc commun, directions et noms de fichiers : `art/style/PROMPTS.md` (seule source des prompts de style).
- Contraintes : `docs/VISION.md` (pas Liar's Bar), `docs/ASSETS.md` (contenu de chaque asset), `docs/UX.md` (lisibilité à la table).
- Décisions déjà prises : `docs/DECISIONS.md` (D-67 et suivantes).

## 1. Générer

- `flow_status` : connecté, et `usageToday` laisse assez d'images (PRO : 100 Nano Banana Pro par jour). Prévoir le lot avant de lancer : directions × images × variantes.
- Projet Flow : `Dubito DA` (un seul projet pour toute la phase, les références y restent).
- Prompt = bloc commun de `PROMPTS.md` + prompt de l'image, **mot pour mot**. Format indiqué dans `PROMPTS.md`.
- Exploration : `count: 4`, `resolution: "1k"`. Correction ciblée : `count: 2`.
- Sortie brute : `art/style/<direction>/_gen/` (ignoré par git), `name_prefix` = `<direction>-<lettre>` (ex. `3-neon-A`).
- Références locales (`references`) : seulement des images du dépôt ou validées (ex. image B validée pour garder le même personnage en C). Jamais d'image d'un jeu existant.

## 2. Analyser (soi-même, avant de montrer)

Ouvrir chaque image. Noter chaque critère 0, 1 ou 2 :

| Critère | Ce qu'on regarde |
|---|---|
| Fidélité | tout ce que le prompt demande est là, rien d'ajouté d'important |
| Interdits | **éliminatoire** si texte, logo, filigrane, personnage connu, ambiance Liar's Bar (bar sombre, animaux, arme) |
| Lisibilité de jeu | table, cartes et visages lisibles de loin, lumière principale au-dessus de la table |
| Personnages | formes simples et trapues, visage et mains expressifs, pas de cheveux fins ni de vêtements flottants |
| Modélisable | image B : face et profil, pose en A, fond uni, membres séparés (prête pour Meshy, DA.3) |
| Cohérence | A, B et C d'une même direction semblent venir du même jeu |
| Originalité | ne ressemble pas à un jeu existant |

Garder les 1 ou 2 meilleures variantes par image (total le plus haut, aucun éliminatoire).

## 3. Itérer

- Changer **une seule chose** à la fois (une phrase du prompt de l'image, ou une référence). Ne jamais toucher au bloc commun pendant une comparaison : sinon la comparaison n'est plus juste.
- 3 tours au plus par image ; ensuite montrer le meilleur au propriétaire et dire ce qui résiste.
- Un changement de prompt qui marche et qui servira encore va dans `PROMPTS.md` (avec l'accord du propriétaire s'il change une direction).

## 4. Classer et présenter

- Copier les variantes gardées sous les noms de `PROMPTS.md` (`A-table.png`, `A-table-2.png`…), dans `art/style/<direction>/` ; convertir en PNG si Flow a donné un JPG.
- Planche de comparaison (une ligne par direction, une colonne par image) :

```powershell
python .agents/skills/dubito-art-direction/scripts/planche.py art/style/_planches/<date>-<sujet>.png art/style/1-salon art/style/3-neon --title "DA.1"
```

- `art/style/<direction>/NOTES.md` : 5 lignes au plus, les notes du tableau et pourquoi ces variantes. Le prompt exact se retrouve dans `_gen/flow-manifest.jsonl`.
- Présenter avec `dubito-visual-review` : la planche, puis une question à choix multiples (`dubito-ask-owner`), la recommandation en premier.

## 5. Valider

- Seul le propriétaire valide (AGENTS règle 7). Sa réponse devient une entrée de `docs/DECISIONS.md`.
- Image validée : version 2K avec `flow_download_media` (`resolution: "2k"`, id lu dans le manifeste), rangée à côté sous `<nom>-2k.png`.
- Cocher l'étape de la ROADMAP quand son critère de fin est atteint.

## Au-delà de DA.1

Même boucle pour : la charte d'interface (DA.2, maquettes d'écrans en 16:9), les fiches personnage pour Meshy (DA.3, 3:4, face/profil en A), les dos de cartes ou textures (1:1). Dépôt dans `art/<asset>/ref/` comme indiqué par `docs/ASSETS.md`. Les faces de cartes restent générées par script (`dubito-card-faces`).

## Claude et Codex

Les deux peuvent appeler le MCP ; le serveur fait passer les générations une par une. Pour un gros lot, le confier à Codex avec un brief complet (directions, prompts, dossier, nombre d'images, budget du jour) et vérifier les images avant de les présenter.
