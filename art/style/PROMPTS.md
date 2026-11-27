# Dubito : prompts d'exploration du style (phase DA.1)

But : choisir le style pas à pas (D-72) : rendu, puis lumière et couleurs, puis décor. Chaque étape compare la même scène où une seule chose change.

## Mode d'emploi

Claude ou Codex génèrent, analysent et classent eux-mêmes (skill `dubito-art-direction`, D-68). À la main : coller les phrases dans l'ordre indiqué par l'étape, générer 2 à 4 variantes, garder la meilleure sous le nom indiqué (`-2` pour une variante en plus).

## Contraintes de toutes les étapes

- **Pas Liar's Bar** (VISION) : pas de bar sombre et enfumé, pas de personnages animaux, pas de roulette ni d'arme.
- **Personnages stylisés simples** : formes lisibles, tête et mains expressives (le doute se lit sur le visage et les gestes), pas de cheveux fins ni de vêtements flottants (plus simple à modéliser et animer).
- **Jouable** : la table, les cartes et les visages doivent rester lisibles de loin, sous une lumière principale au-dessus de la table.

## Étape 1 · Rendu graphique (`0-rendu`)

Déjà choisi : ambiance comique entre amis (D-70), humains stylisés (D-71). On compare d'abord le **rendu**, sur une seule scène identique ; décor et couleurs viennent après (D-72). Format 16:9. Fichiers : `art/style/0-rendu/R<n>-<nom>.png`.

**Scène commune** (à coller après la phrase du rendu)
```text
Video game screenshot, first-person view from a seated player at a round card table in a simple cozy room, one lamp hanging above the table. Three stylized human friends sit around the table, each holding a fan of face-down playing cards, exaggerated comedic reactions: one squints suspiciously, one grins nervously, one laughs. A face-down pile of cards in the center of the table. The player's own hand of cards fanned in the lower part of the frame. Original designs only, no text, no letters, no logos, no watermark, no existing characters or franchises. Simple chunky proportions, big expressive faces and hands, no fine hair strands, no loose flowing clothes.
```

| Rendu | Phrase à coller avant la scène |
|---|---|
| **R1 cel-shading (choisi, D-73)** | `Cel-shaded 3D art style: flat color areas, two-tone hard shadows, bold black outlines around every shape, comic-book look.` |
| R2 animation lisse | `Smooth stylized 3D animated-film look: soft rounded shapes, subtle realistic lighting and soft shadows, clean simple materials.` |
| R3 peint à la main | `Hand-painted 3D art style: low-detail models with painted textures, visible brush strokes, lighting painted into the colors.` |
| R4 low-poly | `Low-poly 3D art style: simple geometric shapes with visible flat facets, flat shading, no textures, solid colors.` |

## Étape 2 · Lumière et couleurs (`0-lumiere`)

Rendu fixé : cel-shading (D-73). Prompt = phrase R1 + phrase d'éclairage + scène commune de l'étape 1, mot pour mot. Format 16:9. Fichiers : `art/style/0-lumiere/L<n>-<nom>.png`.

| Éclairage | Phrase à coller entre R1 et la scène |
|---|---|
| **L1 spot (choisi, D-74)** | `Lighting and colors: a single strong warm spotlight from the lamp on the table and faces, the rest of the room falls into deep cool shadow, high contrast.` |
| L2 chaud | `Lighting and colors: the whole room is evenly and brightly lit with warm golden light, soft warm palette of orange, honey and cream, no dark areas.` |
| L3 pop | `Lighting and colors: bright, vivid, saturated pop colors, bold contrasting primary and complementary hues on clothes, walls and furniture, cheerful.` |
| L4 pastel | `Lighting and colors: soft gentle daylight, light pastel palette of mint, peach, lavender and sky blue, low contrast, calm.` |

---

## Étape 3 · Décor (`1-salon`, `2-tribunal`, `3-neon`, `4-garage`)

Fixés : cel-shading (D-73), spot sur la table et fond sombre (D-74). Prompt = phrase R1 + phrase L1 + scène ci-dessous (même réactions et mêmes contraintes que l'étape 1, seuls le lieu, la lampe et les tenues changent). Format 16:9. Fichier gardé : `art/style/<direction>/A-table.png`. Les fiches personnage (B) et objets (C) seront écrites pour le décor choisi.

**Scène** (remplacer `<LIEU>`, `<LAMPE>`, `<TENUES>`)
```text
Video game screenshot, first-person view from a seated player at a round card table in <LIEU>, <LAMPE> hanging above the table. Three stylized human friends <TENUES> sit around the table, each holding a fan of face-down playing cards, exaggerated comedic reactions: one squints suspiciously, one grins nervously, one laughs. A face-down pile of cards in the center of the table. The player's own hand of cards fanned in the lower part of the frame. Original designs only, no text, no letters, no logos, no watermark, no existing characters or franchises. Simple chunky proportions, big expressive faces and hands, no fine hair strands, no loose flowing clothes.
```

| Direction | `<LIEU>` | `<LAMPE>` | `<TENUES>` |
|---|---|---|---|
| 1 Salon années 20 | `an elegant 1920s private art deco salon with brass details, emerald velvet and geometric gold wall patterns` | `an art deco pendant lamp` | `in stylized 1920s outfits` |
| **2 Tribunal des serments (choisi, D-75)** | `a slightly whimsical courtroom with carved wooden balustrades, tall arched windows, red curtains and a judge's bench in the background` | `a large brass courtroom lamp` | `in simple robes and short judge-like hats` |
| 3 Néon rétro-futur | `a retro-futuristic 1980s lounge with smooth rounded furniture and pink and turquoise neon tubes glowing on the walls` | `a round floating lamp` | `in simple retro-futuristic jumpsuits` |
| 4 Garage entre potes | `a cluttered garage turned into a game room, with an old couch, string lights, shelves of boxes and tools` | `a bare bulb lamp` | `in casual hoodies and t-shirts` |

Le premier essai (direction 1 en 3D lisse, avant D-72) reste dans `_gen/flow-manifest.jsonl` et l'historique git.

## Après le choix

- Les images **B** serviront aussi de référence pour le test Meshy AI (DA.3) : vue de face et de profil, pose en A, fond uni.
- La charte d'interface (DA.2) partira de la palette et des formes de la direction choisie.
