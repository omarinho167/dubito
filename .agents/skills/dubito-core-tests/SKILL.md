---
name: dubito-core-tests
description: Transforme les cas de docs/CASES.md en tests NUnit et lance les tests de Dubito (dotnet test hors Unity, puis Unity Test Framework en batchmode) avec un verdict fiable. À utiliser pour écrire, corriger ou exécuter un test, ou avant de déclarer une étape de code terminée.
---

# Tests de Dubito

Règle du dépôt : tests d'abord pour les règles (`AGENTS.md` règle 4). `docs/RULES.md` a raison sur un test.

## Écrire un test depuis CASES

- Un cas `C-XXX-NN` = un test **nommé** par son identifiant. Un nom de méthode C# ne peut pas contenir de tiret : utiliser `[TestCase(TestName = "C-DOU-03")]` (ou `TestCaseData(...).SetName("C-DOU-03")`), pas `Description`, qui ne change pas le nom du test.
- Construire l'état de départ du cas, appliquer l'action, vérifier **tout** le résultat attendu (main, tas, joueur actif, chrono, coup doutable).
- Hasard : source injectable avec graine ou suite imposée (ARCHITECTURE §3). Temps : passé en paramètre, jamais l'horloge.
- Si le cas semble faux par rapport à RULES : s'arrêter, `dubito-ask-owner`. Ne pas adapter le test au code.

## Partage des sources

- Sources de Core uniquement dans `unity/Assets/Dubito/Core/`.
- Deux projets hors Unity :
  - une **bibliothèque** (`netstandard2.1`, `LangVersion 9.0`, `EnableDefaultCompileItems` à `false`) qui compile seulement `<Compile Include="../../unity/Assets/Dubito/Core/**/*.cs" />` ;
  - le **projet de tests** `tests/Dubito.Core.Tests` (runtime .NET installé, NUnit + adaptateur + `Microsoft.NET.Test.Sdk` à versions figées), qui garde l'inclusion automatique de ses propres fichiers et référence la bibliothèque.
- Ne jamais copier les sources ni compiler contre les `.csproj` générés par Unity.

## Lancer : d'abord dotnet (secondes)

```powershell
dotnet test .\tests\Dubito.Core.Tests --logger "trx;LogFileName=core.trx"
```

## Puis Unity (EditMode)

- L'éditeur ouvert **bloque** un second Unity sur le même projet. Soit lancer les tests dans l'éditeur ouvert (`dubito-unity-editor`), soit fermer l'éditeur (avec l'accord du propriétaire), soit travailler sur une copie.
- Chemin de l'exécutable depuis `ProjectVersion.txt` (`C:\Program Files\Unity\Hub\Editor\<version>\Editor\Unity.exe`), jamais une version recopiée.
- Arguments : `-batchmode -nographics -projectPath <unity> -runTests -testPlatform EditMode -testResults <xml unique> -logFile <log>`. **Pas de `-quit`** avec `-runTests`. Lancer avec `Start-Process -Wait -PassThru` pour attendre la fin.

## Verdict

« Ça passe » seulement si **tout** est vrai :

1. code de sortie 0 ;
2. fichier XML **créé par ce lancement** (chemin unique par exécution) ;
3. nombre de tests exécutés = nombre attendu (pas 0) et aucun échec dans le XML ;
4. pas d'erreur de compilation dans le log : après une compilation ratée, Unity peut lancer les **anciens** assemblages.

Rapporter les commandes lancées, le nombre de tests et les échecs, sans arrondir.
