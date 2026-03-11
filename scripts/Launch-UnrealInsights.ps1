param(
    [string]$ProjectPath = "G:\workspace\ue5\Lyra",

    [string]$EngineRoot = "",

    [string]$TraceFile = "",

    [bool]$SyncPlugin = $true,

    [bool]$BuildInsights = $false,

    [switch]$AutoQuit,

    [switch]$DisableOtherPlugins
)

function Resolve-UProjectPath {
    param([Parameter(Mandatory = $true)][string]$InputPath)

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

function Resolve-EngineRoot {
    param([string]$Requested)

    $candidates = @()
    if (-not [string]::IsNullOrWhiteSpace($Requested)) {
        $candidates += $Requested
    }

    $candidates += @(
        "G:\workspace\repo\github\UnrealEngine",
        "E:\eworkspace\UnrealEngine",
        "C:\Program Files\Epic Games\UE_5.7",
        "C:\Program Files\Epic Games\UE_5.6",
        "C:\Program Files\Epic Games\UE_5.5"
    )

    foreach ($candidate in $candidates | Select-Object -Unique) {
        $insightsExe = Join-Path $candidate "Engine\Binaries\Win64\UnrealInsights.exe"
        $buildBat = Join-Path $candidate "Engine\Build\BatchFiles\Build.bat"
        if ((Test-Path $insightsExe) -and (Test-Path $buildBat)) {
            return $candidate
        }
    }

    throw "No valid engine root found. Tried: $($candidates -join ', ')"
}

function Resolve-TraceFile {
    param(
        [string]$RequestedTraceFile,
        [string]$ProjectDir
    )

    if (-not [string]::IsNullOrWhiteSpace($RequestedTraceFile)) {
        if (-not (Test-Path $RequestedTraceFile)) {
            throw "Trace file not found: $RequestedTraceFile"
        }

        return (Get-Item -LiteralPath $RequestedTraceFile).FullName
    }

    $profilingDir = Join-Path $ProjectDir "Saved\Profiling"
    if (-not (Test-Path $profilingDir)) {
        return ""
    }

    $latestTrace = Get-ChildItem -LiteralPath $profilingDir -Filter *.utrace -File -Recurse |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1

    if ($null -eq $latestTrace) {
        return ""
    }

    return $latestTrace.FullName
}

$resolvedEngineRoot = Resolve-EngineRoot -Requested $EngineRoot
$insightsExe = Join-Path $resolvedEngineRoot "Engine\Binaries\Win64\UnrealInsights.exe"
$buildBat = Join-Path $resolvedEngineRoot "Engine\Build\BatchFiles\Build.bat"

$uprojectPath = Resolve-UProjectPath -InputPath $ProjectPath
$projectDir = Split-Path -Parent $uprojectPath
$resolvedTraceFile = Resolve-TraceFile -RequestedTraceFile $TraceFile -ProjectDir $projectDir

$sourcePluginRoot = Split-Path -Parent $PSScriptRoot
$destinationPluginsDir = Join-Path $projectDir "Plugins"
$destinationPluginRoot = Join-Path $destinationPluginsDir "ObjectNetInspector"

if ($SyncPlugin) {
    Write-Host "Syncing plugin to project..."
    Write-Host "Source: $sourcePluginRoot"
    Write-Host "Destination: $destinationPluginRoot"

    New-Item -ItemType Directory -Path $destinationPluginsDir -Force | Out-Null
    New-Item -ItemType Directory -Path $destinationPluginRoot -Force | Out-Null

    Get-ChildItem -LiteralPath $sourcePluginRoot -Force |
        Where-Object { $_.Name -ne ".git" } |
        ForEach-Object {
            Copy-Item -LiteralPath $_.FullName -Destination $destinationPluginRoot -Recurse -Force
        }
}

if ($BuildInsights) {
    Write-Host "Building UnrealInsights Win64 Development..."
    & $buildBat UnrealInsights Win64 Development -Project="$uprojectPath" -EnablePlugins=ObjectNetInspector -WaitMutex -FromMsBuild
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed for UnrealInsights (exit code $LASTEXITCODE)."
    }
}

$args = @()
$args += "-project=""$uprojectPath"""

if (-not [string]::IsNullOrWhiteSpace($resolvedTraceFile)) {
    $args += "-OpenTraceFile=""$resolvedTraceFile"""
}
else {
    Write-Host "No trace file provided/found. UnrealInsights will open without an initial trace."
}

if ($AutoQuit.IsPresent) {
    $args += "-AutoQuit"
}

if ($DisableOtherPlugins.IsPresent) {
    $args += "-DisableAllPlugins"
    $args += "-EnablePlugins=ObjectNetInspector"
}

$args += "-log"

Write-Host "Launching UnrealInsights..."
Write-Host "Engine: $resolvedEngineRoot"
Write-Host "Project: $uprojectPath"
if (-not [string]::IsNullOrWhiteSpace($resolvedTraceFile)) {
    Write-Host "Trace: $resolvedTraceFile"
}
Write-Host "Args: $($args -join ' ')"

$insightsPluginDllPath = Join-Path $destinationPluginRoot "Binaries\\Win64\\UnrealInsights-ObjectNetInspector.dll"
if (-not (Test-Path $insightsPluginDllPath)) {
    throw "Missing Program plugin binary: $insightsPluginDllPath. Build UnrealInsights once with -BuildInsights:`$true (or run Build.bat with -EnablePlugins=ObjectNetInspector)."
}

& $insightsExe @args
exit $LASTEXITCODE
