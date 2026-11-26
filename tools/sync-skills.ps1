# Vérifie (et avec -Apply, rétablit) que Claude Code et Codex voient exactement les mêmes skills.
#
#   powershell -ExecutionPolicy Bypass -File tools\sync-skills.ps1          # rapport seulement, n'écrit rien
#   powershell -ExecutionPolicy Bypass -File tools\sync-skills.ps1 -Apply   # corrige (sauvegarde vérifiée d'abord)
#
# Organisation (AGENTS.md, section « Skills partagés ») :
#   - Projet : <dépôt>\.agents\skills, lu par Codex ; Claude le lit via la jonction <dépôt>\.claude\skills.
#   - PC     : ~\.agents\skills est le dossier commun (lu par Codex). Claude y accède par une jonction
#              par skill dans ~\.claude\skills. Les skills de ~\.codex\skills (dont certains sont gérés
#              par d'autres outils) sont aussi reliés à Claude par jonction, sans être déplacés.
#   - Jamais touchés : ~\.claude\skills\synced (application Claude), ~\.codex\skills\.system (Codex),
#              les plugins (propres à chaque outil ; le plugin Unity est installé des deux côtés).
param([switch]$Apply)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$projectSkills = Join-Path $repo '.agents\skills'
$projectLink = Join-Path $repo '.claude\skills'
$shared = Join-Path $HOME '.agents\skills'
$codexDir = Join-Path $HOME '.codex\skills'
$claudeDir = Join-Path $HOME '.claude\skills'

function Test-Excluded([string]$name) { return ($name -like '.*') -or ($name -eq 'synced') }
function Test-Link($item) { return [bool]$item.LinkType }
function Get-LinkTarget($item) {
    $t = $item.Target; if ($t -is [array]) { $t = $t[0] }
    if (-not [System.IO.Path]::IsPathRooted($t)) { $t = Join-Path (Split-Path -Parent $item.FullName) $t }
    return [System.IO.Path]::GetFullPath($t)
}
function Get-Entries([string]$root) {
    if (-not (Test-Path -LiteralPath $root)) { return @() }
    @(Get-ChildItem -LiteralPath $root -Force | Where-Object { -not (Test-Excluded $_.Name) })
}
function Test-Skill([string]$dir) { return Test-Path -LiteralPath (Join-Path $dir 'SKILL.md') }
function Get-Inventory([string]$dir) {
    # Chemin relatif + type + empreinte, fichiers cachés compris : deux dossiers sont identiques si et seulement si ces listes le sont.
    $base = [System.IO.Path]::GetFullPath($dir).TrimEnd('\') + '\'
    @(Get-ChildItem -LiteralPath $dir -Recurse -Force | Sort-Object FullName | ForEach-Object {
        $rel = $_.FullName.Substring($base.Length)
        if (Test-Link $_) { "L|$rel|$(Get-LinkTarget $_)" }
        elseif ($_.PSIsContainer) { "D|$rel" }
        else { "F|$rel|$((Get-FileHash -LiteralPath $_.FullName).Hash)" }
    })
}
function Test-SameTree([string]$a, [string]$b) {
    return -not (Compare-Object (Get-Inventory $a) (Get-Inventory $b) -SyncWindow 100000)
}
function Test-Frontmatter([string]$dir) {
    $name = Split-Path -Leaf $dir
    $text = (Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $dir 'SKILL.md')).TrimStart([char]0xFEFF)
    if ($text -notmatch '(?s)^---[ \t]*\r?\n(.*?)\r?\n---') { return "pas de frontmatter" }
    $fm = $Matches[1]
    if ($fm -notmatch '(?m)^name:\s*["'']?([^"''\r\n]+)') { return "name manquant" }
    if ($Matches[1].Trim() -ne $name) { return "name '$($Matches[1].Trim())' différent du dossier" }
    if ($fm -notmatch '(?m)^description:') { return "description manquante" }
    return $null
}

