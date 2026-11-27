# Dubito : prompts d'exploration du style (phase DA.1)

But : comparer **4 directions** sur **les mêmes 3 images**, puis en choisir une (ou en mélanger deux). Rien n'est décidé tant que tu n'as pas choisi.

## Mode d'emploi

Claude ou Codex peuvent tout générer, analyser et classer eux-mêmes (skill `dubito-art-direction`, D-68) ; les étapes ci-dessous restent valables si tu génères à la main.

1. Dans Nano Banana Pro, colle **le bloc commun** puis **le prompt de l'image**. Garde le même bloc commun pour toutes les images, c'est ce qui rend la comparaison juste.
2. Génère **2 ou 3 variantes** par prompt et garde la meilleure (ou deux si tu hésites).
3. Dépose les images dans le dossier de la direction, avec ces noms :
   - `art/style/1-salon/` · `art/style/2-tribunal/` · `art/style/3-neon/` · `art/style/4-jouets/`
   - `A-table.png` (scène vue depuis un siège), `B-personnage.png` (fiche personnage), `C-objets.png` (carte, chaise, lampe).
   - Une variante en plus : `A-table-2.png`, etc.
4. Dis-moi quand c'est fait : je monte une planche de comparaison et je te pose la question du choix.

Si une direction ne te plaît pas dès la première image, inutile de faire B et C. Tu peux aussi proposer ta propre direction : je t'écris ses prompts.

## Contraintes de toutes les directions

- **Pas Liar's Bar** (VISION) : pas de bar sombre et enfumé, pas de personnages animaux, pas de roulette ni d'arme.
- **Personnages stylisés simples** : formes lisibles, tête et mains expressives (le doute se lit sur le visage et les gestes), pas de cheveux fins ni de vêtements flottants (plus simple à modéliser et animer).
- **Jouable** : la table, les cartes et les visages doivent rester lisibles de loin, sous une lumière principale au-dessus de la table.

## Bloc commun (à coller avant chaque prompt)

```text
Stylized 3D video game concept art for "a multiplayer bluffing card game", rendered like a real-time game engine screenshot (clean shapes, simple materials, readable silhouettes), original designs only, no text, no letters, no logos, no watermark, no existing characters or franchises. Characters are stylized humans with simple, chunky proportions, expressive faces and big readable hands, no fine hair strands, no loose flowing clothes.
```

---

## Direction 1 · Salon clandestin années 20 (`1-salon`)

Art déco chaleureux et lumineux : laiton, velours, motifs géométriques. Pas un bar, un salon privé élégant. Personnages en costumes d'époque stylisés.

**A · table** (format 16:9)
```text
First-person view from a seated player at a round card table in an elegant 1920s private art deco salon, warm brass and emerald velvet, geometric gold patterns on the walls, bright and inviting rather than dark. Three stylized characters in 1920s outfits sit around the table, each holding a fan of face-down playing cards, one raising an eyebrow suspiciously. A small face-down pile of cards in the center of the table, a single art deco pendant lamp above casting warm light. The player's own hand of cards fanned in the lower part of the frame.
```

**B · personnage** (format 3:4)
```text
Character sheet of one stylized 1920s card player: full body front view and side view, standing in a relaxed A-pose, arms slightly away from the body, plain light grey background, even studio lighting. Chunky stylized proportions, expressive face with a sly smile, big hands, simple tailored suit with a bow tie, clearly separated arms and legs.
```

**C · objets** (format 1:1)
```text
Prop sheet on a plain light grey background, even studio lighting: the back of a playing card with an original symmetrical art deco pattern, a round card table with a green felt top and brass trim, a matching wooden chair with velvet seat, and a pendant lamp. Each object shown separately, three-quarter view.
```

---

## Direction 2 · Tribunal des serments (`2-tribunal`)

*Dubito* veut dire « je doute » : chaque joueur est à la fois accusé et juge. Boiseries, balustrades, une lampe de bureau de juge, des pupitres. Solennel mais avec humour, couleurs franches.

