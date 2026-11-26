# Source de `systematic-debugging`

- Dépôt : https://github.com/obra/superpowers
- Version : 22f0b463e7a5e579904dd6211a520321035526f7
- Chemin d'origine : `skills/systematic-debugging`
- Licence : MIT (fichier joint)
- Importé le : 2026-11-26, relu par Claude et audité par Codex (aucun script exécuté).

## Modifications locales

- Ajout de la section « Dans Dubito » en tête du SKILL.md : Dans Dubito : les renvois aux autres skills Superpowers sont remplacés par `dubito-core-tests` (tests qui reproduisent le bug) et les contrôles locaux ; ne jamais journaliser de secrets ni les mains privées des joueurs ; toute décision de règle passe par `dubito-ask-owner` (choix multiples). Le script `find-polluter.sh` (npm) n'est pas importé.

Pour mettre à jour : comparer avec la nouvelle version amont (diff), relire, puis remplacer en gardant ces modifications.
