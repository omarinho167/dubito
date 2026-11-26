# Recherche : faire la 3D avec l'IA

> Recherche Codex du **2026-11-26**. Prix en dollars US, hors taxes. Les outils évoluent très vite : revérifier avant d'acheter.
> Aucune décision n'est prise : à trancher avant la phase 5 (personnages).

## En bref

- **Ni Codex (GPT-6.2) ni Claude ne génèrent de modèle 3D eux-mêmes.** Ils écrivent du code qui pilote Blender ou Unity (via MCP).
- **Décor simple** (pièce, table, chaises, cartes) : les agents le font très bien par script.
- **Personnages** : partir d'un pack déjà riggé et cohérent, puis le décliner. La génération IA de personnages animés reste fragile (mains, déformations).
- **Générateurs IA** (Tripo, Meshy…) : utiles pour quelques accessoires particuliers, environ 20 $/mois quand on en a besoin.
- **« Une pièce refaite depuis des photos »** : c'est de la photogrammétrie ou du *Gaussian splatting*. Très joli, mais lourd à nettoyer et à éclairer, peu adapté à une pièce stylisée interactive.

## Générateurs de modèles

| Outil | Intérêt | Prix | Attention |
|---|---|---|---|
| [Tripo](https://www.tripo3d.ai/product-doc/ai-3d-models/p-series-smart-mesh) (Smart Mesh P2.0) | Maillage propre en quadrilatères, bon pour l'animation | 5–20 $/mois | Droits commerciaux : offre payante |
| [Meshy](https://www.meshy.ai/pricing) 7.1 | Textures, rigging, plugins Unity/Blender | 20 $/mois | Gratuit = CC BY, téléchargements récents payants |
| [Hyper3D Rodin](https://hyper3d.ai/pricing) | Objets décoratifs détaillés | 30–120 $/mois | — |
| [Sloyd](https://www.sloyd.ai/pricing) | Mobilier stylisé paramétrable | 15 $/mois | Crédits pour certaines opérations |
| Hunyuan3D 2.1 | Poids téléchargeables | — | **Licence qui exclut l'UE : à éviter** |
| TRELLIS.2 | Recherche, local | — | Linux + GPU 24 Go |
| Luma Genie | — | — | **Arrêté le 1/1/2026** |
| CSM / Cube | — | — | Racheté par Google, service incertain |

## Personnages

| Option | Recommandation |
|---|---|
| [Quaternius](https://quaternius.com/packs/universalbasecharacters) | Gratuit, squelette humanoïde. Licence QAL (2026) : jeu commercial OK, pas de revente des assets. Garder la licence du téléchargement. |
| [Kenney](https://kenney.nl/support) | CC0, idéal pour les prototypes |
| [Synty](https://syntystore.com/pages/licences-overview) | Payant (30–100 $), style homogène |
| IA + auto-rig ([Mixamo](https://helpx.adobe.com/creative-cloud/faq/mixamo-faq.html), [AccuRIG](https://www.reallusion.com/auto-rig/accurig/)) | Seulement pour un style original voulu ; tester un seul personnage d'abord |

Animations utiles : assis en permanence, respiration, regard vers un joueur, hochement de tête, petit geste du bras.

## Connecteurs MCP

| Outil | Choix |
|---|---|
| Blender | **Installé (2026-11-26)** : [serveur MCP officiel de Blender Lab](https://projects.blender.org/lab/blender_mcp) (add-on « MCP » dans Blender 5.3 + programme `blender-mcp`, port local 9876), branché pour Claude Code et Codex. Alternative communautaire : [ahujasid/mcp-for-blender](https://github.com/ahujasid/mcp-for-blender). |
| Unity | **Installé (2026-11-26)** : MCP officiel d'Unity, fourni par le paquet `com.unity.ai.assistant` du projet et le relais `~/.unity/relay`, branché pour Claude Code et Codex. Ne marche que si le projet est ouvert dans l'éditeur. Alternative communautaire : [CoplayDev/unity-mcp](https://github.com/CoplayDev/unity-mcp). |

Ces connecteurs exécutent du code dans l'application : à utiliser sur un projet sauvegardé (git).

## Pipeline conseillé

1. Phases 1–4 : formes simples dans Unity, aucun achat.
2. Décor (pièce, table, chaises, cartes) créé par script Blender/Unity ; faces des cartes dessinées par nous pour une lisibilité parfaite.
3. Une fois le style choisi : un pack de personnages, valider **un** personnage assis avec deux gestes, puis décliner.
4. Accessoires particuliers seulement : générateur IA, nettoyage dans Blender, import FBX dans Unity.
5. Validation par le propriétaire, en mouvement, depuis le siège du joueur.

Budget : 0 $ pour commencer, puis environ 20 $ (un mois de Tripo ou Meshy) ou un pack acheté une fois.