**A · table** (format 16:9)
```text
First-person view from a seated player at a round card table placed in the middle of a stylized, slightly whimsical courtroom: carved wooden balustrades, tall arched windows with daylight, red curtains, a judge's bench in the background. Three stylized characters in robes and wigs-inspired hats sit around the table, each holding a fan of face-down playing cards, one pointing an accusing finger. A face-down pile of cards in the center under a green banker's lamp. The player's own hand of cards fanned in the lower part of the frame. Playful solemn mood, warm wood and deep red colors.
```

**B · personnage** (format 3:4)
```text
Character sheet of one stylized courtroom card player: full body front view and side view, standing in a relaxed A-pose, arms slightly away from the body, plain light grey background, even studio lighting. Chunky stylized proportions, expressive suspicious face, big hands, simple judge-like robe with a short stylized hat, clearly separated arms and legs.
```

**C · objets** (format 1:1)
```text
Prop sheet on a plain light grey background, even studio lighting: the back of a playing card with an original symmetrical pattern inspired by scales of justice and a wax seal, a round card table in carved dark wood with a green leather top, a matching high-backed wooden chair, and a green banker's desk lamp. Each object shown separately, three-quarter view.
```

---

## Direction 3 · Néon rétro-futur (`3-neon`)

Rétro-futur coloré et pastel, comme une salle de jeu d'un futur des années 80 : néons roses et turquoise, surfaces lisses, lumière douce. Joyeux, pas sombre.

**A · table** (format 16:9)
```text
First-person view from a seated player at a round card table in a bright retro-futuristic lounge, pastel pink and turquoise neon accents, smooth rounded furniture, soft glow, cheerful and colorful rather than dark. Three stylized characters in retro-futuristic outfits with simple visors or headsets sit around the table, each holding a fan of face-down playing cards with glowing edges, one grinning confidently. A face-down pile of cards in the center, a round floating lamp above. The player's own hand of cards fanned in the lower part of the frame.
```

**B · personnage** (format 3:4)
```text
Character sheet of one stylized retro-futuristic card player: full body front view and side view, standing in a relaxed A-pose, arms slightly away from the body, plain light grey background, even studio lighting. Chunky stylized proportions, expressive confident face, big hands, simple fitted jumpsuit with a few glowing pastel stripes, clearly separated arms and legs.
```

**C · objets** (format 1:1)
```text
Prop sheet on a plain light grey background, even studio lighting: the back of a playing card with an original symmetrical retro-futuristic geometric pattern in pink and turquoise, a round card table with a soft glowing edge, a matching rounded chair, and a floating round lamp. Each object shown separately, three-quarter view.
```

---

## Direction 4 · Jouets et pâte à modeler (`4-jouets`)

Comme une maquette faite main : personnages en pâte à modeler, meubles en bois peint, empreintes de doigts visibles, lumière de jour douce. Très différent des autres jeux de bluff.

**A · table** (format 16:9)
```text
First-person view from a seated player at a round card table in a handmade miniature diorama room, everything looks like plasticine and painted wood, visible fingerprints and soft clay textures, warm daylight, cozy and playful. Three stylized clay characters sit around the table, each holding a fan of face-down playing cards, one squinting suspiciously. A face-down pile of cards in the center, a simple paper lampshade above. The player's own hand of cards fanned in the lower part of the frame.
```

**B · personnage** (format 3:4)
```text
Character sheet of one stylized plasticine card player: full body front view and side view, standing in a relaxed A-pose, arms slightly away from the body, plain light grey background, even studio lighting. Chunky clay proportions, expressive face with big eyes and eyebrows, big hands, simple sweater and trousers, visible clay texture, clearly separated arms and legs.
```

**C · objets** (format 1:1)
```text
Prop sheet on a plain light grey background, even studio lighting: the back of a playing card with an original symmetrical hand-painted pattern, a round card table in painted wood with a felt top, a matching painted wooden chair, and a paper lampshade. All objects look handmade from clay, wood and paper. Each object shown separately, three-quarter view.
```

---

## Après le choix

- Les images **B** serviront aussi de référence pour le test Meshy AI (DA.3) : vue de face et de profil, pose en A, fond uni.
- La charte d'interface (DA.2) partira de la palette et des formes de la direction choisie.
