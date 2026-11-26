---
name: dubito-card-faces
description: Génère par script les 52 faces et le dos des cartes de Dubito (images PNG lisibles, valeurs françaises A à R) et les importe dans Unity avec les bons réglages. À utiliser pour créer, modifier ou régénérer l'apparence des cartes.
---

# Faces des cartes

Décision : D-42. Contenu attendu : `docs/ASSETS.md` §1 (ligne « Faces des cartes »). Les règles n'utilisent que la valeur (RULES §1) : la couleur est décorative.

## Générateur

- Un seul script, `tools/cards/generate_cards.py` (Python 3 + Pillow), **déterministe** : même script = mêmes images. Paramètres en tête du fichier (taille, couleurs, police, marges).
- Sortie : `unity/Assets/Dubito/Art/Cards/Faces/<Rank>_<Suit>.png` (ex. `Queen_Hearts.png`, noms alignés sur les enums `CardRank`/`CardSuit` de Core) et `Back.png`. PNG en Git LFS.
- Taille : rapport 63 × 88 (ex. 504 × 704 px, multiples de 4), image pleine et opaque : les coins arrondis viennent du **modèle 3D** (ASSETS §1), pas de transparence.
- Police : libre (OFL), fichier versionné avec sa licence dans `tools/cards/fonts/`. Jamais une police système Windows.

## Lisibilité

Le contenu (valeurs, placements, symboles, dos) est défini dans ASSETS §1. Méthode :

- Contraste fort, fond clair uni, aucun détail fin qui disparaît à distance.
- Tester chaque face réduite à 64 px de large : la valeur doit rester lisible.
- Dos symétrique (identique dans les deux sens), pour qu'aucune carte ne se distingue par son orientation.

## Import dans Unity

- Texture Type Default, sRGB activé, mipmaps activés, compression normale ; Wrap Mode Clamp.
- Un matériau URP/Lit partagé pour le dos ; les faces par matériaux partagés ou un atlas, **pas** de matériau créé à l'exécution par carte.
- Régénérer = relancer le script puis rafraîchir Unity ; les `.meta` existants doivent rester (mêmes noms de fichiers).

## Vérification

- 53 fichiers, noms exacts, aucune face en double.
- Planche-contact (toutes les faces sur une image) montrée au propriétaire, puis vue en jeu depuis un siège : `dubito-visual-review`.
