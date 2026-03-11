param(
    [string]$ProjectPath = "G:\workspace\ue5\Lyra\Lyra.uproject",
    [string]$EngineRoot = "",
    [string]$TraceFile = "",
    [bool]$SyncPlugin = $true,
    [bool]$BuildInsights = $false
)

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
        if (Test-Path (Join-Path $candidate "Engine\Binaries\Win64\UnrealInsights.exe")) {
            return $candidate
        }
    }

    throw "No valid engine root found. Tried: $($candidates -join ', ')"
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$launchScript = Join-Path $PSScriptRoot "Launch-UnrealInsights.ps1"
if (-not (Test-Path -LiteralPath $launchScript)) {
    throw "Launch script not found: $launchScript"
}

$resolvedEngineRoot = Resolve-EngineRoot -Requested $EngineRoot
$insightsLogDir = Join-Path $resolvedEngineRoot "Engine\Programs\UnrealInsights\Saved\Logs"

$launchArgs = @{
    ProjectPath = $ProjectPath
    EngineRoot = $resolvedEngineRoot
    SyncPlugin = $SyncPlugin
    BuildInsights = $BuildInsights
    AutoQuit = $true
}

if (-not [string]::IsNullOrWhiteSpace($TraceFile)) {
    $launchArgs["TraceFile"] = $TraceFile
}
else {
    $launchArgs["NoAutoTraceScan"] = $true
}

if ($SyncPlugin) {
    $projectDir = Split-Path -Parent $ProjectPath
    if ((Test-Path -LiteralPath $ProjectPath) -and (Test-Path -LiteralPath $ProjectPath -PathType Leaf) -and ([IO.Path]::GetExtension($ProjectPath) -eq ".uproject")) {
        $projectDir = Split-Path -Parent $ProjectPath
    }
    elseif ((Test-Path -LiteralPath $ProjectPath) -and (Test-Path -LiteralPath $ProjectPath -PathType Container)) {
        $projectDir = $ProjectPath
    }

    $programDllPath = Join-Path $projectDir "Plugins\ObjectNetInspector\Binaries\Win64\UnrealInsights-ObjectNetInspector.dll"
    if (-not (Test-Path -LiteralPath $programDllPath) -and (-not $BuildInsights)) {
        Write-Host "Program plugin binary missing; enabling one-time UnrealInsights build for smoke check."
        $launchArgs["BuildInsights"] = $true
    }
}

Write-Host "Running UnrealInsights smoke launch..."
Write-Host "Engine: $resolvedEngineRoot"
Write-Host "Project: $ProjectPath"
if (-not [string]::IsNullOrWhiteSpace($TraceFile)) {
    Write-Host "Trace: $TraceFile"
}

& $launchScript @launchArgs
if ($LASTEXITCODE -ne 0) {
    throw "UnrealInsights launch failed with exit code $LASTEXITCODE."
}

if (-not (Test-Path -LiteralPath $insightsLogDir)) {
    throw "Insights log directory not found: $insightsLogDir"
}

$latestLog = Get-ChildItem -LiteralPath $insightsLogDir -Filter *.log -File |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1
if ($null -eq $latestLog) {
    throw "No UnrealInsights log found in: $insightsLogDir"
}

$logText = Get-Content -LiteralPath $latestLog.FullName -Raw
$requiredMarkers = @(
    "Mounting Project plugin ObjectNetInspector",
    "ObjectNetInspector module started and tab spawner registered."
)

$missing = @()
foreach ($marker in $requiredMarkers) {
    if (-not $logText.Contains($marker)) {
        $missing += $marker
    }
}

if ($missing.Count -gt 0) {
    throw "Smoke check failed. Missing log markers in $($latestLog.FullName): $($missing -join '; ')"
}

$netHints = @("NetProfiler", "Networking Insights", "NetTraceVersion")
$matchedHints = @()
foreach ($hint in $netHints) {
    if ($logText.Contains($hint)) {
        $matchedHints += $hint
    }
}

Write-Host "Smoke check passed."
Write-Host "Log: $($latestLog.FullName)"
Write-Host "Found markers: $($requiredMarkers -join ', ')"
if ($matchedHints.Count -gt 0) {
    Write-Host "Net-related log hints: $($matchedHints -join ', ')"
}
else {
    Write-Host "Net-related log hints not found in Insights log. This does not prove trace contains NetProfiler data; validate in Session Info."
}

exit 0