$problems = New-Object System.Collections.Generic.List[string]
$plan = New-Object System.Collections.Generic.List[object]   # actions à appliquer

# 1. Projet
if (Test-Path -LiteralPath $projectLink) {
    $item = Get-Item -LiteralPath $projectLink -Force
    if (-not (Test-Link $item) -or (Get-LinkTarget $item) -ne [System.IO.Path]::GetFullPath($projectSkills)) {
        $problems.Add("Projet : $projectLink n'est pas une jonction vers $projectSkills")
    }
} else { $problems.Add("Projet : jonction absente ; lancer tools\setup-skills.ps1") }
$projectNames = @(Get-Entries $projectSkills | Where-Object { $_.PSIsContainer -and (Test-Skill $_.FullName) } | ForEach-Object { $_.Name })
foreach ($n in $projectNames) { $e = Test-Frontmatter (Join-Path $projectSkills $n); if ($e) { $problems.Add("Projet : $n : $e") } }

# 2. Sources PC (lues par Codex) : dossier commun puis ~\.codex\skills
if (-not (Test-Path -LiteralPath $shared)) {
    if ($Apply) { New-Item -ItemType Directory -Force $shared | Out-Null } else { $problems.Add("PC : $shared absent (créé par -Apply)") }
}
$sources = @{}
foreach ($pair in @(@($shared, 'commun'), @($codexDir, 'codex'))) {
    foreach ($e in Get-Entries $pair[0]) {
        if (-not $e.PSIsContainer -or -not (Test-Skill $e.FullName)) { continue }
        if ($sources.ContainsKey($e.Name)) { $problems.Add("Doublon PC : '$($e.Name)' existe dans ~\.agents\skills et ~\.codex\skills"); continue }
        $sources[$e.Name] = [pscustomobject]@{ Name = $e.Name; Path = [System.IO.Path]::GetFullPath($e.FullName); Owner = $pair[1] }
        $err = Test-Frontmatter $e.FullName; if ($err) { $problems.Add("PC : $($e.Name) : $err") }
    }
}

# 3. Entrées de ~\.claude\skills (hors synced et fichiers cachés)
$claudeEntries = @{}
foreach ($e in Get-Entries $claudeDir) { $claudeEntries[$e.Name] = $e }
foreach ($name in @($claudeEntries.Keys)) {
    $e = $claudeEntries[$name]
    $src = $sources[$name]
    if (Test-Link $e) {
        $target = Get-LinkTarget $e
        if (-not (Test-Path -LiteralPath $target)) { $plan.Add([pscustomobject]@{ Kind = 'unlink'; Name = $name; Why = "jonction cassée vers $target" }) }
        elseif (-not $src) { $problems.Add("Claude : '$name' est relié à $target, que Codex ne voit pas (déplacer la cible dans ~\.agents\skills)") }
        elseif ($target -ne $src.Path) { $problems.Add("Claude : '$name' pointe vers $target au lieu de $($src.Path)") }
    } elseif ($e.PSIsContainer -and (Test-Skill $e.FullName)) {
        if ($src) {
            if (Test-SameTree $e.FullName $src.Path) { $plan.Add([pscustomobject]@{ Kind = 'replace'; Name = $name; Why = "copie identique de $($src.Path)" }) }
            else { $problems.Add("Conflit : '$name' existe chez Claude et dans $($src.Path) avec un contenu différent (à trancher à la main)") }
        } else { $plan.Add([pscustomobject]@{ Kind = 'move'; Name = $name; Why = "propre à Claude, à mettre dans le dossier commun" }) }
    } else {
        $problems.Add("Claude : '$name' n'est ni un skill ni une jonction (fichier ou dossier sans SKILL.md) ; à vérifier à la main")
    }
}
foreach ($src in $sources.Values) {
    if (-not $claudeEntries.ContainsKey($src.Name)) { $plan.Add([pscustomobject]@{ Kind = 'link'; Name = $src.Name; Why = "$($src.Owner) : $($src.Path)" }) }
}

