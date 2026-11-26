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
