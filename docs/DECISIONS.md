# Dubito : journal des décisions

> Chaque décision importante, avec sa raison. On ajoute en bas, on ne réécrit pas l'historique :
> si une décision change, on ajoute une nouvelle entrée qui remplace l'ancienne (« remplace D-07 »).
> **P** = décidé par le propriétaire. **C** = proposé par Claude, à confirmer en relecture.

## 2026-11-26 : reprise du projet sous Unity

| ID | Décision | Pourquoi | Qui |
|---|---|---|---|
| D-01 | Reprise de zéro sous **Unity**. Le projet Unreal est abandonné, seules les idées de design sont gardées. | La tentative Unreal était trop lourde et mal organisée. Unity est plus adapté au développement assisté par IA. | P |
| D-02 | Base des règles : **version de juillet 2026** (valeur de manche imposée). | Plus cohérente que la version de juin, où la valeur était libre à chaque coup. | P |
| D-03 | **Pas de bluff sur le nombre de cartes en V1.** Le nombre posé est vrai et visible, on ne ment que sur la valeur. | Simplifie fortement les règles, l'affichage (mains visibles en 3D) et le réseau. Le bluff sur le nombre est gardé pour une version plus avancée. | P |
| D-04 | **Toutes les cartes sont distribuées**, certains ont une carte de plus. | Choix du propriétaire, à surveiller en playtest. | P |
| D-05 | On peut douter **tant qu'on n'a pas confirmé** Jouer ou Défausser. | Évite les doutes perdus sur un simple clic de sélection. | P |
| D-06 | Après un doute juste, le douteur a un **nouveau chrono complet**. | La révélation ne doit pas lui coûter de temps. | P |
| D-07 | **Temps écoulé = tour sauté** (pas de carte posée automatiquement). | Plus simple et plus juste qu'un jeu automatique. Remplace l'auto-jeu de juillet. | P |
| D-08 | **Exclusion au 2e tour sauté** d'affilée. | Les parties ne doivent pas traîner. | P |
| D-09 | Pas de durée maximale en V1. | À observer en playtest ; la Défausse casse déjà les boucles. | P |
| D-10 | La révélation d'un doute est **visible par toute la table**. | C'est le moment fort du jeu. | P |
| D-11 | Direction artistique **reportée** : d'abord des mécaniques qui marchent. | Priorité au jeu fonctionnel. | P |
| D-12 | Personnages **stylisés simples**. | Rapides à produire. Les MetaHumans ont ralenti la version Unreal. | P |
| D-13 | Pas de vocal intégré en V1 (Discord suffit). Pas de bots en V1. | Réduit le périmètre. | P |
| D-14 | Réseau : **Mirror + FizzySteamworks + Steamworks.NET**. | Gratuit, sans service cloud, bien connu des IA, adapté au tour par tour. Repli : Netcode for GameObjects + Unity Multiplayer Services. | P |
| D-15 | **3D basique dès le premier jalon**, à la 1re personne assis, sélection des cartes au clic en 3D. | Choix du propriétaire. Un mode hot-seat sans réseau (phase 2) limite le risque. | P |
| D-16 | **Souris + clavier** seulement en V1. | La manette doublerait le travail d'interface. | P |
| D-17 | **2 à 6 joueurs** en V1. | Table plus lisible à la 1re personne, moins de cas à tester. | P |
| D-18 | Doc en **français**, code et noms techniques en anglais. | Le propriétaire relit et valide. | P |
| D-19 | Le nouveau projet vit dans le dépôt **omarinho167/dubito** (public), le contenu Unreal y est supprimé (il reste dans l'historique). | Choix du propriétaire. | P |
| D-20 | Unity **6.3 LTS** + URP. | LTS supportée jusqu'à fin 2027. 6.0 n'est plus supporté, 6.7 n'est pas encore sortie en LTS. | C |
| D-21 | Le moteur de règles est du **C# pur sans Unity**, testé par `dotnet test`. | Itérations rapides, l'IA peut vérifier ses changements sans l'éditeur. | C |
| D-22 | Un joueur en victoire en attente qui se déconnecte **ne gagne pas** (victoire annulée, partie continue). | Il a quitté avant d'avoir survécu au dernier doute. | C |
| D-23 | Si le menteur (doute juste) ou le douteur (doute faux) part pendant la révélation, **le tas est retiré du jeu**. | Personne ne peut le ramasser. | C |
| D-24 | Un tour sauté rend le dernier coup **non doutable**. | Seul le joueur juste après le poseur peut douter. | C |
| D-25 | Distribution en commençant par le **premier joueur** : il a donc parfois une carte de plus. | Cohérent avec D-04. À surveiller en playtest. | C |
| D-27 | Pendant une révélation, **le doute est résolu avant** les conséquences d'un départ. La fenêtre de doute d'un coup se ferme définitivement au premier geste (ou absence de geste) de son répondant. | Supprime les contradictions relevées en relecture Codex. | C |
| D-28 | Apprentissage : **un seul écran de Règles** (menu + salon). Pas de tutoriel, de bulles d'aide ni d'historique des coups en V1. | Périmètre minimal, le jeu de base est connu. | P |
| D-29 | L'hôte règle la **durée du chrono** (30/45/60/90 s) et les **places max** (2–6). | Utile pour les débutants et les playtests. | P |
| D-30 | Places autour de la table **tirées au hasard**, dans l'ordre du tour. | Simple et équitable. | P |
| D-31 | Skills partagés Claude + Codex dans le dépôt : changer une règle, poser une question au propriétaire, faire relire par Codex. | Rendre les bonnes pratiques répétables. | P |
| D-26 | Garder **Liar's Bar** en tête : pas de copie de sa présentation, la différence vient d'abord des mécaniques. | Risque de perception élevé (voir research/CONCURRENCE.md). | C |

## 2026-11-26 : relecture des décisions C par le propriétaire

| ID | Décision | Pourquoi | Qui |
|---|---|---|---|
| D-32 | D-20, D-21, D-23, D-24, D-26 et D-27 sont **confirmées** telles quelles. | Relecture du propriétaire. | P |
| D-33 | Un joueur en victoire en attente qui se déconnecte : **son dernier coup est révélé** avant de décider s'il gagne (`RULES §7`). Remplace D-22. | Plus juste : on juge le coup, pas le départ. | P |
| D-34 | Les cartes en trop de la distribution vont à des **joueurs tirés au hasard** (`RULES §3`). Remplace D-25. | Choix du propriétaire : plus imprévisible. | P |
| D-35 | Travail à deux IA : **Claude est le chef d'équipe** (parle au propriétaire, conçoit, découpe, intègre), **Codex** prend de grosses tâches de code, des recherches et les relectures. Chacun relit l'autre. | Le propriétaire a un abonnement aux deux ; chacun a ses points forts. | P |
| D-36 | Précisions de D-33 après relecture Codex : le verdict de cette révélation passe avant tout autre départ et avant la règle du dernier joueur restant ; le départ de l'hôte arrête tout, sans révélation (`RULES §6–7`). | Étend D-27 (verdict d'abord) à la nouvelle révélation, sans cas contradictoire. | C |
| D-37 | Migrer **tout de suite** du MCP intégré à Unity (déprécié) vers **Unity CLI**, et installer **en plus** le plugin officiel de skills Unity pour Claude Code et Codex. | Choix du propriétaire. Les skills Unity visent parfois 6.7+ ou Netcode : nos skills Dubito priment (voir `dubito-unity-editor`). | P |
| D-38 | **Pas de générateur IA image → 3D** pour l'instant : la 3D est modélisée par **Codex dans Blender** (scripts, MCP), à partir des références du propriétaire. | Choix du propriétaire. Recherche gardée dans `research/3D-IA.md`. | P |
| D-39 | Le propriétaire **valide la doc de la phase 0** (RULES, CASES, UX, VISION, ARCHITECTURE) et **confirme D-36**. | Fin de phase 0 ; les règles pourront encore évoluer via `dubito-rule-change`. | P |

