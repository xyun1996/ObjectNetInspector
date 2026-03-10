param(
    [string]$ProjectPath = "G:\workspace\ue5\Lyra",

    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.7",

    [string]$ReportOutputPath = "",

    [string]$TestName = "ObjectNetInspector.Provider.FilteringAndAggregation",

    [string]$EditorTarget = "LyraEditor",

    [switch]$SkipBuild
)

function Resolve-UProjectPath {
    param([Parameter(Mandatory=$true)][string]$InputPath)

    if (-not (Test-Path $InputPath)) {
        throw "Project path not found: $InputPath"
    }

    $item = Get-Item -LiteralPath $InputPath
    if ($item.PSIsContainer -eq $false) {
        if ($item.Extension -ne ".uproject") {
            throw "Expected a .uproject file, got: $InputPath"
        }

        return $item.FullName
    }

    $uprojects = Get-ChildItem -LiteralPath $item.FullName -Filter *.uproject -File
    if ($uprojects.Count -eq 0) {
        throw "No .uproject file found under directory: $($item.FullName)"
    }

    if ($uprojects.Count -gt 1) {
        throw "Multiple .uproject files found under directory: $($item.FullName). Pass an explicit .uproject path."
    }

    return $uprojects[0].FullName
}

$editorCmd = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$buildBat = Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat"
if (-not (Test-Path $editorCmd)) {
    throw "UnrealEditor-Cmd.exe not found: $editorCmd"
}
if (-not (Test-Path $buildBat)) {
    throw "Build.bat not found: $buildBat"
}

$uprojectPath = Resolve-UProjectPath -InputPath $ProjectPath
$projectDir = Split-Path -Parent $uprojectPath

$sourcePluginRoot = Split-Path -Parent $PSScriptRoot
$destinationPluginsDir = Join-Path $projectDir "Plugins"
$destinationPluginRoot = Join-Path $destinationPluginsDir "ObjectNetInspector"

Write-Host "Syncing plugin to project..."
Write-Host "Source: $sourcePluginRoot"
Write-Host "Destination: $destinationPluginRoot"

New-Item -ItemType Directory -Path $destinationPluginsDir -Force | Out-Null

if (Test-Path $destinationPluginRoot) {
    Remove-Item -LiteralPath $destinationPluginRoot -Recurse -Force
}

New-Item -ItemType Directory -Path $destinationPluginRoot -Force | Out-Null

Get-ChildItem -LiteralPath $sourcePluginRoot -Force |
    Where-Object { $_.Name -ne ".git" } |
    ForEach-Object {
        Copy-Item -LiteralPath $_.FullName -Destination $destinationPluginRoot -Recurse -Force
    }

if (-not $SkipBuild) {
    Write-Host "Building target before test: $EditorTarget Win64 Development"
    & $buildBat $EditorTarget Win64 Development -Project="$uprojectPath" -WaitMutex -FromMsBuild
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed for target '$EditorTarget' (exit code $LASTEXITCODE)."
    }
}

if ([string]::IsNullOrWhiteSpace($ReportOutputPath)) {
    $ReportOutputPath = Join-Path $projectDir "Saved\AutomationReports"
}

Write-Host "Running test: $TestName"
Write-Host "Project: $uprojectPath"
Write-Host "Engine: $EngineRoot"
Write-Host "Report: $ReportOutputPath"

& $editorCmd `
    $uprojectPath `
    -unattended -nop4 -nosplash -nullrhi `
    -ExecCmds="Automation RunTests $TestName; Quit" `
    -TestExit="Automation Test Queue Empty" `
    -log `
    -ReportOutputPath="$ReportOutputPath"

exit $LASTEXITCODE