# Pipeline 3D et animation pour Dubito — 27 novembre 2026

Recherche pour la phase DA de la [ROADMAP](../ROADMAP.md), sans décision d'achat ni validation artistique. Prix consultés sur les sites officiels pendant cette session, en USD sauf indication contraire ; taxes et montant du panier non inclus dans les budgets. Une donnée inaccessible, contradictoire ou non confirmée est marquée **à vérifier**.

## 1. Besoin de Dubito et méthode

La contrainte est un ensemble de personnages simples partageant un squelette : voir [VISION, Direction artistique](../VISION.md#direction-artistique). Les choix antérieurs restent dans [DECISIONS D-38, D-43, D-53 et D-67](../DECISIONS.md) ; cette recherche ne les remplace pas. Dimensions de la scène : [ASSETS §2](../ASSETS.md#2-dimensions-de-référence).

Objectif : une image de référence devient un personnage préparé dans Blender ; une réaction assise filmée à l'iPhone devient un clip transféré sur ce personnage dans Unity. « Compatible Unity » ne prouve ni qu'un squelette Humanoid sera valide ni que les mains toucheront correctement la table. Les appréciations ci-dessous sont des analyses de pipeline, pas des essais sur nos personnages. L'intégration future d'assets externes reste soumise à AGENTS, règle 8.

## 2. Meshy AI : image vers personnage ou objet

### 2.1 Prix et droits

| Offre | Paiement mensuel, USD | Paiement annuel affiché le 27/11/2026 | Crédits par mois |
|---|---:|---:|---:|
| Free | 0 | 0 | 100 |
| Pro | 20 $ | 144 $ immédiatement, soit 12 $/mois | 1 000 |
| Premium | 40 $ | 240 $ immédiatement, soit 20 $/mois | 3 000 |
| Ultra | 100 $ | 960 $ immédiatement, soit 80 $/mois | 8 000 |

Les annuels sont la promotion Black Friday du 23 novembre au 2 décembre 2026 ; prix de renouvellement **à vérifier**. Studio : annuel affiché 672 $ la première année puis 840 $/an, 5 500 crédits partagés ; Enterprise sur devis. Free : CC BY 4.0, usage commercial avec attribution, jusqu'à 10 téléchargements Meshy 6 Lite/mois. Payant : modèles privés, propriété annoncée au client, téléchargements illimités. Les crédits mensuels expirent au renouvellement. [Tarifs officiels](https://www.meshy.ai/pricing). Les mensuels hors remise sont aussi dans le [comparatif officiel](https://help.meshy.ai/en/articles/12062933-which-meshy-plan-is-right-for-you-free-vs-pro-vs-premium-vs-ultra).

### 2.2 Crédits : version et interface changent le coût

La grille web distingue l'Agent (image texturée Standard : 30 crédits ; Smart Topology : 15) et l'outil Model (forme Meshy 7 : 25, Meshy 6 : 20, Lite : 10 ; texture 2K/4K : +10 ; UV : +5). Une image texturée Meshy 7 peut donc coûter **35 crédits** dans Model. Le montant montré avant lancement fait foi. [Coûts web officiels](https://help.meshy.ai/en/articles/10000507-how-many-credits-does-each-generation-task-cost).

La documentation web actuelle précise **remesh 0, rigging 0, animation de bibliothèque 0 crédit** ; Smart Topology 15, et texture à la génération +10. L'accès aux fonctions reste soumis au plan. Elle cite Meshy 7.1 à 25 crédits par défaut en Ultra 2K sur les offres payantes hors Enterprise, donc **35 avec texture**. Le centre d'aide et la documentation ne nomment pas toutes les versions pareil : conserver le devis montré avant lancement. [Grille web détaillée](https://docs.meshy.ai/en/webapp/pricing).

L'API affiche Meshy 6/7 texturé 2K/4K à 30 crédits, +5 pour Ultra de Meshy 7 ; remesh 5, UV 5, auto-rig 5, animation 3. Ce sont les **tarifs API**, différents du site web. [Barème API officiel](https://help.meshy.ai/en/articles/16815622-how-many-credits-does-each-meshy-api-task-cost).

### 2.3 Squelette, topologie, UV et limites

L'auto-rig accepte un humanoïde en A-pose ou T-pose, membres séparés ; export FBX ou GLB, squelette Mixamo proposé par défaut. Un rig facial personnalisé doit être fait dans Blender. Au-delà de 300 000 faces, remesh obligatoire avant rig. [Guide officiel](https://docs.meshy.ai/en/webapp/guides/3d-model/rigging).

**Analyse pour Dubito :** générer debout en pose neutre, puis asseoir par animation. Une image de personnage déjà assis risque de souder cuisses, mains, vêtement et chaise. Un remesh ne garantit pas des articulations propres aux épaules et aux doigts. Des UV générés ne garantissent pas une densité régulière ni des coutures invisibles. Contrôler doigts séparés, paumes, bouche, paupières, dents et symétrie ; prévoir corrections de géométrie et de poids dans Blender. Aucun taux de réussite n'est établi ici.

Garder une armature commune et adapter les variantes autour de celle-ci. Les noms Mixamo facilitent le transfert mais ne prouvent pas que proportions, orientations et poses soient identiques. Unity Humanoid exige une validation de l'Avatar après import. Le rire exige aussi un visage animé : le mouvement du torse seul ne l'apporte pas.

### 2.4 API

L'API est intéressante pour traiter un lot, enregistrer paramètres et coûts, puis récupérer les fichiers ; elle n'améliore pas à elle seule la qualité des modèles. Pour 21 assets, l'interface web suffit à l'évaluation. Ne pas développer une intégration avant d'avoir un personnage réussi. Les tarifs API ci-dessus donnent seulement une base distincte de comparaison.

### 2.5 Estimer six personnages et quinze objets

Hypothèse de travail : **35 crédits par essai entièrement texturé**, puis 10 par retouche de texture ; remesh et rig web à 0 d'après la grille ci-dessus. Chaque « essai » est une génération complète, même si elle est rejetée. Une relance gratuite éventuelle n'est pas déduite ; une itération peut aussi ne nécessiter que la forme, donc cette méthode conserve une marge. Les objets simples pourront être faits directement par Codex dans Blender.

| Scénario estimé | Essais | Calcul | Crédits avec réserve | Achat permettant ce volume |
|---|---:|---|---:|---|
| Économe | 6 × 3 personnages + 15 × 2 objets = 48 | 48 × 35 + 10 retouches × 10 = 1 780 ; +20 % | 2 136 | Premium un mois, 40 $ ; ou 3 mois Pro si l'on préfère étaler, 60 $ |
| Central | 6 × 4 + 15 × 3 = 69 | 69 × 35 + 15 × 10 = 2 565 ; +15 % | 2 950 arrondis | Premium un mois, 40 $, quota presque entièrement utilisé |
| Beaucoup de reprises | 6 × 6 + 15 × 5 = 111 | 111 × 35 + 21 × 10 = 4 095 ; +20 % | 4 914 | Deux mois Premium, 80 $, en répartissant les travaux ; ou Ultra un mois, 100 $ |

Ces volumes ne comprennent pas les images Nano Banana, les autres outils ni le temps de correction. Les crédits se renouvellent chaque mois : deux mois Premium ne donnent pas un lot de 6 000 crédits utilisable le premier jour. Pour un **test DA d'un personnage + un objet**, quatre essais chacun et quelques retouches restent largement sous 1 000 crédits : Pro un mois, 20 $, suffit. Les montants en euros et le budget global figurent en §7.

## 3. Alternatives à Meshy

| Outil | Prix officiel relevé | Droits et intérêt pour Dubito |
|---|---|---|
| **Tripo** | Page actuelle : Pro 20 $/mois ou 240 $/an, 3 000 crédits/mois ; Max 90 $/mois ou 1 080 $/an, 25 000 crédits. Free : 200 crédits. | Page actuelle : Free public et **non commercial** ; Pro privé et commercial. Remesh, multi-vues et rig disponibles selon offre ; alternative directe à comparer sur la même référence. |
| **Rodin / Hyper3D** | Creator 30 $/mois ou 288 $/an ; Business 120 $/mois ou 1 152 $/an. | Creator annonce modèles privés et exports « any use », smart low-poly, normales cuites et textures HD. Business ajoute API complète et high-poly quads. Intéressant si Meshy manque la silhouette ; pas une garantie de personnage déformable. |
| **Unity AI** | Essai 14 jours ; Personal annonce 1 000 crédits mensuels dans l'offre AI, abonnement ensuite : prix exact **à vérifier** (panier non lisible). | Les générateurs CLI actuels font modèles, rig et mouvement via fournisseurs tiers ; chaque fournisseur garde ses conditions. Intérêt : intégrer le flux ; qualité dépend du modèle choisi, pas du nom Unity. |
| **Hunyuan3D local** | Pas de redevance par génération dans la licence communautaire ; matériel, électricité et préparation restent à charge. | 2.0 : 6 Go annoncés pour la forme, 16 Go pour forme + texture. 2.1 : 10 / 21 / 29 Go pour forme / texture / ensemble. La RTX 3070 8 Go peut théoriquement tenter la forme 2.0 seule ; pas le pipeline texturé standard. **Licences 2.0 et 2.1 excluant l'UE : non retenu pour notre usage local en France.** |

Sources : [Tripo, prix et droits actuels](https://www.tripo3d.ai/pricing), [Rodin, offres](https://hyper3d.ai/pricing), [Unity, offres](https://unity.com/products), [Unity, générateurs CLI](https://docs.unity.com/en-us/ai/generators-cli/about-generators-cli), [Hunyuan3D 2.0, mémoire](https://github.com/Tencent-Hunyuan/Hunyuan3D-2), [2.1, mémoire](https://github.com/Tencent-Hunyuan/Hunyuan3D-2.1), [licence 2.0](https://raw.githubusercontent.com/Tencent-Hunyuan/Hunyuan3D-2/main/LICENSE), [licence 2.1](https://raw.githubusercontent.com/Tencent-Hunyuan/Hunyuan3D-2.1/main/LICENSE).

**Divergence Tripo :** sa documentation d'octobre donne encore Professional 13 $/mois en annuel et Free commercial avec attribution ; la page tarifaire actuelle indique les conditions différentes ci-dessus. Retenir la restriction actuelle pour préparer un achat ; droits applicables et tarif du panier **à vérifier**, sans considérer Free comme acquis pour Steam. La même documentation donne 15 crédits par forme HD, textures en supplément, rig 20 et UV 20 : ces coûts sont indicatifs et ne sont pas interchangeables avec les crédits Meshy. [Documentation officielle Tripo](https://www.tripo3d.ai/product-doc/getting-started/pricing-credits).

Unity documente à titre d'exemple 38–194 crédits par modèle, 13–95 pour rig/retopo/retexture et environ 6 par seconde pour vidéo vers animation ; ce ne sont pas des tarifs fixes. [Crédits Unity](https://docs.unity.com/en-us/ai/credits/credits-about). Son article de mai sur le générateur d'objets concerne des formes statiques pour prototypes : ne pas en déduire qu'il livre un personnage animé final. [Générateur d'objets Unity](https://unity.com/blog/unity-ai-3d-object-generator).

**Analyse de qualité :** Tripo et Rodin sont des candidats, pas des vainqueurs établis. Comparer silhouette de face/profil, arrière inventé par l'IA, UV, texture sans éclairage peint, doigts et déformation assise. Hunyuan n'élimine ni retopologie ni rig. Les variantes locales à faible mémoire ne suppriment pas les conditions de licence.

## 4. Vidéo iPhone vers animation assise

### 4.1 Tableau des offres et exports

| Outil | Prix, quota et durée vérifiés ou réserves explicites | Export et squelette | Mains, assise et droits |
|---|---|---|---|
| **Move.ai / Move One** | Aide officielle : Starter 18 $/mois + TVA, 60 crédits ; Standard 48 $, 180 ; Plus 225 $, 240 ; Advanced 490 $, 700. Un crédit = 1 s en s1, 2 en s2. Gratuit : 30 crédits une seule fois ; clips 30 s gratuits / 60 s payants. **Tarifs actuels à vérifier** : ancien lien pricing en erreur ; annonce officielle plus récente contradictoire, voir dessous. Annuel à vérifier. | FBX pour le transfert ; la documentation Genesis propose aussi BVH, GLB et Blender. Ne pas attribuer automatiquement tous les exports Genesis à Move One iPhone. Squelette Move à recibler, sans garantie d'identité Mixamo. | Corps et doigts annoncés. Bon candidat technique pour un geste avec mains visibles ; pas de preuve ici sur une personne assise. Licence d'utilisation des sorties prévue par l'EULA, avec restrictions de sous-licence : vérifier la distribution dans le jeu. |
| **Rokoko Vision 3.0 → Create / Studio** | Starter 0 $, 30 s de vidéo/mois ; Basic 12 $/mois ou 120 $/an, 600 s/mois ; Plus 28 $/mois ou 240 $/an, 3 000 s ; Pro 70 $/mois ou 600 $/an, 15 000 s. Durée maximale par fichier : **à vérifier**. | FBX dès Starter ; BVH annoncé dans Basic ; préréglages Mixamo, HIK, etc. Nettoyage et retargeting personnalisé dans Basic. La FAQ tarifaire contredit le tableau sur le palier BVH : privilégier FBX pour notre essai. | **Pas de suivi des doigts au lancement de Vision 3.0.** Le bassin et les bras cachés par une table restent à corriger. Conditions consultées : pas de restriction non commerciale repérée pour nos propres captures ; voir réserve juridique ci-dessous. |
| **DeepMotion Animate 3D** | Grille annuelle indexée officielle : Starter 9 $/mois (180 crédits), Innovator 17 $ (480), Professional 39 $ (1 500), Studio 83 $ (illimité, 7 200 prioritaires). Soit 108 / 204 / 468 / 996 $/an par multiplication. **Mensuels sans engagement à vérifier** : cartes absentes de la page ouverte. Clips max 20 / 30 / 120 / 360 s. | FBX/BVH ; GLB pour personnages personnalisés. Squelette fourni ou personnage importé ; transfert vers notre rig à contrôler. | Corps 1 crédit/s, mains +0,5, visage +0,5. Gratuit non commercial. Grille Free : 25 crédits/mois et 20 s/clip ; ancienne FAQ : 60 s/mois, quota à vérifier. Commercial autorisé aux offres payantes. |
| **Plask** | Free : 15 s/jour. Standard 18 $/mois facturés annuellement (216 $/an), 10 min/mois ; Pro 50 $ équivalents (600 $/an), 1 h/mois. Mensuel sans engagement et durée max par fichier : **à vérifier**. | FBX, BVH, GLB annoncés ; modèles personnels importables. Squelette à mapper, pas une garantie de rig Mixamo identique. | Corps et mains sur la page tarifaire, mais suivi des doigts **V1 seulement** dans la FAQ. Conditions commerciales insuffisamment raccordées aux noms des offres actuelles : **à vérifier**, y compris Free. |
| **Meshcapade** | Accueil officiel indexé : plateformes en ligne fermées le 18 avril, société désormais chez Epic Games. Page ouverte bloquée 403 ; ancien prix sans valeur pour un achat actuel. | Anciens exports FBX/GLB, anciens modèles de corps de famille SMPL. Accès et droits actuels **à vérifier**. | Ancienne offre corps/doigts ; **ne pas budgéter un abonnement disponible aujourd'hui**. |

Sources tarifaires : [Move One, aide](https://docs.move.ai/knowledge/move-one-pricing), [Rokoko, tarifs](https://www.rokoko.com/pricing), [DeepMotion, grille officielle](https://www.deepmotion.com/pricing-animate3d), [Plask, tarifs](https://plask.ai/en-US/pricing), [Meshcapade, avis officiel indexé](https://meshcapade.com/?source=himalayas.app). Les multiplications annuelles DeepMotion/Plask sont des calculs, pas des totaux de panier observés.

Move a annoncé le 18 septembre 2025 des plans personnels dès 15 $/mois et Advanced à 250 $ avec environ 6 000 s s1 ou 3 000 s s2 : cela contredit l'aide encore en ligne. **Aucun de ces deux jeux de chiffres n'est suffisamment sûr pour notre budget.** [Annonce officielle Move](https://www.youtube.com/watch?v=Xiv_Jrq5hPU). Exports modernes et doigts : [dépôt Move](https://github.com/move-ai/recipes), [formats Genesis](https://docs.move.ai/knowledge/genesis-output-formats).

### 4.2 Ce que les licences permettent réellement de conclure

- **Move :** l'EULA autorise l'import des sorties dans un moteur et envisage l'exploitation commerciale ; elle encadre la sous-licence et les usages IA. Ce n'est pas une autorisation de publier librement les FBX bruts. [EULA Move One, §7 et définitions](https://move.ai/move-one-app-eula).
- **Rokoko :** les conditions de juillet 2026 définissent les animations produites comme contenu utilisateur et ne revendiquent pas leur propriété. Pour nos propres performances, l'usage en jeu paraît cohérent avec ces conditions ; c'est une **lecture**, pas une garantie juridique ni une licence sur leur bibliothèque. Archivage des conditions de l'offre au moment du choix recommandé. [Conditions §3–4](https://www.rokoko.com/terms-of-use).
- **DeepMotion :** les conditions accordent une licence commerciale perpétuelle aux sorties créées pendant Starter ou une autre offre payante, utilisables dans un jeu compilé ; distribution des fichiers autonomes interdite. Un essai Free ne devient donc pas automatiquement un clip commercial après abonnement : le produire sous le bon plan. [Conditions, Products of the Site](https://www.deepmotion.com/terms-of-use).
- **Plask :** la FAQ dit que les animations des abonnés leur appartiennent, mais les conditions parlent d'un plan « Commercial » absent de la grille actuelle. Confirmer quelle offre couvre un jeu Steam avant de choisir ce service. [FAQ](https://plask.ai/en-US/docs/faqs), [conditions, Commercial Users](https://plask.ai/en-US/docs/100002).

### 4.3 Qualité assise : classement de test, pas verdict commercial

**Rokoko en premier pour les mouvements du corps**, parce que le FBX d'essai est gratuit et Basic offre 10 minutes de vidéo par mois à faible coût. Ses doigts devront être posés/animés dans Blender. Vision 3.0 utilise un seul fichier vidéo ; les anciennes fonctions double caméra et webcam ont été retirées. [Limites officielles Vision](https://www.rokoko.com/products/vision).

**DeepMotion en second si les doigts sont déterminants** : sa FAQ accepte le cadrage demi-corps avec mains visibles, recommande au moins 1080p et signale les difficultés pour mains sur le visage et mains en contact entre elles. « Réfléchir, main au menton » est donc précisément un geste à tester et à corriger. Cette FAQ date de 2022 : vérifier si les défauts subsistent dans le modèle actuel. [Suivi des mains DeepMotion](https://www.deepmotion.com/post/animate-3d-hand-tracking-faq).

**Move One** reste à comparer si son accès et son prix sont clarifiés ; les annonces de doigts ne prouvent pas un contact stable avec la table. **Plask** est une solution de repli : comparer V1 avec doigts et V2 pour le corps, sans supposer que V2 ait les mêmes fonctions. Aucun de ces outils ne garantit l'absence de glissement du bassin, d'inversion d'avant-bras ou de tremblement des doigts quand la chaise et la table cachent le corps. Le visage nécessite un traitement distinct de l'animation Humanoid.

### 4.4 Autre option commerciale : Uthana à l'usage

**Video-to-Motion 3.0 : 0,06 $ par seconde de sortie**, 2.0/2.1 : 0,05 $. Pas d'abonnement obligatoire ; téléchargements illimités et crédits achetés sans expiration. Ainsi, 4 réactions × 3 prises × 8 s = 96 s coûtent 5,76 $ en 3.0 si la sortie conserve la même durée ; 4 minutes avec reprises coûtent 14,40 $. Le minimum de recharge éventuel du panier est **à vérifier** : ce sont des coûts consommés, pas la preuve qu'on peut payer exactement 5,76 $. [Tarifs officiels](https://uthana.com/pricing).

Une personne, vidéo de 2–60 s à 24–120 images/s, export FBX/GLB/BVH à 24/30/60. Corps bipède et doigts ; 3.0 ajoute un traitement des mains. Squelette à recibler sur notre référence. Le fournisseur recommande corps entier visible et démarrage debout : **une capture exclusivement assise reste à démontrer** ; prévoir un début debout, s'asseoir puis couper cette partie dans Blender. [Spécifications et limites](https://uthana.com/models/video-to-motion).

Les conditions §4 permettent les sorties commerciales ; premier achat de crédits : droits sans attribution sur les sorties du compte, y compris antérieures ; avant achat, attribution contractuelle requise. Cela ne transfère pas les droits de tiers éventuels. [Conditions officielles](https://uthana.com/terms). Pour quelques réactions avec doigts, c'est un essai complémentaire raisonnable ; aucun résultat comparatif n'a été mesuré ici.

### 4.5 Options locales sur la RTX 3070

| Option | Coût, GPU et export | Limites et licence |
|---|---|---|
| **FreeMoCap** | Logiciel gratuit ; le README actuel propose un suivi NVIDIA CUDA sur Windows/Linux. Le site décrit un système calibré à **2 caméras ou plus**, puis des résultats FBX et Blender. | Candidat local crédible si l'on accepte plusieurs vues et la calibration ; pas l'équivalent immédiat d'un seul iPhone. Code AGPL-3.0 ; licences du suivi et des modèles à contrôler dans la version choisie avant usage commercial. La licence du programme ne signifie pas automatiquement que les animations imposent l'AGPL au jeu. |
| **MediaPipe Pose + Hand Landmarker** | Bibliothèque sans abonnement ; pose à 33 points, GPU proposé notamment sur le Web. Il faut construire le transfert vers os et l'export dans Blender. | Des points 3D estimés ne sont pas un FBX animé prêt à l'emploi. Profondeur, rotations et contacts à reconstruire ; mains au visage toujours problématiques. Licence des bibliothèques, poids et dépendances à vérifier avant de bâtir le pipeline. |
| **GVHMR** | Reconstruction de mouvement depuis vidéo avec code local ; installation CUDA et conversion de sortie nécessaires. Mémoire exacte et exécution complète sur 8 Go **à vérifier**, aucun benchmark effectué. | Licence du dépôt réservée recherche/enseignement/non lucratif, commercial sur accord séparé. Dépendances SMPL et poids à examiner également. **Pas une solution gratuite autorisée pour livrer Dubito commercialement.** |

Sources : [FreeMoCap, GPU et licence](https://github.com/freemocap/freemocap), [FreeMoCap, caméras et formats](https://freemocap.org/), [MediaPipe, sortie et exemples GPU](https://ai.google.dev/edge/mediapipe/solutions/vision/pose_landmarker), [GVHMR](https://github.com/zju3dv/GVHMR), [licence GVHMR](https://raw.githubusercontent.com/zju3dv/GVHMR/main/LICENSE). L'accélération CUDA documentée ne constitue pas un test sur notre PC. Tout calcul lourd local qui la permet doit utiliser la RTX 3070, conformément à AGENTS règle 9.

Pour quatre réactions stylisées, l'animation manuelle dans Blender à partir de la vidéo peut demander moins d'effort que l'installation et la réparation d'un outil de recherche. C'est une hypothèse de production à mesurer sur un clip, sans abonnement.

## 5. Pipeline concret et tournage

### 5.1 Du modèle au clip Unity

1. **Référence** : personnage entier de face et de profil, même vêtement et proportions, A/T-pose, mains séparées du corps ; pas de chaise dans l'image de génération. Créer d'abord une variante, avant les six.
2. **Modèle Blender** : nettoyer les surfaces, optimiser le nombre de faces et les matériaux, contrôler UV/textures ; garder les pièces qui bougent séparément. Construire ou corriger une armature d'export commune, mains comprises. Le rig facial et les expressions sont des besoins distincts.
3. **Source de mouvement** : conserver la vidéo et le FBX/BVH bruts. Préférer FBX si les doigts existent. BVH porte le squelette et le mouvement, pas les textures ; les conventions d'axes et de noms varient selon l'export.
4. **Transfert dans Blender** : importer l'armature source à côté de la cible, régler échelle, orientation, cadence et pose neutre ; faire correspondre bassin, colonne, tête, bras, jambes et doigts. Transférer les rotations avec leurs corrections de pose, puis enregistrer le résultat image par image sur les os d'export. Ne pas seulement renommer les os.
5. **Correction assise** : stabiliser bassin sur l'assise et pieds au sol, corriger coudes/épaules, poser les mains à la bonne hauteur de table. Pour une carte tenue, faire une pose de doigts volontaire plutôt que conserver du tremblement. Corriger les contacts avec IK (contraintes de position), puis cuire le résultat sur le squelette exporté. Conserver les gestes expressifs du torse et de la tête.
6. **Clips** : une action par animation : repos, doute, rire, déception, réflexion. Seul le repos doit boucler ; les réactions reviennent proprement au repos. Exporter un FBX personnage en pose neutre et un FBX par animation, avec les os utiles, animations cuites et sans os supplémentaires automatiques inutiles.
7. **Unity 6.3** : personnage en Humanoid avec Avatar valide et pose correcte. Pour chaque clip, réutiliser l'Avatar uniquement si le squelette est identique ; sinon créer l'Avatar de la source pour le transfert Humanoid. Conserver le déplacement sur place et vérifier bassin/hauteur ; un réglage automatique de root motion ne résout pas les contacts. Les expressions faciales par déformation de maillage demandent leur propre import et animation. Matériaux convertis pour URP.
8. **Revue** : vue depuis un siège, vue latérale, six variantes ensemble ; vérifier contacts, doigts, visage, absence de tremblement et transitions. Le propriétaire valide le résultat visible. Si une variante casse les contacts, corriger ses proportions ou l'animation ; partager un Avatar ne garantit pas les mêmes positions des mains.

Méthode de projet : [skill animation](../../.agents/skills/blender-animation/SKILL.md), [rigging](../../.agents/skills/blender-character-rigging/SKILL.md), [import personnage](../../.agents/skills/dubito-character-import/SKILL.md). Fondement du transfert Humanoid : [manuel officiel Unity 6.3](https://docs.unity3d.com/6000.3/Documentation/Manual/Retargeting.html). Ce pipeline est proposé, pas exécuté dans cette recherche.

### 5.2 Filmer les réactions assises avec un iPhone

- Téléphone fixe sur support, objectif principal, image horizontale 1080p ; 30 images/s suffisent pour des gestes lents, 60 si le service et le quota le permettent. Éviter le mode Cinématique, les filtres et le zoom qui change pendant la prise. Ces réglages sont une proposition de tournage pour Dubito.
- Lumière uniforme, fond simple contrasté, vêtements près du corps, poignets et mains dégagés, pas de gants. Une seule personne, entièrement dans le cadre pour les outils corps entier ; garder assez de marge au-dessus des mains levées. Les détails de doigts exigent une bonne taille dans l'image, pas seulement une vidéo 4K.
- Chaise stable sans accoudoirs. Faire une première prise sans table qui cache les jambes et les avant-bras ; mimer la hauteur du plateau. Garder une prise de référence avec la vraie table pour reconstruire ensuite les contacts. Comparer face et léger trois-quarts, sans supposer qu'un angle convienne à tous les solveurs.
- Une réaction lisible de 3–6 s, plus environ 1 s immobile avant/après ; préparer trois prises de chaque. Si le service exige un début debout (Uthana), enregistrer une prise plus longue, s'asseoir, agir, puis couper l'animation après extraction. Respecter les 20 s/clip de DeepMotion Starter.
- Doute : inclinaison de tête et paume ouverte ; rire : torse et épaules ; déception : épaules qui tombent ; réflexion : main d'abord à distance du menton, contact corrigé ensuite. Éviter de croiser les mains ou de cacher le visage pendant l'essai.
- Réaliser une prise de contrôle où les mains restent séparées et visibles, puis la vraie réaction. Cela permettra de distinguer un défaut de capture d'un défaut du rig.

Ces conseils adaptent les [recommandations Uthana](https://uthana.com/models/video-to-motion), les [limites d'occultation Rokoko](https://www.rokoko.com/products/vision) et les [conseils mains DeepMotion](https://www.deepmotion.com/post/animate-3d-hand-tracking-faq). La hauteur de chaise/table à reproduire reste celle d'[ASSETS §2](../ASSETS.md#2-dimensions-de-référence).

## 6. Higgsfield : images, vidéo et extension Blender

Pour Dubito, les fonctions image/vidéo servent aux recherches de style, références Nano Banana et plans de communication. Une vidéo générée ne devient pas à elle seule une animation 3D transférable ; les captures du jeu réel restent le moyen de montrer le résultat jouable.

**Higgsfield propose aussi une véritable extension Blender**, et pas seulement des images : Scene Builder place des objets modifiables ; 3D Model génère un maillage ; Character Animation annonce une armature pondérée et des clés d'animation sur la timeline. Compatibilité annoncée Blender 5.1+, Windows/macOS ; leur FAQ cite **Meshy 5 pour la génération 3D**. [Extension officielle](https://higgsfield.ai/plugins/blender).

L'éditeur annonce UV/quads, matériaux PBR et actions Blender éditables ; ce sont des promesses de fournisseur, sans test ici sur le personnage Dubito. L'extension utilise le même compte/crédits, sans abonnement séparé. Prix d'une génération 3D ou animation, suivi d'une vidéo iPhone, squelette commun et export Unity Humanoid : **à vérifier**. [Guide officiel du 28 octobre](https://higgsfield.ai/blog/higgsfield-blender-plugin). La documentation précise que l'illimité web ne s'étend **pas aux extensions**. [Intégrations et limites](https://higgsfield.ai/creator-hub/help-center/integrations/external-integrations-higgsfield).

**Analyse pour Dubito :** utile à comparer si le propriétaire possède déjà Higgsfield pour les images, car il pourrait regrouper certains outils. Pas de gain démontré justifiant un abonnement supplémentaire face à Codex + Blender et Meshy direct ; la version Meshy citée par cette extension est différente de celle tarifée en §2. Il faut comparer le devis et le fichier modifiable, pas une démo vidéo. L'assemblage automatique d'une scène exige aussi de vérifier la provenance des objets proposés.

| Offre individuelle, USD | Crédits/mois | Mensuel | Annuel affiché |
|---|---:|---:|---:|
| Starter | 270 | 19 $ | 228 $ |
| Plus | 1 200 | 40 $ premier mois, puis 59 $ | 360 $ (30 $/mois équivalents) |
| Ultra | 3 000 | 75 $ premier mois, puis 129 $ | 480 $ (40 $/mois équivalents) |

Grille officielle Markdown datée **2026-11-27**, prix pour les États-Unis ; tarifs français **à vérifier**. Promotion Black Friday annoncée jusqu'au 28/11 à 00:00 UTC. Les offres « illimitées » ont leurs propres modèles, durées et restrictions, et ne s'appliquent pas au MCP/CLI. [Tarifs officiels lisibles](https://higgsfield.ai/pricing.md), [page régionale](https://higgsfield.ai/pricing).

Les conditions §4.4 ne revendiquent pas la propriété de nos entrées/sorties et n'interdisent pas leur exploitation commerciale ; elles ne garantissent pas l'exclusivité. Une ancienne grille indexée Free/Basic/Pro/Max annonce d'autres restrictions : droits exacts de l'offre Free actuelle **à vérifier**, ne pas les extrapoler. [Conditions actuelles](https://higgsfield.ai/terms-of-use-agreement). **Recommandation : 0 $ pour la 3D en jeu** ; éventuellement un mois Starter pour un besoin d'images/vidéo précisément défini, si l'accès Nano Banana déjà choisi ne le couvre pas. Le coût du service d'images existant n'est pas connu ici.

## 7. Budget et recommandation

### 7.1 Budget proposé, sans engagement annuel

Conversion indicative : **1 € = 1,0957 USD**, cours affiché par la BCE au 27 novembre 2026 ; euros = dollars ÷ 1,0957, arrondis. Ce n'est pas le taux de carte bancaire ni un prix TTC. [BCE, USD](https://www.ecb.europa.eu/stats/policy_and_exchange_rates/euro_reference_exchange_rates/html/eurofxref-graph-usd.en.html).

| Étape et hypothèse | Composition | Total USD | Équivalent EUR hors taxes |
|---|---|---:|---:|
| DA : démontrer le pipeline sur un personnage et une réaction | Meshy Pro 1 mois (20) + Rokoko Basic 1 mois (12) | 32 $ | ≈ 29,21 € |
| V1 après validation, scénario central §2.5 | Meshy Premium 1 mois (40) + Rokoko Basic 1 mois (12) | 52 $ supplémentaires | ≈ 47,46 € |
| **DA + V1 centrale** | Deux phases ci-dessus | **84 $ cumulés** | **≈ 76,66 €** |
| V1 avec reprises et travail réparti sur 2 mois | Meshy Premium 2 mois (80) + Rokoko Basic 2 mois (24) | 104 $ supplémentaires | ≈ 94,92 € |
| **DA + V1 avec reprises** | DA 32 + V1 104 | **136 $ cumulés** | **≈ 124,12 €** |

Prix unitaires : [Meshy](https://www.meshy.ai/pricing), [Rokoko](https://www.rokoko.com/pricing). Les crédits et durées sont alloués par mois ; les volumes ci-dessus supposent que la production tienne dans ces périodes. Une V1 plus longue exige soit de nouveaux mois de génération, soit de finir les corrections hors abonnement. Les prix promotionnels de première inscription ne sont pas nécessaires à ce budget.

Si les doigts demandent une comparaison, ajouter **14,40 $ de consommation Uthana** pour 4 minutes de sortie (≈ 13,14 €), soit DA à 46,40 $ (≈ 42,35 €), sous réserve du minimum de recharge. Cela porte DA + V1 centrale à 98,40 $ (≈ 89,81 €). Voir [tarif à l'usage](https://uthana.com/pricing). La capture d'une seconde de plus et chaque nouvelle prise facturée augmentent ce coût. Pour quelques secondes seulement, commencer par le test gratuit Rokoko ; ne pas prendre simultanément plusieurs abonnements de capture.

**Non compris :** TVA, frais de change, abonnements IA déjà détenus, accès Nano Banana, Steam, son, matériel ou travail humain. Ce budget est le coût supplémentaire des outils 3D/animation, pas le coût complet du développement. Sans connaître l'offre d'images utilisée, un total incluant Nano Banana serait inventé. Aucun abonnement permanent n'est requis par le pipeline une fois modèles et animations correctement exportés et les droits acquis.

### 7.2 Ce que l'on peut faire sans nouvel abonnement

- Codex modélise dans Blender depuis les références du propriétaire, conformément au flux D-53 ; les quinze objets simples se prêtent particulièrement à cette méthode.
- Garder les mannequins et clips Mixamo déjà retenus pour les essais. Adobe annonce Mixamo gratuit avec Adobe ID et utilisable commercialement dans un jeu ; limites sur les humanoïdes trop déformés. Aucun nouvel asset n'a été téléchargé ici. [FAQ officielle Adobe](https://helpx.adobe.com/creative-cloud/faq/mixamo-faq.html).
- Animer quatre réactions dans Blender depuis la vidéo comme référence, avec poses de mains et expressions simples ; réutiliser le squelette et les clips sur les variantes.
- Meshy Free permet une évaluation limitée avec attribution ; Rokoko Starter permet quelques secondes de corps. Un export disponible gratuitement n'est pas une preuve de licence commerciale pour les autres services.
- FreeMoCap peut être exploré plus tard si plusieurs caméras sont disponibles ; coût d'installation et de correction à comparer au prix de quelques minutes de service.

### 7.3 Recommandation et critère pour poursuivre

**Prendre un seul mois Meshy Pro (20 $), puis Rokoko Basic un mois (12 $) seulement si l'essai Starter est exploitable.** Budget DA : 32 $ hors taxes (≈ 29 €), avec Blender comme outil central. Garder les mains posées à la main si cela suffit au style ; essayer Uthana à l'usage si leur mouvement exige une capture. DeepMotion est le second comparatif pour mains/visage une fois son mensuel confirmé ; Move attend une clarification des prix, Plask celle des droits. Ne pas souscrire annuellement pendant cette évaluation.

Pour la V1, **un mois Meshy Premium (40 $)** offre plus de marge que trois mois Pro pour les 21 assets du scénario central. Si l'IA conserve mal le style ou produit un personnage coûteux à réparer, revenir au modèle Blender original et fabriquer les variantes sur son squelette. Higgsfield reste hors budget de la 3D jouable ; Hunyuan local n'est pas retenu dans les conditions de licence observées.

Le test DA doit montrer le personnage assis, une réaction courte qui revient au repos, un visage lisible, les mains stables, et le même mouvement sur une seconde variante. Relever le nombre d'essais payés et le temps de correction. Comparer au même clip animé directement dans Blender. **Poursuivre seulement après validation visuelle du propriétaire**, sans changer une décision du projet dans ce document.

### 7.4 Informations encore à vérifier au choix de l'outil

| Sujet | Pourquoi cela reste ouvert |
|---|---|
| Move One : prix, quotas, disponibilité iPhone et dépassements | Aide et annonce officielles contradictoires ; ancien lien tarifaire en erreur. |
| DeepMotion : mensuel, quota gratuit et options exactes par offre | Cartes seulement lisibles dans l'index officiel ; icônes de fonctionnalités non interprétables ; FAQ Free contradictoire. |
| Plask : mensuel, durée max et licence Standard/Pro/Free | La grille et les conditions « Commercial » ne correspondent pas clairement. |
| Tripo : Free commercial et annuel | La page actuelle contredit la documentation d'octobre. |
| Unity AI : prix final, fournisseur et droits de ses sorties | Devis lié au modèle et à l'organisation ; panier public non lisible. |
| Higgsfield : prix région France et Free | Grille Markdown américaine ; anciens noms d'offres encore indexés. |
| Tous les services : résultat assis et mains | Aucun essai de génération ou de capture réalisé dans cette recherche. |

Toutes les sources externes citées sont celles des éditeurs, mainteneurs ou de la BCE ; les analyses de qualité et les scénarios budgétaires sont propres à cette recherche. Aucun achat, compte, installation ou appel à une API payante n'a été effectué.
