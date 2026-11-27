# Générer des images Nano Banana Pro depuis Claude et Codex avec l'abonnement Google Flow — 27 novembre 2026

Audit de Claude (code de GabrielGargiuloDev/google-flow-mcp, essais réels dans Flow) et recherche de Codex (projets concurrents, voies officielles). Résultat retenu : D-68.

## Ce que permet l'abonnement

- **Aucune API Flow officielle** ne débite les quotas de l'abonnement grand public. L'API Gemini (AI Studio, Vertex) est facturée à part : environ 0,134 $ par image Nano Banana Pro 1K/2K ([tarifs](https://ai.google.dev/gemini-api/docs/pricing#gemini-3-pro-image)).
- Gemini CLI avec connexion Google : arrêt pour les comptes grand public le 18 juin 2026, remplacé par Antigravity CLI ([annonce](https://developers.googleblog.com/an-important-update-transitioning-gemini-cli-to-antigravity-cli/)), dont le générateur d'images est Nano Banana 2, non configurable ([modèles](https://antigravity.google/docs/models)). Whisk a fermé le 30 avril 2026.
- **Limites de Flow** ([aide Flow](https://support.google.com/flow/answer/16729550?hl=en)) : Nano Banana Pro jusqu'à **100 images par jour en Pro** (500 en Ultra), agrandissement 2K jusqu'à 200 par jour, **4K réservé à Ultra**. Zéro crédit ne veut pas dire illimité ; Google peut ajouter des limites aux usages très rapides.

## Projets existants (relevé de Codex)

Aucun ne respecte tel quel nos contraintes (pas de drapeau anti-détection, aucune dépense vidéo sans accord humain, références locales, choix 1K/2K).

| Projet | Pourquoi écarté |
|---|---|
| GabrielGargiuloDev/google-flow-mcp (demandé) | journal sur stdout (casse le protocole MCP), `--disable-blink-features=AutomationControlled`, modèle et format jamais choisis dans l'interface, références ignorées, clic automatique sur « Approuver » (crédits vidéo), prompt enveloppé en italien, ancienne adresse labs.google |
| TMSSS05/google-flow-browser-mcp (ancêtre) | mêmes défauts de lancement et de journal |
| Dilantha-Wijesinghe/google-flow-mcp | le plus propre, mais pas d'import de références et coût illisible accepté |
| felipedamacenoteodoro/mcp-google-flow | agrandissements non vérifiés, acceptation automatique d'un dialogue |
| Rocco3D/FlowPilot, swissmarley/gflow-cli, ParkSangGwon, BRPLia, ffroliva/gflow-cli | drapeau anti-détection ou navigateur furtif |
| GTAI-1/flow-mcp | pas de licence, faille signalée (issue 28), vidéo autorisée sous plafond |
| miyakejima/google-flow-mcp | approbation automatique, « ne plus demander » |

## Solution retenue

Réécriture minimale du MCP demandé, appuyée sur l'interface réelle de `flow.google.com` (relevée le 27 novembre 2026, compte Pro, interface en français), installée au niveau du PC : `~\.agents\mcp\google-flow` (mode d'emploi dans son README). Vérifié de bout en bout par le protocole MCP : choix de Nano Banana Pro, formats 16:9 et 3:4, 1 et 2 variantes, une et deux références locales, téléchargement 1K automatique (896×1200 en 3:4), agrandissement 2K (1792×2400), refus du 4K en Pro, compteur quotidien, verrou entre Claude et Codex.

Fragilité connue : les sélecteurs suivent les libellés de l'interface ; si Google la change, `flow_inspect` donne les nouveaux noms à reporter dans `src/labels.js`.
