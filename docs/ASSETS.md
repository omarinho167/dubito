# Dubito : assets de la V1

> Ce qu'il faut à l'écran, d'où ça vient, qui le fait. Le style est volontairement **greybox** tant que la direction artistique n'est pas choisie (DECISIONS D-11, D-40).
> Ce que le joueur voit : [UX.md](UX.md). Méthodes de fabrication : skills listés par `dubito-route`.

## 1. Phase 2 : table jouable (greybox)

| Asset | Contenu | Source | Qui |
|---|---|---|---|
| **Pièce neutre** | Sol, 4 murs sombres, plafond. Aucune fenêtre ni thème. | Formes simples dans Unity | Claude |
| **Lumière** | Une suspension chaude au-dessus de la table (lumière principale avec ombres), ambiance faible. La table est l'endroit le plus éclairé. | Unity (URP) ; abat-jour simple modélisé dans Blender | Claude ; Codex pour l'abat-jour |
| **Table** | Ronde, plateau + pied central, couleur unie (tapis de jeu). | Formes simples (Unity ou Blender) | Codex |
| **Chaise** | Une chaise, dupliquée pour les 6 places. **Test** : modélisée d'après des photos du propriétaire (D-41). Repli : formes simples. | Blender, d'après photos | Codex (photos : propriétaire) |
| **Carte (modèle)** | Carte fine aux coins arrondis, une face recto et une face verso (deux matériaux). | Blender | Codex |
| **Faces des cartes** | 52 faces + 1 dos. Valeurs françaises (A, 2…10, V, D, R) en grand au centre et en petit dans deux coins opposés (lisibles en éventail) ; symboles ♣ ♦ ♥ ♠, rouges pour ♦ ♥, noirs pour ♣ ♠ (le symbole suffit, UX §8). Dos original et symétrique, aucun motif emprunté. | Générées par script (D-42) | Codex |
| **Adversaires** | Mannequin **Y Bot** de Mixamo, un par siège, teinté de la couleur du siège, assis au repos. | Mixamo (Adobe), téléchargé par le propriétaire (D-43) | Propriétaire → Claude (import) |
| **Animations** | Assis au repos (obligatoire) + 2 ou 3 réactions assises (penser, rire, désapprouver). | Mixamo | Propriétaire → Claude |
| **Interface** | Panneau d'action, annonce, chronos, bannières, noms. Police par défaut de TextMesh Pro (Liberation Sans, licence OFL). | Unity uGUI | Claude |
| **Sons** | Aucun en phase 2 (phase 5). | — | — |

## 2. Dimensions de référence

En mètres, pour que tout s'assemble sans retouche :

| Élément | Valeur |
|---|---|
| Table | diamètre 1,6 ; hauteur 0,75 |
| Assise de chaise | hauteur 0,45 |
| Distance centre de la table → bassin d'un joueur assis | 1,05 |
| Yeux d'un joueur assis (caméra) | hauteur ≈ 1,15 |
| Carte | 0,063 × 0,088 (format poker), épaisseur visuelle ≈ 0,001 |
| Pièce | environ 6 × 6, hauteur 3 |

## 3. Fournis par le propriétaire

1. **Photos de chaise** : 1 de face et 1 de côté (et 3/4 si possible), chaise entière, fond dégagé. À déposer dans `art/chair/ref/`.
2. **Mixamo** (compte Adobe), à déposer dans `art/mixamo/` :
   - personnage **Y Bot** : *Download* → Format **FBX for Unity (.fbx)**, Pose **T-pose** ;
   - animations : avec **Y Bot sélectionné**, chercher « sitting » et appliquer **Sitting Idle** + 2 ou 3 réactions assises. Pour chacune : *Download* → **FBX for Unity**, **Without Skin**, 30 images/s.

Licence Mixamo : libre de droits dans un jeu, redistribution des fichiers bruts interdite ; le propriétaire accepte de les versionner dans le dépôt public (D-44).

## 4. Plus tard

Personnages définitifs et décor d'ambiance : après le choix de la direction artistique (VISION). Sons : phase 5 (UX §7).

## 5. Personnage jouable (fiche DA.6)

Vue cible : `art/style/2-tribunal/A-maquette-jeu.png` (D-93).

Un seul personnage, le juge (D-83, D-89), sur toutes les places, varié par textures (D-80). Pensé pour être vu **en buste au-dessus de la table, de près** (caméra plus large, D-90). Étude : `docs/research/2026-11-28-ETUDE-LIARS-BAR.md`.

| Élément | Exigence |
|---|---|
| Silhouette | Celle du juge actuel : maigre, voûté, perruque à rouleaux, lunettes, veste noire, ceinture rouge. |
| Mains | **Grandes et simples**, sans détail fin : main ouverte ≈ 2 × la hauteur d'une carte. 5 doigts épais et bien séparés, comme sur la vue cible (D-93). Mêmes mains pour les adversaires et pour **tes mains à l'écran** (D-88). |
| Os | Squelette Humanoid + 3 os par doigt. Poses de main préparées : tenir le paquet, main à plat, pointer, poing, tapoter. |
| Cartes des adversaires | **Paquet serré tenu dans une main** (D-92), attaché à l'os de la main, épaisseur selon le nombre ; l'autre main libre. Nombre exact affiché en 3D au-dessus du joueur (RULES §9). |
| Tes cartes | Tenues par tes mains visibles, effet loupe à la souris (UX, D-91). |
| Visage | Lisible de 1 à 2 m : gros sourcils, yeux, bouche ; expressions à tester (étape 4 de DA.6 : os, formes ou textures). |
| Animations | Attente assise (2 ou 3 variantes), tête qui suit celui qui parle, jouer (poser des cartes au centre), douter (pointer ou taper la table), réactions à la révélation (rire, colère, dépit), victoire. |
| Budget | Environ 25 000 triangles, 1 ou 2 matériaux, texture 2K. |

## 6. Charte de style (D-95)

Planche : `art/style/charte/charte-v1.png` ; matières et objets de référence : `art/style/charte/C-matieres.png`.

| Élément | Règle |
|---|---|
| Rendu | 3D stylisée, textures peintes douces, sans contours (D-78). |
| Formes | Rondes, généreuses, légèrement exagérées, peu de détails fins. |
| Matières | Bois verni, velours rouge, laiton, parchemin, laine noire ; pas de métal chromé. |
| Lumière | Spot ambre au-dessus de la table, ombres bleu-gris douces (D-74) ; fond sombre mais jamais noir pur (D-54). |
| Table | Plateau en **bois verni** (plus de tapis vert). |

| Couleur | Hex | Usage |
|---|---|---|
| Nuit chaude | `#1A1316` | Fonds, coins sombres |
| Brun d'ombre | `#422620` | Boiseries dans l'ombre |
| Bois verni | `#8B5A3C` | Table, chaises, balustrades |
| Spot ambre | `#F2C27B` | Lumière principale, reflets chauds |
| Peau | `#E0B08C` | Mains, visages |
| Perruque | `#EDE6DA` | Perruques, jabots, cartes |
| Veste noire | `#1E1B22` | Vestes des juges |
| Rouge justice | `#9E2A2B` | Rideaux, ceintures, velours |
| Laiton | `#C9A24A` | Lampe, horloge, détails métal |
| Ombre froide | `#3E4357` | Ombres, contre-jour |
