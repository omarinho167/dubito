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
| R1 cel-shading (abandonné, D-78) | `Cel-shaded 3D art style: flat color areas, two-tone hard shadows, bold black outlines around every shape, comic-book look.` |
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

Image de comparaison faite en cel-shading (D-73, remplacé depuis par la 3D Unity de base, D-78), spot sur la table et fond sombre (D-74). Prompt = phrase R1 + phrase L1 + scène ci-dessous (même réactions et mêmes contraintes que l'étape 1, seuls le lieu, la lampe et les tenues changent). Format 16:9. Fichier gardé : `art/style/<direction>/A-table.png`. Les fiches personnage (B) et objets (C) seront écrites pour le décor choisi.

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

## Étape 4 · Personnage pour le test 3D (`2-tribunal`, D-77, D-78)

Deux images : un **concept** (pour le charme et les détails) puis la **fiche 3D** faite avec le concept gardé en référence (pour Meshy). Le premier essai « flat lighting, simple solid colors » donnait une poupée sans détail : ne pas réduire le personnage pour le rendre modélisable, c'est la fiche qui l'est.

**Concept** (3:4, `B-concept.png`)
```text
Premium stylized 3D character concept render for a comedic multiplayer card game, high-end real-time game quality, appealing and charming shape language. A plump, charismatic courtroom card player: a pompous old judge with a big round nose, thick bushy grey eyebrows, a curled grey moustache, rosy cheeks and a sly half-smile, one eyebrow raised in suspicion. He wears a deep crimson velvet judge's robe with gold embroidered trims and a row of brass buttons, a crisp white pleated jabot collar, and a small black velvet cap with a gold tassel. Big expressive hands with chunky fingers, one hand holding a fan of three face-down playing cards close to his chest. Three-quarter view, waist up, soft warm key light with a gentle rim light, smooth stylized skin with subtle subsurface scattering, visible fabric texture, dark warm gradient background. Original design, no text, no letters, no logos, no watermark.
```

**Fiche 3D** (16:9, `B-personnage.png`, référence : le concept gardé)
```text
Character turnaround model sheet of the exact same character as the reference image, same face, same outfit, same colors and materials, premium stylized 3D game character render. Full body visible from head to shoes, three views side by side with identical proportions and height: front view, side view, back view. Neutral A-pose, arms straight and away from the body at 45 degrees, hands open with fingers slightly apart, legs slightly apart, feet flat, nothing held in the hands. The robe stops at the knees, showing dark trousers and black leather shoes. Soft neutral studio lighting, no dramatic shadows, plain uniform light grey background, no floor shadow, no props. Original design, no text, no letters, no logos, no watermark.
```

## Après le choix

- Les images **B** serviront aussi de référence pour le test Meshy AI (DA.3) : vue de face et de profil, pose en A, fond uni.
- La charte d'interface (DA.2) partira de la palette et des formes de la direction choisie.
