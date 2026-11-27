# Style toon dans Unity : ce que Dubito peut réellement obtenir

Recherche du **28 novembre 2026**, phase DA.1, pour **Unity 6000.3.29f1 / URP 17.3.0**. Sources des éditeurs et des développeurs, consultées le **28/11/2026**. Aucun shader installé, aucun achat ni essai dans Unity : « documenté » signifie vérifié dans les sources, **pas testé sur notre projet**. Prix en USD, hors taxes ; promotions susceptibles de finir.

**Oui, le tribunal en 3D avec aplats, ombres franches, contours noirs et réactions comiques est réaliste.** Une image Nano Banana dessine chaque détail pour un seul angle ; le jeu doit conserver des volumes et des expressions cohérents depuis chaque siège. La difficulté principale sera la fabrication des personnages et de leurs visages, davantage que la puissance du PC.

Contexte faisant autorité : [DECISIONS D-70 à D-75](../DECISIONS.md), [UX §2](../UX.md#2-la-scène), [ASSETS §2](../ASSETS.md#2-dimensions-de-référence). Les abonnements, droits des sorties et outils de capture restent dans la [recherche du 27/11](2026-11-27-PIPELINE-3D-ANIMATION.md) ; seuls les compléments utiles au toon sont traités ici.

## 1. Options de shader

Un **shader** décide comment une surface reçoit la lumière. Le toon remplace ses dégradés par quelques couleurs choisies. Il ne transforme pas automatiquement un visage réaliste ou mal construit en personnage de BD.

**Lecture de la compatibilité :** Unity 6.0 correspond à URP 17.0 ; cela ne suffit pas à prouver Unity 6.3 / URP 17.3. Les preuves précises et leurs limites figurent ci-dessous. Aucun éditeur consulté ne certifie notre correctif exact **6000.3.29f1**.

| Option, version repérée | Compatibilité documentée avec notre cible | Plusieurs lumières, spot et ombres | Prix relevé | Licence / GitHub public | Avis pour Dubito |
|---|---|---|---|---|---|
| **Unity Toon Shader (UTS), 0.16.1-preview** | Branche Unity 6 ; corrections explicites **6.3+** en 0.15.0. URP pris en charge ; pas de certificat spécifique 17.3.0. | Plusieurs lumières ; **ombres additionnelles ajoutées en 0.14.0**. Le spot est donc un candidat documenté, à éprouver en build. | **0 $** | Unity Companion License (UCL) : source distribuable dans un projet Unity sous conditions ; exemples Unity-chan sous licence distincte. | **Choix principal pour un prototype.** Complet, gratuit ; statut preview à surveiller. [S1][S2][S3] |
| **Shader maison, Shader Graph 17.3 + HLSL** | Outils et exemples Custom Lighting officiels **17.3.0 / Unity 6.3**. Le shader final reste à développer et vérifier. | Oui si nous programmons la boucle des lumières, atténuation du spot et lecture des ombres. Un simple graphe de tutoriel peut ignorer le spot. | **0 $ d'achat**, travail de développement | Notre code original publiable ; conserver l'UCL et les notices si l'on reprend des exemples Unity. | **Repli maîtrisable**, limité aux besoins de la table ; entretien à notre charge. [S4][S5] |
| **Toony Colors Pro 2, 2.9.22** | URP et corrections Unity **6.1 / 6.2 / 6.5** dans le journal ; tableau Store arrêté à 2022.3. **Couple 6.3/17.3 non explicitement certifié.** | Hybrid Shader 2 / générateur : lumières additionnelles, ombres, rampes réglables. Écarter Standard PBS, réservé au Built-in. | **37,50 $** en promotion ; **75 $** normal | EULA Asset Store, Extension Asset ; fichiers bruts **non publiables** dans GitHub public. | Bon outil généraliste, mais achat et organisation privée nécessaires. [S6][S7][S8] |
| **RealToon, 5.0.16** | Store : **6000.0 URP compatible**, annonce Unity 6 ; **6.3/17.3 précis non confirmé** dans les pages consultées. | Multi-lighting, points/spots et ombres annoncés ; Forward, Forward+ et Deferred. Contours géométriques et post-process proposés. | **17,50 $** en promotion ; **35 $** normal | EULA Asset Store, Extension Asset ; fichiers bruts **non publiables**. | Beaucoup de réglages pour peu d'argent ; pas de raison d'acheter avant l'essai gratuit UTS. [S9] |
| **lilToon, 2.4.1** | URP et Unity 6 suivis ; **6.3/17.3 non garanti**. Incident de compilation signalé en **6000.3.10f1 avec 2.3.2** ; ne pas l'attribuer sans test à 2.4.1. | Lumières locales et ombres ; ombres du spot à confirmer sur notre cible. Calcul des lumières additionnelles URP déplacé par pixel en 2.3.4. | **0 $** | **MIT**, publication permise avec copyright et licence. | Intéressant pour avatars ; moins rassurant ici que la piste officielle ciblant 6.3. [S10][S11] |
| **Flat Kit, 4.9.15** | Store : **6000.3 URP compatible** ; journal 4.9.9 : correction explicite **6.3+**. Bonne preuve documentaire pour cette famille ; 17.3.0 exact non nommé. | Points/spots, plusieurs lumières et ombres Unity ; contours par objet ou image. Réglages toon et éclairage réel à équilibrer. | **19,95 $** en promotion ; **39,90 $** normal | EULA Asset Store, Single Entity ; fichiers bruts **non publiables**. | Alternative payante pertinente si ses outils font gagner du temps ; le dépôt public reste une contrainte. [S12][S13] |

**Maintenance observée :** UTS 0.16.1 daté du **31/10/2026** ; TCP2 2.9.22 publié le **24/07/2026** ; RealToon 5.0.16 le **17/09/2026** ; lilToon 2.4.1 le **20/10/2026**. Flat Kit 4.9.15 : journal **29/10**, publication Store **04/11/2026**. Ces mises à jour sont des signes de suivi, pas une garantie d'absence de bugs. [S1][S6][S9][S10][S12]

**Point à ne pas manquer sur UTS :** la documentation souvent trouvée par recherche est encore en **0.12-preview (2025)**. Elle précède les améliorations d'ombres et les corrections Unity 6.3. Comparer les versions avant d'écarter ou d'adopter le shader. Son ajout de lumière précalculée en 0.16.0 est récent : ne pas supposer qu'un ancien UTS gérait déjà tous les lightmaps. [S1][S2]

**Shader Graph 17.3 :** Unity fournit même un exemple Custom Lighting Toon ; sa personnalisation de l'éclairage vise **Forward/Forward+**, pas Deferred/Deferred+. Les lumières multiples passent par HLSL, car les graphes ne proposent pas de boucle. C'est une base de travail officielle, pas une solution sans code pour tous les cas. [S4]

**Dépôt public :** l'UCL autorise la distribution liée à un contenu créé sous licence Unity, avec licence, copyrights et notices conservés ; elle n'est pas une licence permettant un usage libre dans n'importe quel moteur. Les exemples Unity-chan ont leurs propres conditions. [S3]
L'EULA Asset Store permet le jeu distribué avec le shader intégré, mais pas de fournir librement les sources du pack. Gratuit sur le Store ne veut pas dire librement redistribuable. Les Extension Assets ont aussi des règles de postes/licences. Un achat demanderait de garder le pack hors du dépôt public et de prévoir son acquisition pour reconstruire le projet ; ne pas présumer que les shaders générés échappent à ces restrictions. [S14]

Autre piste gratuite : l'[exemple NiloCat](https://github.com/ColinLeung-NiloCat/UnityURPToonLitShaderExample), sous MIT, montre éclairage et contours URP. C'est une base pédagogique, distincte du produit commercial NiloToon ; aucune certification 6.3/17.3 relevée, donc pas un remplacement prêt à l'emploi. [S15]

## 2. Contours : choisir les traits, pas tout entourer

| Technique | Résultat et avantages | Défauts à prévoir | Coût relatif |
|---|---|---|---|
| **Coque inversée**, poussée suivant des normales lissées | Le personnage est redessiné en noir, légèrement gonflé, derrière lui. Silhouette stable et réglable par personnage ; bon départ pour têtes, robes et mains. | Des normales séparées créent des trous ; angles pointus, doigts proches et intersections peuvent produire des taches. Une épaisseur en mètres rétrécit avec la distance ; une extrusion en espace écran peut la stabiliser en pixels. Pas de trait automatique sur chaque détail intérieur. | Une passe supplémentaire par objet/matériau concerné, sommets à traiter et pixels supplémentaires ; pas « deux fois le coût du jeu ». |
| **Détection de bords à l'écran**, Renderer Feature URP | Repère les ruptures de profondeur et de normales ; peut entourer personnages et architecture avec une largeur en pixels. Peu dépendante du nombre d'objets. | Peut souligner chaque pli, faire scintiller les détails ou rater deux surfaces coplanaires. Transparences souvent absentes des buffers ; à traiter séparément. Détails lointains noyés par un gros trait ; antialiasing à régler. | Une ou plusieurs passes plein écran, lectures de textures et parfois prépasse de normales : coût surtout lié à la résolution. |
| **Traits peints dans la texture**, petits éléments géométriques | Sourcils, commissures, plis choisis et détails du tribunal contrôlés par l'artiste. Très utile en complément des silhouettes. | Les traits peints se déforment avec la peau et restent présents sous tous les angles ; résolution et raccords UV à soigner. La géométrie de traits peut flotter ou croiser la surface. | Texture déjà utilisée : faible surcoût ; objets/matériaux séparés : davantage d'appels de dessin. |

Analyse technique fondée sur les documentations des contours UTS/Flat Kit, le Full Screen Pass Unity 6.3 et le retour de production Rollerdrome. [S13][S16][S17][S18]

**Pour Dubito :** coque inversée fine sur les personnages ; normales lissées dédiées au contour sans effacer les normales artistiques du visage. Masquer le contour là où il bouche les doigts, les lèvres ou les yeux. Ajouter des lignes intérieures dessinées, puis envisager un contour écran discret pour le décor seulement si nécessaire. Éviter de doubler le même trait avec les deux méthodes.
UTS fournit largeur, masque et réglage selon la distance. Un visage plus sombre que son contour perdra sa silhouette : l'éclairage et le fond comptent autant que l'épaisseur. [S16]
Pour un contour écran maison, utiliser une Renderer Feature compatible **Render Graph de Unity 6**, ou le Full Screen Pass existant ; un ancien script de post-process Built-in ne suffit pas. [S17]

## 3. Expressions comiques : le compromis le plus faisable

| Solution | Ce qu'elle permet | Travail et compatibilité |
|---|---|---|
| **Textures 2D interchangeables** | Yeux, pupilles, sourcils et bouche dessinés sur la tête ; rire, soupçon, surprise et clignement très lisibles. | **Premier choix, conforme à D-73.** Faire une planche de visages ou plusieurs zones UV. Fonctionne sur Meshy ou Blender après préparation des UV ; indépendant du squelette du corps et des clips Mixamo. Ne change ni le volume des joues ni la silhouette de la bouche. |
| **Blendshapes** (formes de visage à mélanger) | Ouvrir la bouche, gonfler les joues, fermer de vraies paupières, déformer un sourire ; transitions continues. | À fabriquer dans Blender avec une topologie propre et inchangée entre les formes, puis exporter en FBX. Unity sait les animer. Meshy ne garantit pas de livrer ces formes ; travail spécifique par tête. [S19] |
| **Os du visage** | Mâchoire, yeux et sourcils géométriques ; mouvements réutilisables si les têtes partagent le même rig. | Poids et contrôles à construire ; les clips corporels Mixamo n'apportent pas automatiquement ces expressions. Plus simple pour une mâchoire que pour un sourire subtil ; trop de contrôles augmente le temps de réglage. |

**Proposition concrète :** commencer par six états dessinés — neutre, sourire, rire, soupçon, surprise, dépit — avec pupilles orientables et clignement. Réutiliser les poses de corps ; synchroniser expression et réaction. Préparer une petite forme d'ouverture de mâchoire seulement si le rire paraît trop plat. Ce complément serait à faire valider, sans remplacer D-73 dans cette recherche.

La bouche dessinée restera collée à la joue si elle déborde vers le profil ; limiter sa largeur et vérifier les sièges voisins. Des yeux plats sur une tête très réaliste sembleront plaqués : dessiner la tête pour cette technique. Préférer un masque dans le matériau opaque de la tête aux plans transparents superposés (tri, scintillement et ombres incohérentes).

**Mixamo reste l'outil du corps.** Conserver le modèle Blender final et lui appliquer les clips corporels ; gérer le visage séparément dans Unity. Ne pas présumer qu'un aller-retour d'auto-rig préservera un rig facial ou ses blendshapes. Adobe limite l'auto-rig aux humanoïdes suffisamment lisibles ; nos petits chapeaux, mains et robes ne doivent pas masquer les articulations lors de la préparation. Capture iPhone : reprendre les réserves sur corps/doigts/visage dans la recherche du 27/11. [S20]

## 4. Jusqu'où peut-on faire croire à un dessin 2D ?

**Le moteur sait aller très loin ; l'automatisation seule n'y arrive pas.** Les résultats les plus convaincants combinent shader, modèle, couleurs, traits et animation conçus ensemble. Les références suivantes servent à comprendre une technique, sans reprendre leurs personnages ni leur style.

| Jeu 3D de référence | Technique documentée par ses créateurs | Enseignement pour Dubito |
|---|---|---|
| **Guilty Gear Xrd** (2014 ; conférence GDC 2015) | Ombre à seuil, normales retouchées surtout sur les visages, direction d'éclairage dédiée par personnage, coque inversée, lignes par UV ; poses sans interpolation et déformations artistiques. | Un shader simple exige des modèles et poses très travaillés pour paraître dessinés. Ne pas promettre le même résultat depuis six sièges avec des modèles IA bruts. [S21] |
| **Hi-Fi RUSH** (2023 ; GDC 2024) | UE4 modifié : rendu toon différé du monde entier, passes spécialisées, traitement des ombres et visages, composition en post-process ; objectif 60 images/s. | Personnages et décor peuvent partager un monde BD cohérent ; ce pipeline de studio dépasse l'installation d'un shader Unity. [S22] |
| **Rollerdrome** (2022 ; présentation Unity 04/01/2023) | Contours extérieurs, contrôle par couleurs de sommets, contours d'ombres et de textures, masques pour supprimer les traits indésirables. | Exemple directement pertinent sous Unity : choisir où tracer les lignes est essentiel à la lisibilité. [S18] |
| **Sable** (2021 ; étude Unity et GDC 2022) | Modèles peu polygonés, esthétique aplatie, couleurs pour définir l'ambiance, textures et effets dessinés sur iPad, animation à allure stop-motion. Les sources consultées ne détaillent pas l'algorithme de contour. | Réduire formes et matières est une stratégie de production crédible ; ne pas supposer qu'un preset recrée son travail artistique. [S23] |

**Techniques réalistes à combiner :**

- **Normales retouchées** : changer les directions utilisées pour éclairer les joues et le nez, sans changer leur forme. Cela évite les petites taches d'ombre sur le visage. Conserver ces normales à l'import ; les recalculer peut détruire le résultat. [S21]
- **Rampe de couleur** : choisir deux couleurs et un seuil entre lumière et ombre ; une transition très courte limite le scintillement. Plusieurs sources additionnées peuvent recréer des dégradés : il faut limiter leur influence ou plafonner leur somme. [S7]
- **Textures à aplats** : peau, robe, cheveux sans pores, fibre réaliste ni reflet photographique. Les plis importants sont modelés ou dessinés ; pas besoin de milliers de petits détails.
- **Lignes intérieures choisies** : sourcils, nez, couture, pli de manche ; peints dans une texture suffisamment nette aux distances de jeu. La coque inversée ne dessine pas ces lignes. [S18][S21]
- **Rim light** : fin liseré clair sur un bord pour détacher une robe sombre du fond. Effet artistique à masquer ; trop fort, il donne une apparence plastique ou lumineuse. UTS le propose déjà. [S2]
- **Animation par poses** : accentuer anticipation, arrêt et retour au repos ; tester une cadence de poses réduite uniquement sur les réactions. La caméra et les cartes doivent rester fluides. Une capture Mixamo/iPhone brute aura généralement un mouvement plus continu qu'un dessin animé. [S21]

**Hors d'un premier objectif raisonnable :** redessiner la silhouette de chaque visage selon l'angle, corriger chaque image d'animation, faire tourner les détails comme dans un dessin de profil tout en restant parfait de face, ou reproduire un long métrage à six personnages avec capture automatique. Ce sont des limites de production et d'itération artistique, pas une interdiction de Unity.

## 5. Spot chaud et fond sombre : faisable avec un éclairage arrangé

**Oui**, avec un vrai spot chaud au-dessus de la table, un environnement bleu désaturé et un appoint faible pour les visages. Dans URP, la **Main Light est directionnelle** ; le spot est une **Additional Light**, même s'il domine visuellement la scène. Le shader doit donc lire ses lumières additionnelles **et leurs ombres**. Un shader calculant seulement Main Light manquera l'effet central voulu. [S5][S24]

Réglage de départ proposé, à éprouver plus tard :

1. Un spot assez large pour couvrir table, mains et visages ; ombres actives dans le spot, les matériaux et les réglages URP. Commencer en Forward, suffisant avec peu de sources ; Forward+ reste possible mais exige le bon traitement des lumières dans un shader maison.
2. Un appoint doux sans ombres, ou une contribution d'ambiance contrôlée, pour garder yeux et bouche visibles. Teinter l'ombre du matériau en bleu/violet ; une lumière froide seule ne fabrique pas automatiquement la bonne couleur d'ombre.
3. Sous les chapeaux, atténuer l'ombre reçue par les visages ou ajuster la source : un spot strictement vertical peut cacher les expressions. Conserver les ombres utiles des bras et des cartes sur la table.
4. Peindre les teintes sombres du décor, garder quelques bords lisibles et limiter les lumières qui se chevauchent ; le noir complet ferait disparaître balustrades et robes.
5. Le **cône lumineux visible dans l'air** de la planche n'apparaît pas avec un spot seul : prévoir un effet de faisceau simple, facultatif, sans promettre brouillard volumétrique et ombres volumétriques. Le halo de la lampe peut rester discret.

Les bandes de couleur de la surface et les ombres projetées sont deux calculs distincts. Garder une petite transition/antialiasing sur leurs bords évite des escaliers trop visibles. Les matériaux du visage pourront volontairement recevoir moins de contraste que les robes : c'est une mise en scène, pas une simulation physique.

## 6. Meshy : une base à adapter, pas un personnage toon fini

La documentation actuelle annonce **Meshy 7.1**, sorties notamment FBX/GLB/OBJ, modèles texturés et cartes PBR selon options. Remesh : triangles ou maillage majoritairement en quadrilatères ; Smart Topology T2/T2.5 : **triangles uniquement**, avec nombre de faces ciblé. Ces fonctions ne prouvent pas que les boucles autour des lèvres et des doigts seront adaptées à l'animation. [S25][S26]

**Adaptation proposée dans Blender :**

1. Partir d'une référence du personnage seul, debout en A/T-pose, mains séparées du corps, éclairage neutre. Une image assise dans le tribunal sert à l'ambiance ; elle risque de faire fusionner robe, chaise et mains lors d'une génération 3D.
2. Contrôler silhouette, doigts, paupières et bouche ; corriger ou refaire les zones devant se déformer. Une robe volumineuse doit pouvoir s'asseoir sans traverser la table.
3. Refaire la couleur en aplats propres et retirer les ombres/reflets déjà peints. L'API propose `remove_lighting` **seulement pour certains modèles de texture**, pas comme garantie universelle. [S25]
4. Réduire ou supprimer normales de microdétail, métal et rugosité inutiles ; conserver surtout couleur, masque d'ombre et masque de contour. Le shader toon reste calculé dans Unity.
5. Préparer les UV du visage pour ses expressions ; retoucher les normales ; rig et tests d'assise dans Blender. Exporter en FBX pour le flux Unity/Mixamo déjà étudié.

**Si le visage ou les mains coûtent trop à réparer :** fabriquer une tête et des mains originales dans Blender, réutilisables sur les variantes. C'est plus prévisible que six générations indépendantes. Pas d'évaluation chiffrée de réussite Meshy réalisée ici ; tarifs et licences des modèles restent dans la recherche du 27/11.

## 7. Performance : six personnages ne sont pas un obstacle de principe

**Estimation de conception, sans benchmark :** viser 60 images/s à 1080p sur un PC de jeu de classe GTX 1660 / RTX 2060, 16 Go RAM, paraît raisonnable pour une seule salle et six personnages simples. Ce n'est pas une configuration minimale validée. La RTX 3070 de développement ne suffit pas à prouver la performance des PC des amis.

| Budget de départ proposé | Pourquoi |
|---|---|
| **20 000–40 000 triangles par personnage**, 120 000–240 000 pour six ; décor/table autour de 100 000 | Ordre de grandeur de travail, pas limite Unity ; privilégier visages et mains, simplifier les parties cachées. Les passes de contour/ombre retraitent une partie de cette géométrie. |
| **2–3 matériaux par personnage**, couleurs partagées, textures 1K à 2K | Éviter un matériau pour chaque œil, sourcil, dent ou accessoire ; atlas de visage pour varier les expressions. |
| **Un spot avec ombres**, appoints sans ombres ; carte d'ombre à tester dès 1 024 pixels | Les ombres imposent des dessins supplémentaires. Un point avec ombres demande six directions ; le spot est moins coûteux à cet égard. [S27] |
| Une seule technique de contour au départ ; pas de simulation de tissu/cheveux | Moins de passes, transparences, calculs de déformation et risques de traversées. Cheveux en volumes simples, plis de robe modélisés. |

Le coût réel vient aussi de la couverture des personnages à l'écran, des passes, de la résolution, des lumières et du nombre d'objets dessinés ; le nombre de triangles seul ne prédit pas les images/s. Si contour écran, comparer 1080p et 1440p ; un effet plein écran traite environ **1,78 fois** plus de pixels à 1440p. Un rendu toon n'est pas automatiquement plus rapide qu'un matériau standard.

Validation future : build PC, six personnages animés, mains/cartes et décor complets, depuis les six sièges ; relever temps CPU/GPU, pics et mémoire dans Profiler/Frame Debugger. **60 images/s = 16,7 ms par image** pour le CPU comme pour le GPU, avec marge souhaitable. Tester sur un PC représentatif avant d'annoncer un minimum matériel. Aucun test de performance n'a été effectué pendant cette recherche.

## 8. Ce qui est réaliste pour Dubito

Lecture de la variante **« Tribunal des serments »** de la [planche décor](../../art/style/_planches/2026-11-28-decor.png) : la scène peut devenir un **petit décor de théâtre en 3D à couleurs BD**, avec personnages caricaturaux et réactions lisibles. Le résultat peut être fortement dessiné, tout en laissant percevoir le volume quand on tourne la tête.

| On garde | On simplifie ou l'on perd au départ |
|---|---|
| Robes et petits chapeaux, proportions exagérées, gros sourcils, nez reconnaissables | Microplis et mimiques uniques de chaque illustration ; expressions issues d'une bibliothèque préparée |
| Table orange/chaude, fond bleu sombre, rideaux rouges, balustrades et banc du juge | Traits de décor moins nombreux et plus réguliers ; détails fins simplifiés pour rester lisibles |
| Deux tons dominants et contours noirs ; yeux/mains mis en valeur | Ombres moins parfaitement « dessinées à la main » sur chaque joue ; quelques transitions et ombres projetées supplémentaires |
| Mise en scène comique, regards, rire, gestes des mains | Déformation moins libre qu'un dessin ; une bouche réellement béante et des joues mobiles demandent du travail supplémentaire |
| Vue assise et plusieurs places cohérentes en 3D | La perspective embellie par Nano Banana et le placement idéal des visages ne seront pas identiques pour tous les sièges |

**Ce qu'il faut montrer dans la prochaine maquette :** les volumes d'un modèle 3D propre, un contour régulier, un visage dessiné sur sa tête et une robe simple. Éviter l'illustration où le nez, les lèvres et les ombres changent de forme indépendamment de la géométrie. La planche actuelle peut rester une intention de couleur et d'humour ; elle n'est pas une capture attendue pixel pour pixel.

## 9. Contraintes à ajouter aux prompts d'images

Pour les maquettes de scène, phrases anglaises courtes à combiner :

- “Real-time 3D game screenshot, Unity 6 URP toon rendering.”
- “Consistent modeled volumes and perspective.”
- “Simple stylized human meshes, expressive large hands.”
- “Two-tone cel shading with clean, stable shadow shapes.”
- “Thin uniform black silhouette outlines.”
- “Flat albedo colors, minimal painted interior lines.”
- “Swappable 2D eyes, eyebrows and mouth on a 3D head.”
- “Simple mouth shapes, readable from three-quarter views.”
- “Chunky modeled hair, simple robes, no cloth simulation.”
- “One warm overhead spotlight and subtle face fill lighting.”
- “Cool dark background, readable faces under small judge hats.”
- “Seated first-person view around a round table.”
- “Hand-painted comic feel, consistent 3D geometry.”
- “No painterly brushwork, no cinematic ray tracing.”
- “No dense fog or complex volumetric light beams.”

Pour une référence **destinée à Meshy/Blender**, remplacer la mise en scène et la lumière dramatique par : “Single character, neutral A-pose, separated fingers, plain background.” et “Front, side and back views, identical proportions, neutral lighting.” Séparer cette fiche du personnage de la maquette d'ambiance ; demander plusieurs vues ne garantit pas que l'IA les rendra cohérentes.

## 10. Recommandation et risques

**Option principale : Unity Toon Shader 0.16.1-preview**, version figée après vérification, deux tons réglés, coque inversée fine, textures de visage interchangeables, un spot chaud avec ombres et un appoint discret. Fabriquer d'abord **un personnage et une réaction assise**, puis vérifier sa lecture depuis tous les sièges. Cette voie respecte les choix artistiques et évite un pack payant incompatible avec le versionnement public.

**Repli : un shader maison limité en Shader Graph 17.3 + HLSL**, avec les mêmes personnages, textures et principes de contour. Contrôler explicitement le spot, sa portée, ses ombres et la couleur froide des zones ombrées. C'est plus de code à maintenir, mais le besoin est circonscrit à une salle, quelques matières opaques et peu de lumières. Le shader maison n'élimine pas le travail sur les visages.

**Risques prioritaires :** compatibilité exacte de la preview UTS en build ; ombres du spot et sous les chapeaux ; contour aux doigts et aux intersections ; topologie et textures Meshy ; expressions trop plates vues de côté ; multiplication des lumières qui dilue les aplats. Les packs payants peuvent gagner du temps, mais ne résolvent pas seuls ces problèmes ni les droits du dépôt public.

**Avant de produire six personnages**, le futur essai doit montrer un visage lisible au repos et en réaction, les mains propres autour des cartes, des ombres sans taches et un retour au repos sans traversée. La validation visuelle appartient au propriétaire ; cette recherche ne valide ni un achat, ni DA.1, ni un rendu final.

## Sources

**Toutes consultées le 28/11/2026.** Les dates indiquées sont celles des versions/publications, quand disponibles ; une documentation courante sans date stable est marquée comme telle. Les budgets, choix de production et limites pour notre équipe sont des analyses de cette recherche.

- **S1 — UTS, journal officiel**, 0.14.0 (17/04/2026), 0.15.0 (25/06), 0.16.1 (31/10) : https://github.com/Unity-Technologies/com.unity.toonshader/blob/master/com.unity.toonshader/CHANGELOG.md
- **S2 — UTS, versions et description du paquet**, 0.16.1-preview : https://github.com/Unity-Technologies/com.unity.toonshader/releases ; https://github.com/Unity-Technologies/com.unity.toonshader/blob/master/com.unity.toonshader/package.json ; ancienne documentation 0.12 (28/07/2025) : https://docs.unity3d.com/Packages/com.unity.toonshader@0.12/manual/index.html
- **S3 — UTS, licences**, courant ; UCL 1.4 du 29/10/2024 : https://github.com/Unity-Technologies/com.unity.toonshader#license ; https://unity.com/legal/licenses/unity-companion-license
- **S4 — Unity, Custom Lighting et limites**, documentation Shader Graph **17.3.0**, générée le 21/10/2026 : https://docs.unity3d.com/Packages/com.unity.shadergraph@17.3/manual/Shader-Graph-Sample-Custom-Lighting.html ; https://docs.unity3d.com/Packages/com.unity.shadergraph@17.3/manual/Shader-Graph-Sample-Custom-Lighting-Introduction.html
- **S5 — Unity, éclairage personnalisé et fonctions des lumières**, manuel **6000.3**, courant : https://docs.unity.com/en-us/engine/6000.3/manual/lighting-overview/lighting/custom-lighting/introduction ; https://docs.unity.com/en-us/engine/6000.3/manual/materials-and-shaders/shaders/writing-custom-shaders-urp/use-built-in-shader-methods/lighting
- **S6 — TCP2, fiche et prix**, 2.9.22, 24/07/2026 : https://assetstore.unity.com/packages/vfx/shaders/toony-colors-pro-2-8105
- **S7 — Jean Moreno, documentation TCP2 et générateur**, juillet 2026 : https://www.jeanmoreno.com/unity/toonycolorspro/doc/ ; https://jeanmoreno.com/unity/toonycolorspro/doc/shader_generator_2
- **S8 — Jean Moreno, journal TCP2**, 2.9.22, juillet 2026 : https://www.jeanmoreno.com/unity/toonycolorspro/doc/changelog
- **S9 — MJQ Studio Works, fiche officielle RealToon**, 5.0.16, 17/09/2026, prix et fonctions : https://assetstore.unity.com/packages/vfx/shaders/realtoon-pro-anime-toon-shader-65518
- **S10 — lilxyzw, lilToon : journal et licence**, 2.4.1, 20/10/2026 : https://github.com/lilxyzw/lilToon/blob/master/Assets/lilToon/CHANGELOG.md ; https://github.com/lilxyzw/lilToon/blob/master/LICENSE
- **S11 — lilToon, incident URP #400**, ouvert le 20/03/2026, ancien 2.3.2 : https://github.com/lilxyzw/lilToon/issues/400
- **S12 — Dustyroom, Flat Kit : fiche et journal**, 4.9.15, octobre/novembre 2026 : https://assetstore.unity.com/packages/vfx/shaders/flat-kit-toon-shading-and-water-143368 ; https://flatkit.dustyroom.com/release-notes/
- **S13 — Dustyroom, matériau toon et contours**, documentation courante : https://flatkit.dustyroom.com/stylized-surface/
- **S14 — Unity, Asset Store EULA**, texte courant, §§2.2–2.4 : https://unity.com/legal/as-terms
- **S15 — NiloCat, exemple URP et licence MIT**, dépôt courant : https://github.com/ColinLeung-NiloCat/UnityURPToonLitShaderExample ; https://github.com/ColinLeung-NiloCat/UnityURPToonLitShaderExample/blob/master/LICENSE
- **S16 — Unity, contours UTS**, documentation courante : https://github.com/Unity-Technologies/com.unity.toonshader/blob/master/com.unity.toonshader/Documentation~/Outline.md
- **S17 — Unity, Full Screen Pass Renderer Feature**, manuel **6000.3**, courant : https://docs.unity.com/en-us/engine/6000.3/manual/post-processing-and-full-screen-effects/urp/custom-post-processing/renderer-feature-full-screen-pass
- **S18 — Alina Sommer / Roll7, présentation Unity de Rollerdrome**, **04/01/2023**, notamment 0:00, 1:31, 3:52, 4:30 et 6:00 : https://www.youtube.com/watch?v=G1NY0LKDqJo
- **S19 — Unity, import et animation des blendshapes**, manuel **6000.3**, courant : https://docs.unity.com/en-us/engine/6000.3/manual/animation-section/animation-mecanim/animation-clips/animation-editor-guide/blend-shapes
- **S20 — Adobe, FAQ Mixamo**, dernière date affichée **14/09/2021**, toujours en ligne : https://helpx.adobe.com/creative-cloud/faq/mixamo-faq.html
- **S21 — Junya C. Motomura / Arc System Works, Guilty Gear Xrd**, **GDC 2015**, pp. 15–27 : https://media.gdcvault.com/gdc2015/presentations/Motomura_Junya_GuiltyGearXrd.pdf
- **S22 — Kosuke Tanaka et Takashi Komada / Tango Gameworks, Hi-Fi RUSH**, **GDC 2024**, vidéo officielle publiée le 08/07/2024 : https://gdcvault.com/play/1034330/3D-Toon-Rendering-in-Hi ; https://www.youtube.com/watch?v=gdBACyIOCtc
- **S23 — Shedworks / Unity, fabrication de Sable**, étude courante : https://unity.com/resources/shedworks-sable-modular-design-approach ; Gregorios Kythreotis, **GDC 2022**, résumé officiel : https://www.gdcvault.com/play/1027721/The-Art-of-Sable-Imperfection
- **S24 — Unity, Main Light et Additional Lights**, manuel **6000.3**, courant : https://docs.unity.com/en-us/engine/6000.3/manual/analysis/graphics-performance-profiling/in-urp/understand-performance/rendering-debugger/reference
- **S25 — Meshy, Image to 3D API**, documentation courante **7.1 / T2 / T2.5** : https://docs.meshy.ai/en/api/image-to-3d
- **S26 — Meshy, Remesh API**, documentation courante : https://docs.meshy.ai/en/api/remesh
- **S27 — Unity, coût et optimisation des ombres**, manuel **6000.3**, courant : https://docs.unity.com/en-us/engine/6000.3/manual/lighting-overview/lighting/shadows-in-urp/shadows-optimization

[S1]: https://github.com/Unity-Technologies/com.unity.toonshader/blob/master/com.unity.toonshader/CHANGELOG.md
[S2]: https://github.com/Unity-Technologies/com.unity.toonshader/releases
[S3]: https://unity.com/legal/licenses/unity-companion-license
[S4]: https://docs.unity3d.com/Packages/com.unity.shadergraph@17.3/manual/Shader-Graph-Sample-Custom-Lighting-Introduction.html
[S5]: https://docs.unity.com/en-us/engine/6000.3/manual/lighting-overview/lighting/custom-lighting/introduction
[S6]: https://assetstore.unity.com/packages/vfx/shaders/toony-colors-pro-2-8105
[S7]: https://www.jeanmoreno.com/unity/toonycolorspro/doc/
[S8]: https://www.jeanmoreno.com/unity/toonycolorspro/doc/changelog
[S9]: https://assetstore.unity.com/packages/vfx/shaders/realtoon-pro-anime-toon-shader-65518
[S10]: https://github.com/lilxyzw/lilToon/blob/master/Assets/lilToon/CHANGELOG.md
[S11]: https://github.com/lilxyzw/lilToon/issues/400
[S12]: https://flatkit.dustyroom.com/release-notes/
[S13]: https://flatkit.dustyroom.com/stylized-surface/
[S14]: https://unity.com/legal/as-terms
[S15]: https://github.com/ColinLeung-NiloCat/UnityURPToonLitShaderExample
[S16]: https://github.com/Unity-Technologies/com.unity.toonshader/blob/master/com.unity.toonshader/Documentation~/Outline.md
[S17]: https://docs.unity.com/en-us/engine/6000.3/manual/post-processing-and-full-screen-effects/urp/custom-post-processing/renderer-feature-full-screen-pass
[S18]: https://www.youtube.com/watch?v=G1NY0LKDqJo
[S19]: https://docs.unity.com/en-us/engine/6000.3/manual/animation-section/animation-mecanim/animation-clips/animation-editor-guide/blend-shapes
[S20]: https://helpx.adobe.com/creative-cloud/faq/mixamo-faq.html
[S21]: https://media.gdcvault.com/gdc2015/presentations/Motomura_Junya_GuiltyGearXrd.pdf
[S22]: https://gdcvault.com/play/1034330/3D-Toon-Rendering-in-Hi
[S23]: https://unity.com/resources/shedworks-sable-modular-design-approach
[S24]: https://docs.unity.com/en-us/engine/6000.3/manual/analysis/graphics-performance-profiling/in-urp/understand-performance/rendering-debugger/reference
[S25]: https://docs.meshy.ai/en/api/image-to-3d
[S26]: https://docs.meshy.ai/en/api/remesh
[S27]: https://docs.unity.com/en-us/engine/6000.3/manual/lighting-overview/lighting/shadows-in-urp/shadows-optimization
