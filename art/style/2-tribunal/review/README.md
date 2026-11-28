# Revue du juge — 2026-11-28

Intégration DA.3 préparée pour validation du propriétaire. Aucun nouveau service de génération ni asset externe utilisé. La source Meshy reste intacte.

## Captures finales Unity

- `unity-seat0-3players.png` : caméra du siège joueur, deux juges, champ vertical normal de 60°.
- `unity-seat0-4players.png` : même siège, trois juges ; champ élargi temporairement à 75° pour les montrer en entier. Ce réglage de revue n'est pas sauvegardé dans le jeu.
- `unity-close.png` : gros plan du juge opposé, caméra rapprochée temporairement.
- `unity-side.png` : contacts avec chaise/sol, genoux, coudes et position des mains.

Les quatre images proviennent du mode Play, partie locale existante, boucle assise Mixamo retargetée en Humanoid. Texture d'origine identique aux sièges, aucune teinte du personnage. Les couleurs utilisées ailleurs restent inchangées. Lumières existantes conservées.

## Livrables

- Source Blender : `../meshy/juge-unity.blend`, générée par `../meshy/prepare_juge.py` avec Blender 5.3 headless.
- Modèle : `unity/Assets/Dubito/Art/Characters/Juge/CHR_Juge.fbx`, texture PNG 2048² `T_Juge_BaseColor.png`, matériau URP/Lit `M_Juge.mat` (chemins depuis la racine du dépôt).
- `PF_Opponent` remplace le Y Bot par le juge, sans changer le contrôleur, ses clips ni l'ancrage HandFan. La scène Table contient le nouvel aperçu.
- Métadonnées générées par Unity ; Avatar valide, mapping explicite Spine02 → Spine, Spine01 → Chest, Spine → UpperChest. Les animations Mixamo gardent leur Avatar Y Bot et sont retargetées, sans copie vers le squelette Meshy.

## Contrôle Blender

Rendus EEVEE GPU : `blender-rest-front.png`, `blender-rest-back.png`, `blender-seated-front.png`, `blender-seated-side.png`, `blender-seated-wire.png`. Le test assis Blender est une pose de stress, pas une nouvelle animation livrée.

Hauteur 1,800 m, 31 491 triangles, 15 782 sommets géométriques, 24 os, un matériau. Transformations normalisées, aller-retour FBX à échelle 1. Suppression du faux clip source de deux images ; 7 750 sommets de couture coïncidents soudés, un triangle quasi nul éliminé, contact des lunettes conservé séparé pour éviter une arête à trois faces. Normales personnalisées Meshy préservées, UV conservés. Aucun sommet sans poids, quatre influences maximum, erreur maximale de normalisation 1,44 × 10⁻⁷.

`blender-qa.json` contient les mesures et les deux contrôles QA réussis. Tolérance de coplanarité 10 µm : la tolérance générale de 1 mm signalait des bords de lunettes et des plis de perruque distincts très proches. Les avertissements acceptés sont deux sommets de contact des lunettes volontairement séparés, une paire de faces opposées en pose extrême, et la hauteur du sol de la pose de stress non descendue sur une chaise. Les limites ouvertes proviennent des coques/vêtements/lunettes du modèle ; pas de non-manifold ni de normales incohérentes. Le budget indicatif Dubito de 25 000 triangles est dépassé ; le modèle est conservé pour ce test sans retopologie ni remodelage.

## Vérifications Unity et tests

- Compilation : terminée, aucune erreur console. Avertissement pendant Play : Steam non lancé ; test local sans Steam.
- `dotnet test` à la racine : **339 réussis, 0 échec, 0 ignoré**, code de sortie 0. NU1900 : audit NuGet inaccessible depuis le sandbox ; restauration et tests terminés.
- `unity command test run --mode editor --filter-type Assembly --filter Dubito.Tests` : **426 réussis, 0 échec, 0 ignoré**, code de sortie 0. Rapport NUnit natif daté dans `unity-tests-20261128-112135.xml`.
- `unity-pose-checks.txt` : 2, 4 et 6 places, cinq instants de Sitting Idle (45 juges échantillonnés). Avatars et os Humanoid requis valides ; aucune couleur de personnage via MaterialPropertyBlock ; sommets finis, limites de hauteur/sol cohérentes et absence d'allongement grossier des triangles. Bassin initial à 0,550 m, bas du maillage à environ 0,002 m du sol. Ces vérifications numériques ne certifient pas l'absence de toute petite traversée.
- Relecture technique croisée par Claude : aucun bug réel signalé.

## Défauts et limites à regarder

- Pas d'os de doigts : mains ouvertes figées, doigts irréguliers issus de Meshy. Les poignets bougent avec l'animation, les doigts ne saisissent pas les cartes.
- Les mains reposent bas, près des genoux (environ 0,46–0,49 m). Les éventails de cartes restent sur l'ancrage existant et flottent au-dessus : conservé expressément pour ce test.
- Petites irrégularités des rouleaux de perruque et des sourcils ; une petite marque sombre est visible sur un rouleau latéral. Lunettes fines, contour assez grossier en gros plan. Pas de déchirure majeure observée sur veste/perruque dans les vues et poses contrôlées.
- Ombres du visage et de la veste très marquées, mains/jambes sombres sous la table. Éclairage existant, non retouché.
- Pas de traversée importante de la chaise ou du plateau observée dans ces vues. Les réactions autres que Sitting Idle, le jeu réseau et les performances à six joueurs ne sont pas validés par ce contrôle.

DA.3 reste non cochée. Le propriétaire doit encore regarder le personnage en mouvement dans Unity : « Valider ce test » / « Corriger la pose » / « Revoir l'éclairage ». Cette revue n'enregistre aucune validation visuelle à sa place.
