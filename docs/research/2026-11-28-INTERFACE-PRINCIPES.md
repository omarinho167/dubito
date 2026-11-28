# Interface de Dubito : principes de design (proposition DA.6 d)

Proposition du 28 novembre 2026, à valider par le propriétaire avant toute maquette. Une fois validée, elle sera reportée dans `docs/UX.md` (seule source de la présentation) ; ce fichier reste la trace du raisonnement.

Sources : `UX §1–5` (ce que le joueur doit savoir et faire), `RULES §4, §9` (actions et informations visibles), étude `2026-11-28-ETUDE-LIARS-BAR.md` (interface très légère, infos posées dans la scène), charte `ASSETS §6`.

## 1. Ce que le joueur doit savoir, et quand

| Information ou action | Quand | Importance |
|---|---|---|
| À qui c'est le tour, et combien de temps il reste | Toujours | 1 |
| La dernière annonce (« Léa : 3 × Dame ») | Après chaque pose | 1 |
| Ce que je peux faire maintenant (Jouer, Douter, Défausser) | Pendant mon tour | 1 |
| Le verdict d'un doute (mensonge ou vérité, qui ramasse) | Pendant la révélation | 1 |
| La valeur de la manche (ou « ouverture libre ») | Toujours | 2 |
| Ma sélection (nombre de cartes, valeur annoncée) | Pendant mon tour | 2 |
| Nombre de cartes de chaque joueur et du tas | Toujours, discret | 3 |
| Pourquoi une action est impossible | Au survol | 3 |

## 2. Règles de design

1. **La scène d'abord.** Aucun grand panneau permanent : au repos, l'écran montre la table, les juges et ta main, avec seulement de petites étiquettes. L'interface apparaît quand elle sert, puis s'efface.
2. **L'information est là où ça se passe.**
   - Le nom et le nombre de cartes flottent au-dessus de chaque juge.
   - Le chrono est un anneau autour de l'étiquette du joueur actif.
   - L'annonce apparaît en bulle au-dessus de celui qui joue, puis reste posée près du tas.
   - La valeur de la manche est une plaque en laiton posée sur la table, près du tas.
3. **Les actions sont près de tes mains.** Les 3 boutons forment une petite barre juste au-dessus de ta main, au centre de l'écran, pas dans un coin. Douter est le plus visible (rouge, icône de marteau, à maintenir).
4. **Un seul message important à la fois**, en haut au centre : « À toi de jouer », « MENSONGE ! ». Il reste court (1 à 2 s), puis disparaît.
5. **Trois tailles, pas plus** : grande pour le moment présent (niveau 1), moyenne pour la manche et les choix (niveau 2), petite pour les compteurs (niveau 3).
6. **Thème par petites touches.**
   - Matières de la charte (parchemin, laiton, cire rouge) sur de petits éléments : étiquettes, plaques, sceaux.
   - Une police à empattements lisible pour les titres et une police simple pour les chiffres.
   - Jamais de cadres décoratifs autour de tout.
7. **Lisible et juste.** Textes d'au moins 24 px en 1080p. Le résultat n'est jamais indiqué par la couleur seule (`UX §8`). Une même information garde toujours la même place.
8. **Chaque animation confirme une action.** Une carte jouée glisse vers le tas, le marteau frappe au doute et le tas glisse vers le perdant. Le joueur comprend par le mouvement, pas par un texte.

## 3. Zones de l'écran

Plan : `art/style/ui/U-zones-v1.png`.

- **Haut centre** : message du moment (temporaire).
- **Au-dessus des juges** : étiquettes nom et cartes ; anneau de chrono sur le joueur actif ; bulle d'annonce.
- **Centre de la table** : tas, plaque de la valeur de manche, nombre de cartes du tas, marteau.
- **Bas centre** : ta main (effet loupe) et, au-dessus pendant ton tour, la barre d'actions avec l'annonce (« 2 × Dame »).
- **Bas droite, discret** : rappel des touches (Entrée, D, Tab).
- **Haut gauche** : rien (le menu de pause s'ouvre avec Échap).

## 4. Déroulé par moment

1. **Tour d'un autre** : étiquettes, anneau de chrono sur lui. Quand il pose, une bulle « 3 × Dame » apparaît au-dessus de lui et ses cartes glissent vers le tas.
2. **Mon tour** : message bref « À toi de jouer », anneau de chrono sur moi, barre d'actions au-dessus de ma main. À l'ouverture, la valeur se choisit dans la barre.
3. **Douter** : je maintiens Douter, la jauge se remplit, le marteau frappe.
4. **Révélation** (le moment fort, `UX §5`) : la caméra se rapproche du tas, les cartes se retournent, « MENSONGE » ou « VÉRITÉ » s'affiche en grand au centre, puis le tas glisse vers le perdant.
5. **Victoire en attente** : bannière en haut au centre (« Léa n'a plus de cartes ! Max peut douter… »), et Douter pulse chez celui qui peut douter.