## 2026-11-26 : définition de la phase 2

| ID | Décision | Pourquoi | Qui |
|---|---|---|---|
| D-40 | Phase 2 en **greybox** (formes simples, couleurs unies), avec **un seul test 3D** réel. Liste des assets : `ASSETS.md`. | Le jeu marche avant d'être beau (D-11), mais on mesure tôt ce que Codex sait modéliser. | P |
| D-41 | Test 3D : **Codex modélise une chaise dans Blender d'après des photos** du propriétaire. | Objet simple mais détaillé, utile autour de la table. | P |
| D-42 | Faces des cartes **générées par script** (52 + dos), valeurs françaises. | Lisibilité maîtrisée, original, aucune licence. | P |
| D-43 | Adversaires : mannequin **Mixamo Y Bot** assis (animations Mixamo), téléchargé par le propriétaire. | Placeholder neutre et animé, sans imposer de style. | P |
| D-44 | Les fichiers Mixamo sont versionnés dans le dépôt public ; le propriétaire assume le risque vis-à-vis des conditions d'Adobe. | Choix du propriétaire. | P |
| D-45 | Décor : **pièce neutre** sombre, une lampe au-dessus de la table. | Met la table en valeur sans thème. | P |
| D-46 | Regarder autour : **clic droit maintenu** ; le curseur reste libre pour cliquer les cartes. | Pas de conflit entre regard et sélection. | P |
| D-47 | Hot-seat : bascule **automatique** vers le joueur actif après chaque action (UX §9). | Le plus rapide pour tester les règles en 3D. | P |
| D-48 | **Claude et Codex voient les mêmes skills**, pour Dubito (tout dans le dépôt) et pour tout le PC (dossier commun `~/.agents/skills` + jonctions), vérifié par `tools/sync-skills.ps1`. | Les deux agents travaillent sur le même sujet. | P |
| D-49 | Skills tiers importés après audit (liste : `.agents/skills/THIRD_PARTY.md`) : Blender EL4CTEO 1.3.3, image→3D, game-feel, audio, assets 2D, Steam, débogage, game-designer ; documentation Scenario Labs sans ses scripts. | Choix du propriétaire ; scripts Scenario refusés à l'audit (pont permissif, incompatible Windows). | P |
| D-50 | Valeurs des cartes en **Liberation Sans Bold** (OFL 1.1, version 2.1.5 du dépôt officiel), versionnée avec sa licence dans `tools/cards/fonts/`. | Même police libre que l'interface (ASSETS §1). | P |
| D-51 | Dos des cartes **bordeaux** avec liseré crème, motif géométrique original symétrique. | Contraste fort avec le tapis vert. | P |
| D-52 | Calcul lourd sur le **GPU** (RTX 3070) : Blender en EEVEE ou Cycles GPU OptiX, jamais Cycles CPU (AGENTS règle 9). | Bien plus rapide que le processeur. | P |
| D-53 | Flux validé pour les modèles 3D : Claude écrit le prompt d'image (Nano Banana Pro), le propriétaire génère les images de référence, Codex modélise dans Blender. Style visuel définitif à choisir plus tard (Meshy AI à évaluer alors, coûts compris). | Test de la chaise réussi (étape 2.0). | P |
| D-54 | Scène plus claire : lumière d'ambiance relevée + lumière d'appoint douce ; la table reste l'endroit le plus éclairé. | « Trop sombre » à la revue de l'étape 2.0. | P |
| D-55 | À la table, le **joueur suivant est à droite** (sens inverse des aiguilles d'une montre vu du dessus). | Habitude des jeux de cartes français (belote, tarot). | P |
| D-56 | Debug en Play : touches **2 à 6** reconstruisent la table avec ce nombre de sièges. | Tester vite 2, 4 et 6 joueurs. | P |
| D-57 | Sélection des cartes **à la souris seulement** (clic gauche) ; pas de raccourci 1–9/0. Les touches 2 à 6 restent le debug « nouvelle partie » (D-56). | Choix du propriétaire ; évite le conflit de touches. | P |
| D-58 | Debug phase 2 : option **chrono court** (10 s par tour) pour tester sans attendre. | Tant que le hot-seat (2.4) n'existe pas, les autres sièges laissent filer leur chrono. | P |
| D-59 | En hot-seat, l'interface **nomme le joueur** (3e personne) au lieu de « tu », et annonce « Au tour de Joueur N » à chaque changement de siège. En réseau, « tu » reste. | Une seule personne joue tous les sièges : « tu » ne disait pas de qui il s'agissait. | P |
| D-60 | **Mirror 96.11.4** (MIT), copié dans `unity/Assets/Mirror` sans les exemples ni le module d'hébergement Edgegap. | Version rodée de la dernière branche ; on n'embarque que l'utile. | P |
| D-61 | Tests à plusieurs instances avec **Multiplayer Play Mode** 2.0.2 (paquet Unity) plutôt qu'éditeur + builds. | Un clic sur Play lance hôte + clients, sans compiler. | P |
| D-62 | Le protocole sérialise ses messages lui-même (C# pur, `Dubito.Protocol`) ; Mirror ne transporte qu'une enveloppe d'octets (ARCHITECTURE §3). | Confidentialité testable sur les vrais octets hors Unity ; Protocol reste sans dépendance à Mirror. | C |
| D-63 | Salon : l'hôte n'a pas de bouton **Prêt** (cliquer sur Lancer vaut prêt) ; il peut lancer dès 2 joueurs quand tous les invités sont prêts. **Revanche** ramène tout le monde au même salon, prêts remis à zéro. | Lecture de UX §6.2 et §5 pour l'étape 3.4. | C |
| D-64 | **Steamworks.NET 2025.165.0** et **FizzySteamworks 6.0.1** (MIT), installés depuis leurs releases officielles. | Transport Steam pour Mirror (ARCHITECTURE §1). | P |
| D-65 | Test entre deux PC (étape 4.3) avec **un ami** et son compte Steam : on lui envoie une build. | Choix du propriétaire. | P |
