# Dubito : architecture technique

> Décisions techniques pour Unity. Le *pourquoi* de chaque choix est dans [DECISIONS.md](DECISIONS.md),
> les comparaisons détaillées dans [research/STACK-UNITY.md](research/STACK-UNITY.md).

## 1. Choix de base

| Sujet | Choix |
|---|---|
| Moteur | **Unity 6.3 LTS** (dernier correctif dans Unity Hub), modèle **Universal 3D (URP)** |
| Réseau | **Mirror** (hôte = un joueur) |
| Transport en ligne | **FizzySteamworks** (réseau Steam P2P) |
| API Steam | **Steamworks.NET** (lobbies, invitations, noms) |
| Transport local (tests) | Transport Mirror par défaut (KCP) sur `localhost`, **sans Steam** |
| Tests | NUnit : Unity Test Framework (EditMode) + projet `dotnet test` hors Unity pour le moteur de règles |
| Langage | C# compatible **.NET Standard 2.1 / C# 9** (limite d'Unity 6.3) |

Les versions exactes de Mirror, FizzySteamworks et Steamworks.NET sont **figées** dans le dépôt dès l'installation et notées ici.

## 2. Découpage du code

Le projet Unity vit dans `unity/`. Le code est découpé en assemblages (asmdef) qui ne peuvent dépendre que vers le bas :

```text
Presentation  ──►  Networking  ──►  Protocol  ──►  Core
     │                                            ▲
     └────────────────────────────────────────────┘ (lecture des types)
```

| Assemblage | Rôle | Interdit |
|---|---|---|
| `Dubito.Core` | **Tout le moteur de règles** : état de partie, commandes, validation, résolution, chrono (en temps simulé), vues filtrées par joueur. | Toute référence à Unity (`No Engine References` activé), Mirror ou Steam. |
| `Dubito.Protocol` | Messages réseau : commandes du client, vue publique, vue privée, erreurs, version du protocole. | Logique de jeu. |
| `Dubito.Networking` | Connexions Mirror, lobby Steam, association connexion → siège, transmission des commandes au moteur, envoi des vues. | Règles du jeu (toujours déléguées à Core). |
| `Dubito.Presentation` | Scène 3D, caméra, cartes, animations, interface, sons, entrées. | Décider d'une règle, lire l'état d'un adversaire. |
| `Dubito.Tests` | Tests EditMode de Core et Protocol. | Embarqué dans le jeu. |

## 3. Le flux d'une action

```text
Client : clic « Jouer »  →  Commande { id, révision attendue, cartes, annonce }
Hôte   : connexion → siège (jamais l'identifiant envoyé par le client)
         → Core valide et applique  →  nouvel état
         → pour chaque joueur : vue filtrée (publique + sa main)
Clients : reçoivent leur vue  →  la présentation anime vers ce nouvel état
```

Règles qui en découlent :

- **L'hôte est l'unique arbitre** (`RULES §8`). Le client n'applique jamais une action lui-même, il attend la vue.
- Chaque commande a un **identifiant** (doublons ignorés) et la **révision** d'état qu'elle vise (actions périmées refusées).
- Après chaque changement, on envoie un **petit instantané filtré** à chaque joueur. Pas d'optimisation par différences en V1.
- **Pas un objet réseau par carte.** Les cartes 3D sont créées localement à partir de la vue reçue.
- **Confidentialité** (`RULES §9`) : une vue est construite par joueur ; les cartes adverses n'y figurent jamais. On ne « cache pas les meshes », on n'envoie pas les données.
- L'aléatoire de Core vient d'une **source injectable** : graine fixe dans les tests, aléatoire côté hôte en partie.
- Le chrono est géré par l'hôte. Core reçoit le temps écoulé, il ne lit pas l'horloge lui-même (testable).

Limite connue : l'hôte a toutes les mains en mémoire. Un hôte tricheur peut lire le jeu. Accepté en V1 (parties entre amis).

## 4. Tester vite

1. **Moteur de règles** : chaque cas de [CASES.md](CASES.md) devient un test. Le même code Core est compilé dans un projet `.NET` (`tests/Dubito.Core.Tests`) pour lancer `dotnet test` en quelques secondes, sans ouvrir Unity.
2. **Dans Unity** : tests EditMode en ligne de commande (`Unity.exe -batchmode -runTests -testPlatform EditMode …`), sur une copie si l'éditeur est ouvert.
3. **Réseau local** : hôte + client(s) sur la même machine (éditeur + build, ou plusieurs builds), transport local sans Steam.
4. **Steam** : deux PC avec deux comptes Steam différents. App ID de test **480 (Spacewar)** au début, avec un filtre de lobby propre à Dubito ; vrai App ID dès qu'il existe.
5. **Unity MCP** : sert à créer et inspecter des scènes et prefabs et à lancer des tests. Il ne remplace ni les tests automatiques ni la vérification visuelle par le propriétaire.

## 5. Pièges Steam connus

- Le client Steam doit tourner. Initialiser Steam une seule fois et exécuter les callbacks à chaque frame.
- `steam_appid.txt` sert seulement en développement ; il est ignoré par git et jamais livré.
- Un lobby Steam ne connecte pas la partie : après l'invitation, il faut récupérer l'hôte et démarrer le client Mirror.
- Lobbies partagés sur l'App ID 480 : filtrer par une clé Dubito et la version du protocole.

## 6. Arborescence prévue

```text
/docs                 conception (ce dossier)
/unity                projet Unity
  /Assets/Dubito/Core           (asmdef Dubito.Core)
  /Assets/Dubito/Protocol
  /Assets/Dubito/Networking
  /Assets/Dubito/Presentation
  /Assets/Dubito/Tests
  /Assets/Dubito/Scenes         Menu, Table
/tests/Dubito.Core.Tests  projet dotnet qui compile les sources de Core
```
