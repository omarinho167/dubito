# Dubito : expérience joueur (V1)

> Ce que le joueur voit, peut faire et ressent. Les règles elles-mêmes sont dans [RULES.md](RULES.md).
> La direction artistique n'est **pas** fixée : V1 est en 3D basique (formes simples), voir [VISION.md](VISION.md).

## 1. Principes

1. **Le jeu marche avant d'être beau.** Formes simples, couleurs unies, textes lisibles.
2. **L'écran répond toujours à 4 questions** : c'est à qui ? quelle est l'annonce à juger ? qu'est-ce que je peux faire maintenant ? combien de cartes a chacun ?
3. **L'affichage suit l'hôte, il ne décide jamais.** Une animation montre un résultat déjà validé. Si un état plus récent arrive, l'animation se termine vite ou saute.
4. **Rien n'est cliquable sans raison.** Une action impossible est grisée et explique pourquoi au survol.

## 2. La scène

- **Caméra** : à la 1re personne, assis à sa place. On tourne la tête en **maintenant le clic droit** et en bougeant la souris, dans une limite (environ ±60° à gauche/droite, un peu en haut/bas). Sans clic droit, le curseur est libre pour cliquer. Pas de déplacement.
- **Table** : ronde, de 2 à 6 places réparties régulièrement. On est toujours placé face au centre. Le joueur suivant est assis à sa droite (D-55).
- **Adversaires** : personnages stylisés simples assis (en phase 2 : mannequins neutres, `ASSETS.md`), avec **nom Steam** et **couleur** de siège.
- **Main des adversaires** : leurs cartes sont visibles de dos, en éventail. On peut donc compter leurs cartes (`RULES §9`). Un petit chiffre au-dessus confirme le nombre.
- **Sa propre main** : en éventail devant soi, cartes face visible, triées par valeur.
- **Tas** : au centre, cartes de dos. Son nombre de cartes est affiché, ainsi que la **valeur de manche** en grand (ou « Ouverture libre » si le tas est vide).
- **Dernière annonce** : affichée près du tas : « Léa : 3 × Dame ».
- **Joueur actif** : son siège est mis en évidence (lumière, contour) et son chrono est visible au-dessus de lui.

## 3. Actions possibles

Quelles actions sont possibles : voir la matrice de `RULES §4`. Côté affichage :

- les trois boutons sont toujours visibles pendant son tour, grisés quand l'action est impossible ;
- **Douter est mis en avant** (couleur, pulsation légère) quand on peut douter, surtout face à une victoire en attente ;
- hors de son tour, le panneau d'action est masqué.

Message au survol d'une action grisée, exemples : « Ce n'est pas ton tour », « Rien à douter : le tas est vide », « Interdit : Léa attend sa victoire ».

## 4. Interactions (souris + clavier)

### Jouer

1. **Clic sur une carte** de sa main : elle se soulève (sélectionnée). Nouveau clic : elle redescend. Maximum 4 cartes ; la 5e est refusée avec un message.
2. Le **panneau d'action** (petit, en bas de l'écran, seulement pendant son tour) affiche l'annonce :
   - **Ouverture** : choix de la valeur (A à R), le nombre suit la sélection : « 2 × [ Dame ▾ ] ».
   - **Manche en cours** : valeur fixe, le nombre suit la sélection : « 2 × Dame ».
3. **Confirmer** : les cartes partent vers le tas. Les boutons sont bloqués en attendant la réponse de l'hôte.

### Douter

- Bouton **Douter** : **maintenir** environ 0,6 s pour valider (pas de clic accidentel). Une jauge se remplit.

### Défausser

- Bouton **Défausser** : un clic, puis une confirmation légère (« Retirer les 7 cartes du tas et passer ton tour ? »).

### Raccourcis clavier

| Touche | Action |
|---|---|
| 1 à 9, 0 | Sélectionner une carte de sa main par position |
| Entrée | Confirmer Jouer |
| D (maintenir) | Douter |
| Échap | Annuler la sélection, ou fermer un panneau |
| Tab (maintenir) | Récapitulatif de la table : joueurs, cartes, ordre |

## 5. Moments clés

- **Pose d'un adversaire** : son paquet de cartes glisse vers le tas, l'annonce apparaît, un son discret.
- **Doute (le seul moment cinématique)** : léger zoom sur le tas, les cartes du dernier coup se retournent une par une, verdict en grand (« MENSONGE » / « VÉRITÉ »), puis le tas glisse vers le perdant. Environ 3 s, toute la table regarde.
- **Défausse** : le tas part sur le côté, disparaît.
- **Victoire en attente** : bannière « Léa n'a plus de cartes ! [Suivant] peut douter… ».
- **Fin de partie** : écran de résultat (gagnant), puis **Revanche** (même salon) ou **Quitter**.
- **Tour sauté** : message « Temps écoulé : tour de Léa sauté ». Au 1er tour sauté, un avertissement pour le joueur concerné : « Encore un tour sauté et tu seras exclu ».

## 6. Écrans hors partie

1. **Menu principal** : Créer une partie, Rejoindre (invitation Steam), **Règles**, Options, Quitter.
2. **Salon** : liste des joueurs, bouton **Prêt** pour chacun, l'hôte lance quand tout le monde est prêt et qu'il y a au moins 2 joueurs. Invitation via l'overlay Steam. Le bouton **Règles** est aussi accessible ici.
   - **Réglages de l'hôte** (modifiables tant que la partie n'est pas lancée) :
     - **Durée du chrono** : 30 / 45 / 60 / 90 s, 45 s par défaut (`RULES §5`) ;
     - **Places max** : de 2 à 6, 6 par défaut. On ne peut pas descendre sous le nombre de joueurs déjà présents.
   - Les places autour de la table sont tirées au hasard au lancement (`RULES §3`).
3. **Écran Règles** : une page illustrée qui résume `RULES §1, §4 et §6` en langage simple (schémas : poser, douter, défausser, dernier doute). C'est **la seule aide au jeu en V1** : pas de tutoriel, pas de bulles d'aide, pas d'historique des coups.
4. **Options** : volume, sensibilité de la souris, réduction des animations.
5. **Messages d'erreur** clairs : salon plein, partie déjà commencée, hôte parti, Steam non lancé.

## 7. Son (V1 minimal)

Sons simples : poser, retourner, verdict du doute, ramasser, défausser, victoire, alerte des 10 dernières secondes. Pas de musique obligatoire en V1.

## 8. Accessibilité (base V1)

- Option **Réduction des animations** : pas de zoom caméra, révélation instantanée mais lisible.
- Le résultat ne repose jamais sur la couleur seule (texte + icône).
- Textes lisibles en 1920×1080 et en 1280×720.

## 9. Mode debug hot-seat (phase 2)

Une seule instance, sans réseau ; une personne joue tous les sièges (D-47).

- Après chaque action validée, la caméra et la main affichée passent **automatiquement** au siège du joueur actif ; la sélection de cartes est remise à zéro.
- Pendant une révélation, on reste au siège du douteur (ou du répondant pour une révélation automatique) ; on bascule à sa résolution.
- Après un tour sauté ou une exclusion, on bascule au nouveau joueur actif.
- Noms locaux « Joueur 1 » à « Joueur 6 » (les noms Steam arrivent en phase 4).
- Une touche de debug permet de simuler un départ (déconnexion) du siège affiché, pour tester `RULES §7`.
