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
| Luma Genie | — | — | **Arrêté le 1/1/2026** |
| CSM / Cube | — | — | Racheté par Google, service incertain |

## Générateurs locaux (PC du propriétaire : RTX 3070 8 Go, 32 Go RAM)

> Ajout Codex du 2026-11-26. Mémoire annoncée par les auteurs, **rien n'a été mesuré sur ce PC**. Le propriétaire accepte d'assumer les licences ; on les note quand même.

| Outil | VRAM | Sortie | Installation | Licence |
|---|---|---|---|---|
| [Stable Fast 3D](https://github.com/Stability-AI/stable-fast-3d) | ≈ 6 Go | GLB avec UV et texture, remesh triangle/quad | Nœuds ComfyUI dans le dépôt | Stability Community (licence entreprise au-delà de 1 M$ de revenus) |
| [SPAR3D](https://github.com/Stability-AI/stable-point-aware-3d) | ≈ 7 Go en mode basse VRAM | GLB texturé, faces cachées mieux contrôlées | Script ou ComfyUI ; Windows expérimental | Stability Community |
| [Hunyuan3D 2mini](https://github.com/Tencent-Hunyuan/Hunyuan3D-2) | forme ≈ 6 Go ; textures plus lourdes | GLB/OBJ | [Pack Windows portable](https://github.com/YanWenKun/Hunyuan3D-2-WinPortable) ou [Pinokio basse VRAM](https://github.com/pinokiofactory/Hunyuan3d-2-lowvram) | Tencent : **exclut l'UE, le Royaume-Uni et la Corée du Sud** |
| [Hunyuan3D 2.1](https://github.com/Tencent-Hunyuan/Hunyuan3D-2.1) | 10–29 Go officiels ; ≈ 3–6 Go avec le pack portable optimisé | GLB, matériaux PBR | Pack portable | Tencent (mêmes exclusions) |
| [TRELLIS.2](https://github.com/microsoft/TRELLIS.2) | 24 Go officiels ; [fork Windows 8 Go](https://github.com/IgorAherne/TRELLIS.2-stableprojectorz) ; ComfyUI natif en INT8 | GLB, PBR, détails fins, maillage dense | [ComfyUI natif](https://docs.comfy.org/tutorials/3d/trellis2) (août 2026) | MIT (cœur) |
| [Pixal3D](https://github.com/TencentARC/Pixal3D) | mode basse VRAM, 8 Go non garanti | GLB, PBR, fidèle à la vue source | [ComfyUI natif](https://docs.comfy.org/tutorials/3d/pixal3d) | MIT (à vérifier par checkpoint) |
| [TripoSR](https://github.com/VAST-AI-Research/TripoSR) / [TripoSG](https://github.com/VAST-AI-Research/TripoSG) | ≈ 6 / 8 Go | ébauche rapide / forme plus détaillée | Scripts Python | MIT |

Prix API (2026-11) : Tripo ≈ 0,20–0,30 $ par image → 3D (+ options quad/low-poly) ; Meshy 20–35 crédits par génération, MCP officiel ; Rodin à partir de 96–120 $/mois. [Tripo](https://developers.tripo3d.ai/en/pricing), [Meshy](https://docs.meshy.ai/en/api/pricing), [Rodin](https://hyper3d.ai/pricing).

Dans tous les cas : maillage à nettoyer, retopologiser et rigger dans Blender. Méthode de reprise : skill `blender-to-unity`. Pas de générateur IA pour l'instant (DECISIONS D-38).

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
| Unity | **Installé (2026-11-26)** : MCP officiel d'Unity, fourni par le paquet `com.unity.ai.assistant` du projet et le relais `~/.unity/relay`, branché pour Claude Code et Codex. Ne marche que si le projet est ouvert dans l'éditeur. **Déprécié par Unity, débranché le 2026-11-26** : remplacé par Unity CLI (voir [SKILLS-ET-OUTILS.md](SKILLS-ET-OUTILS.md)). Alternative communautaire : [CoplayDev/unity-mcp](https://github.com/CoplayDev/unity-mcp). |

Ces connecteurs exécutent du code dans l'application : à utiliser sur un projet sauvegardé (git).

## Pipeline conseillé

1. Phases 1–4 : formes simples dans Unity, aucun achat.
2. Décor (pièce, table, chaises, cartes) créé par script Blender/Unity ; faces des cartes dessinées par nous pour une lisibilité parfaite.
3. Une fois le style choisi : un pack de personnages, valider **un** personnage assis avec deux gestes, puis décliner.
4. Accessoires particuliers seulement : générateur IA, nettoyage dans Blender, import FBX dans Unity.
5. Validation par le propriétaire, en mouvement, depuis le siège du joueur.

Budget : 0 $ pour commencer, puis environ 20 $ (un mois de Tripo ou Meshy) ou un pack acheté une fois.
