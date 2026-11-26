# Rend les skills partages (.agents/skills, lus par Codex) visibles aussi par Claude Code (.claude/skills).
# A lancer une fois apres chaque clone du depot :  powershell -ExecutionPolicy Bypass -File tools\setup-skills.ps1
$root = Split-Path -Parent $PSScriptRoot
$source = Join-Path $root ".agents\skills"
$claudeDir = Join-Path $root ".claude"
$link = Join-Path $claudeDir "skills"

if (-not (Test-Path $source)) { Write-Error "Dossier introuvable : $source"; exit 1 }
if (-not (Test-Path $claudeDir)) { New-Item -ItemType Directory -Path $claudeDir | Out-Null }

if (Test-Path $link) {
    $item = Get-Item $link -Force
    if ($item.LinkType -eq "Junction") { Write-Host "Deja en place : $link -> $($item.Target)"; exit 0 }
    Write-Error "$link existe deja et n'est pas une jonction. Le deplacer ou le supprimer, puis relancer."
    exit 1
}

New-Item -ItemType Junction -Path $link -Target $source | Out-Null
Write-Host "OK : $link -> $source"
