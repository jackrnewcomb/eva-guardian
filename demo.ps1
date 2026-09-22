# EVA Guardian - scripted, timed walkthrough of every feature.
# Usage (from the eva/ repo root, after building eva_guardian):
#   powershell -ExecutionPolicy Bypass -File .\demo.ps1
#
# Only stdin is redirected (via a real .NET Process, not a PowerShell native-command
# pipe) so the child's stdout stays attached to this console and prints live, and
# each command is written+flushed individually so Start-Sleep pacing is real.

$exePath = Join-Path $PSScriptRoot "build\Debug\eva_guardian.exe"
if (-not (Test-Path $exePath)) {
    Write-Error "Build eva_guardian first: cmake -S . -B build; cmake --build build --config Debug"
    exit 1
}

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $exePath
$psi.RedirectStandardInput = $true
$psi.UseShellExecute = $false

$process = [System.Diagnostics.Process]::Start($psi)

function Send-DemoCommand([string]$command) {
    $process.StandardInput.WriteLine($command)
    $process.StandardInput.Flush()
}

Write-Host "`n=== 1) Baseline dashboard: all suits nominal ===" -ForegroundColor Cyan
Send-DemoCommand "dashboard"
Start-Sleep -Seconds 3

Write-Host "`n=== 2) Gradual O2 decline on suit-1: predictive alert should fire before the reactive one ===" -ForegroundColor Cyan
Send-DemoCommand "suit-1 o2 drift -0.3"
Start-Sleep -Seconds 12

Write-Host "`n=== 3) Earth-independence contrast: watch for [EARTH RELAY] ~8s after the [LOCAL] alerts above ===" -ForegroundColor Cyan
Start-Sleep -Seconds 6

Write-Host "`n=== 4) Second simultaneous anomaly on suit-1: composite risk escalation ===" -ForegroundColor Cyan
Send-DemoCommand "suit-1 pressure 25"
Start-Sleep -Seconds 3

Write-Host "`n=== 5) Dashboard: suit-1 flagged across systems, suit-2/suit-3 still nominal ===" -ForegroundColor Cyan
Send-DemoCommand "dashboard"
Start-Sleep -Seconds 3

Write-Host "`n=== 6) Recover suit-1: watch INFO recovery + composite de-escalation ===" -ForegroundColor Cyan
Send-DemoCommand "suit-1 reset"
Start-Sleep -Seconds 3

Write-Host "`n=== 7) Independent multi-suit anomalies at the same time ===" -ForegroundColor Cyan
Send-DemoCommand "suit-2 thermal 40"
Send-DemoCommand "suit-3 pressure 25"
Start-Sleep -Seconds 3

Write-Host "`n=== 8) Final dashboard snapshot ===" -ForegroundColor Cyan
Send-DemoCommand "dashboard"
Start-Sleep -Seconds 3

Write-Host "`n=== 9) Quit: post-EVA debrief report ===" -ForegroundColor Cyan
Send-DemoCommand "quit"

$process.WaitForExit()
