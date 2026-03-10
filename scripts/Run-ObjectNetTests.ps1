param(
    [Parameter(Mandatory=$true)]
    [string]$UProject,

    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.7",

    [string]$ReportOutputPath = "",

    [string]$TestName = "ObjectNetInspector.Provider.FilteringAndAggregation"
)

$editorCmd = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $editorCmd)) {
    throw "UnrealEditor-Cmd.exe not found: $editorCmd"
}

if (-not (Test-Path $UProject)) {
    throw "uproject not found: $UProject"
}

if ([string]::IsNullOrWhiteSpace($ReportOutputPath)) {
    $projectDir = Split-Path -Parent $UProject
    $ReportOutputPath = Join-Path $projectDir "Saved\AutomationReports"
}

Write-Host "Running test: $TestName"
Write-Host "Project: $UProject"
Write-Host "Engine: $EngineRoot"
Write-Host "Report: $ReportOutputPath"

& $editorCmd `
    $UProject `
    -unattended -nop4 -nosplash -nullrhi `
    -ExecCmds="Automation RunTests $TestName; Quit" `
    -TestExit="Automation Test Queue Empty" `
    -log `
    -ReportOutputPath="$ReportOutputPath"

exit $LASTEXITCODE