# 4. Un skill n'existe qu'à un seul endroit
$pcNames = @($sources.Keys) + @($plan | Where-Object Kind -eq 'move' | ForEach-Object { $_.Name })
foreach ($n in $projectNames) { if ($pcNames -contains $n) { $problems.Add("Doublon projet/PC : '$n' existe dans le dépôt et au niveau du PC (garder une seule copie)") } }

# Rapport
Write-Host ("Projet : {0} skills (Claude et Codex)" -f $projectNames.Count)
Write-Host ("PC     : {0} skills visibles par Codex, {1} entrées chez Claude" -f $sources.Count, $claudeEntries.Count)
foreach ($p in $problems) { Write-Host "PROBLÈME : $p" }
foreach ($a in $plan) { Write-Host ("À FAIRE  : {0} '{1}' ({2})" -f $a.Kind, $a.Name, $a.Why) }
if ($problems.Count -eq 0 -and $plan.Count -eq 0) { Write-Host "OK : Claude et Codex voient les mêmes skills."; exit 0 }
if (-not $Apply) { if ($plan.Count -gt 0) { Write-Host "Relancer avec -Apply pour faire les actions À FAIRE." }; exit 1 }

# Application. Sauvegarde vérifiée de tout ce qui sera supprimé ou déplacé.
$todo = @($plan | Where-Object { $_.Kind -in 'replace', 'move' })
if ($todo.Count -gt 0) {
    $backup = Join-Path $HOME ("skills-backup-" + (Get-Date -Format 'yyyyMMdd-HHmmss'))
    New-Item -ItemType Directory -Force $backup | Out-Null
    foreach ($a in $todo) {
        $from = Join-Path $claudeDir $a.Name; $to = Join-Path $backup "claude-$($a.Name)"
        Copy-Item -LiteralPath $from -Destination $to -Recurse -Force
        if (-not (Test-SameTree $from $to)) { throw "Sauvegarde incomplète de $from ; rien n'a été modifié." }
    }
    Write-Host "Sauvegarde vérifiée : $backup"
}
foreach ($a in $plan) {
    $link = Join-Path $claudeDir $a.Name
    switch ($a.Kind) {
        'unlink' { (Get-Item -LiteralPath $link -Force).Delete(); Write-Host "Jonction cassée supprimée : $($a.Name)" }
        'replace' {
            Remove-Item -LiteralPath $link -Recurse -Force
            New-Item -ItemType Junction -Path $link -Target $sources[$a.Name].Path | Out-Null
            Write-Host "Relié : $($a.Name) -> $($sources[$a.Name].Path)"
        }
        'move' {
            $dest = Join-Path $shared $a.Name
            Copy-Item -LiteralPath $link -Destination $dest -Recurse -Force   # copie vérifiée, sûre entre volumes
            if (-not (Test-SameTree $link $dest)) { throw "Copie incomplète de $link vers $dest ; arrêt." }
            Remove-Item -LiteralPath $link -Recurse -Force
            New-Item -ItemType Junction -Path $link -Target $dest | Out-Null
            Write-Host "Déplacé et relié : $($a.Name) -> $dest"
        }
    }
}
# Créations en dernier : les jonctions cassées ont déjà libéré leur nom.
foreach ($a in $plan | Where-Object Kind -eq 'link') {
    New-Item -ItemType Junction -Path (Join-Path $claudeDir $a.Name) -Target $sources[$a.Name].Path | Out-Null
    Write-Host "Relié : $($a.Name) -> $($sources[$a.Name].Path)"
}
foreach ($a in $plan | Where-Object Kind -eq 'unlink') {
    if ($sources.ContainsKey($a.Name) -and -not (Test-Path -LiteralPath (Join-Path $claudeDir $a.Name))) {
        New-Item -ItemType Junction -Path (Join-Path $claudeDir $a.Name) -Target $sources[$a.Name].Path | Out-Null
        Write-Host "Relié à nouveau : $($a.Name)"
    }
}
Write-Host "Terminé. Relancer sans -Apply pour vérifier."
