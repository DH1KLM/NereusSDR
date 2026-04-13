# NereusSDR — Windows spectrum-render diagnostic
#
# Purpose
# -------
# Root-cause an issue where on Windows the main window and menus render fine
# but the spectrum/waterfall region is blank/dark. Two competing hypotheses:
#
#   A. NEREUS_GPU_SPECTRUM was compiled OFF because Qt6ShaderTools or
#      Qt6GuiPrivate was missing at CMake config time on this Windows Qt
#      install. The code paths the SpectrumWidget relies on are #ifdef-ed
#      out; you are running the CPU fallback which is literally
#      "dark background, nothing else drawn."
#
#   B. NEREUS_GPU_SPECTRUM was compiled ON but at runtime Direct3D11
#      initialization or shader pipeline creation fails for this GPU,
#      leaving the QRhiWidget blank.
#
# This script runs a single diagnostic pass and tells you which one,
# without any file spelunking on your part. You just run it and paste
# the "VERDICT" block at the end.
#
# Usage
# -----
#   powershell -ExecutionPolicy Bypass -File docs\debugging\windows-spectrum-diagnose.ps1
#   powershell -ExecutionPolicy Bypass -File docs\debugging\windows-spectrum-diagnose.ps1 -ExePath "C:\Program Files\NereusSDR\NereusSDR.exe"
#
# With no -ExePath it looks in these places in order:
#   1. build\Release\NereusSDR.exe  (local build, MSVC Release)
#   2. build\Debug\NereusSDR.exe    (local build, MSVC Debug)
#   3. build-clean\Release\NereusSDR.exe
#   4. build-clean\NereusSDR.exe
#   5. $env:ProgramFiles\NereusSDR\NereusSDR.exe
#   6. Whatever is on PATH
#
# Safe to run as many times as you like. Does not modify the app or
# your settings.

[CmdletBinding()]
param(
    [string]$ExePath = $null,
    [int]$WaitSeconds = 12
)

$ErrorActionPreference = 'Continue'

function Write-Section($title) {
    Write-Host ""
    Write-Host "══ $title ══" -ForegroundColor Cyan
}

function Resolve-Exe {
    param([string]$override)
    if ($override) {
        if (Test-Path $override) { return (Get-Item $override).FullName }
        Write-Host "Error: -ExePath '$override' does not exist" -ForegroundColor Red
        exit 1
    }
    $candidates = @(
        "build\Release\NereusSDR.exe",
        "build\Debug\NereusSDR.exe",
        "build-clean\Release\NereusSDR.exe",
        "build-clean\NereusSDR.exe",
        (Join-Path $env:ProgramFiles "NereusSDR\NereusSDR.exe")
    )
    foreach ($c in $candidates) {
        if (Test-Path $c) { return (Get-Item $c).FullName }
    }
    $cmd = Get-Command NereusSDR.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    Write-Host "Error: NereusSDR.exe not found in any default location." -ForegroundColor Red
    Write-Host "       Pass -ExePath 'C:\path\to\NereusSDR.exe' to point at it."
    exit 1
}

function Kill-Stale {
    Get-Process -Name NereusSDR -ErrorAction SilentlyContinue | ForEach-Object {
        Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue
    }
    Start-Sleep -Milliseconds 200
}

function Clear-Log {
    $logDir = Join-Path $env:LOCALAPPDATA "NereusSDR"
    $logFile = Join-Path $logDir "nereussdr.log"
    if (Test-Path $logFile) { Remove-Item $logFile -Force -ErrorAction SilentlyContinue }
    return $logFile
}

function Read-Log {
    param([string]$path)
    if (Test-Path $path) { return Get-Content $path -ErrorAction SilentlyContinue }
    return @()
}

function Run-Launch {
    param(
        [string]$exe,
        [hashtable]$envOverrides,
        [string]$label
    )
    Write-Section "LAUNCH: $label"
    Kill-Stale
    $logFile = Clear-Log
    $stderrFile = Join-Path $env:TEMP "nereussdr-diag-stderr-$(Get-Random).txt"
    $stdoutFile = Join-Path $env:TEMP "nereussdr-diag-stdout-$(Get-Random).txt"

    # Apply env overrides in a fresh scope for this launch only
    $saved = @{}
    foreach ($k in $envOverrides.Keys) {
        $saved[$k] = [Environment]::GetEnvironmentVariable($k, "Process")
        [Environment]::SetEnvironmentVariable($k, $envOverrides[$k], "Process")
    }

    Write-Host "  exe         : $exe"
    Write-Host "  env         :"
    foreach ($k in $envOverrides.Keys) {
        Write-Host "    $k = $($envOverrides[$k])"
    }
    Write-Host "  waiting     : $WaitSeconds seconds"
    Write-Host "  stderr file : $stderrFile"
    Write-Host ""

    $proc = Start-Process -FilePath $exe `
                          -PassThru `
                          -NoNewWindow `
                          -RedirectStandardError $stderrFile `
                          -RedirectStandardOutput $stdoutFile

    $startedAt = Get-Date
    $killed = $false
    for ($i = 0; $i -lt $WaitSeconds; $i++) {
        Start-Sleep -Seconds 1
        if ($proc.HasExited) { break }
    }
    if (-not $proc.HasExited) {
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
        $killed = $true
        Start-Sleep -Milliseconds 300
    }

    # Restore env
    foreach ($k in $envOverrides.Keys) {
        if ($null -eq $saved[$k]) {
            [Environment]::SetEnvironmentVariable($k, $null, "Process")
        } else {
            [Environment]::SetEnvironmentVariable($k, $saved[$k], "Process")
        }
    }

    $stderrContent = @()
    if (Test-Path $stderrFile) { $stderrContent = Get-Content $stderrFile -ErrorAction SilentlyContinue }
    $stdoutContent = @()
    if (Test-Path $stdoutFile) { $stdoutContent = Get-Content $stdoutFile -ErrorAction SilentlyContinue }
    $logContent = Read-Log $logFile

    $exitInfo = if ($killed) {
        "STILL RUNNING after $WaitSeconds s (process was then killed)"
    } else {
        "exited with code $($proc.ExitCode) after $([math]::Round(((Get-Date) - $startedAt).TotalSeconds, 1)) s"
    }

    Write-Host "  result      : $exitInfo"

    return [PSCustomObject]@{
        Label         = $label
        Stderr        = $stderrContent
        Stdout        = $stdoutContent
        AppLog        = $logContent
        Exited        = $proc.HasExited -and -not $killed
        ExitCode      = if ($proc.HasExited) { $proc.ExitCode } else { $null }
        KilledAfterWait = $killed
    }
}

function Grep-Markers {
    param([string[]]$lines, [string[]]$patterns)
    $hits = @()
    foreach ($line in $lines) {
        foreach ($p in $patterns) {
            if ($line -match $p) { $hits += $line; break }
        }
    }
    return $hits
}

function Diagnose {
    param([PSCustomObject]$launchA, [PSCustomObject]$launchB)

    Write-Section "RHI / D3D / shader markers — default run (D3D11)"
    $patternsRhi = @(
        'QRhi', 'rhi', 'Direct3D', 'D3D11', 'DXGI',
        'shader', 'pipeline', 'swapchain',
        'backend', 'adapter', 'device',
        'Failed', 'failed', 'error', 'Error',
        'NEREUS', 'spectrum', 'waterfall'
    )
    $hitsA = Grep-Markers @($launchA.Stderr + $launchA.Stdout + $launchA.AppLog) $patternsRhi
    if ($hitsA.Count -eq 0) {
        Write-Host "  (no rhi / d3d / shader / error markers found — see Hypothesis A below)"
    } else {
        foreach ($h in $hitsA | Select-Object -Unique | Select-Object -First 40) { Write-Host "  $h" }
    }

    Write-Section "RHI markers — forced OpenGL run (QT_RHI_BACKEND=opengl)"
    if ($launchB) {
        $hitsB = Grep-Markers @($launchB.Stderr + $launchB.Stdout + $launchB.AppLog) $patternsRhi
        if ($hitsB.Count -eq 0) {
            Write-Host "  (no markers found)"
        } else {
            foreach ($h in $hitsB | Select-Object -Unique | Select-Object -First 40) { Write-Host "  $h" }
        }
    }

    Write-Section "VERDICT"

    # Did the default run produce ANY rhi debug output at all?
    $sawRhi = ($hitsA | Where-Object { $_ -match 'QRhi|Direct3D|D3D11|DXGI|swapchain|backend|adapter' }).Count -gt 0
    $sawNereusGpuDefine = ($launchA.AppLog | Where-Object { $_ -match 'NEREUS_GPU_SPECTRUM|GPU spectrum' }).Count -gt 0
    $sawD3dFailure = ($hitsA | Where-Object { $_ -match '(Failed|failed|error).*(D3D|Direct3D|DXGI|shader|pipeline|adapter|device)' }).Count -gt 0

    if (-not $sawRhi) {
        Write-Host "  Hypothesis A (NEREUS_GPU_SPECTRUM compiled OFF) is most likely." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "  Evidence: even with QT_LOGGING_RULES=qt.rhi.*=debug set, no QRhi"
        Write-Host "  activity shows up in stderr or the app log. That means the"
        Write-Host "  SpectrumWidget never instantiated QRhiWidget — which happens only"
        Write-Host "  if the NEREUS_GPU_SPECTRUM #ifdef block was compiled out. The"
        Write-Host "  gate for that is in CMakeLists.txt:158-172 and fails if either"
        Write-Host "  Qt6ShaderTools OR Qt6GuiPrivate is missing at CMake config time."
        Write-Host ""
        Write-Host "  NEXT STEP: re-run CMake configure for your local build and look"
        Write-Host "  for either of these STATUS lines in the output:"
        Write-Host "    'QRhi support enabled — GPU spectrum/waterfall active'"
        Write-Host "    'Qt6 ShaderTools or GuiPrivate not found — GPU spectrum disabled'"
        Write-Host "  If you see the second one, install Qt Shader Tools via the Qt"
        Write-Host "  online installer (Qt 6.11 → Additional Libraries → Qt Shader"
        Write-Host "  Tools) and reconfigure. The pre-built 0.1.1 installer should"
        Write-Host "  be unaffected by this (CI has ShaderTools) — if it's also blank"
        Write-Host "  on this box, flip to Hypothesis B regardless of this verdict."
        Write-Host ""
    } elseif ($sawD3dFailure) {
        Write-Host "  Hypothesis B (Direct3D11 runtime initialization failing)." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "  Evidence: QRhi debug output shows a D3D11 / DXGI / shader /"
        Write-Host "  pipeline error during init. Check the lines above for the"
        Write-Host "  specific failure."
        Write-Host ""
        if ($launchB) {
            $bLooksBetter = $launchB.Stderr.Count -gt 0 -and ($launchB.Stderr -notmatch '(Failed|failed|error)')
            if ($bLooksBetter) {
                Write-Host "  And: the forced-OpenGL run did NOT show the same failure pattern."
                Write-Host "  That confirms the issue is D3D11-specific. Workaround:"
                Write-Host "  launch NereusSDR with QT_RHI_BACKEND=opengl set."
                Write-Host ""
                Write-Host "  NEXT STEP: decide whether to patch SpectrumWidget to fall"
                Write-Host "  back to OpenGL if D3D11 init fails, or require OpenGL as"
                Write-Host "  the default on Windows until the D3D11 pipeline is fixed."
            }
        }
    } else {
        Write-Host "  Inconclusive — QRhi debug output present but no obvious failure." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "  Paste the full RHI markers section above so we can read the"
        Write-Host "  actual init sequence. Also check the AppLog count:"
        Write-Host "    Default run app log lines: $($launchA.AppLog.Count)"
        if ($launchB) { Write-Host "    OpenGL run  app log lines: $($launchB.AppLog.Count)" }
    }

    Write-Section "FULL CAPTURE LOCATIONS"
    Write-Host "  app log after default run : $(Join-Path $env:LOCALAPPDATA 'NereusSDR\nereussdr.log')"
    Write-Host "  stderr from each run was captured into %TEMP% — see the launch"
    Write-Host "  sections above for exact file names. They stay on disk so you"
    Write-Host "  can attach them to a bug report if needed."
}

function Save-Bundle {
    param([PSCustomObject]$launchA, [PSCustomObject]$launchB, [string]$exe)
    $outDir = Join-Path $env:TEMP "nereussdr-diagnose-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
    New-Item -ItemType Directory -Force -Path $outDir | Out-Null

    "NereusSDR Windows spectrum diagnostic" | Out-File (Join-Path $outDir "README.txt")
    "Exe: $exe"                              | Out-File (Join-Path $outDir "README.txt") -Append
    "Generated: $(Get-Date)"                 | Out-File (Join-Path $outDir "README.txt") -Append
    "Host: $env:COMPUTERNAME"                | Out-File (Join-Path $outDir "README.txt") -Append
    "OS  : $((Get-CimInstance Win32_OperatingSystem).Caption) build $([System.Environment]::OSVersion.Version)" | Out-File (Join-Path $outDir "README.txt") -Append

    $launchA.Stderr | Out-File (Join-Path $outDir "run1-default-stderr.txt")
    $launchA.Stdout | Out-File (Join-Path $outDir "run1-default-stdout.txt")
    $launchA.AppLog | Out-File (Join-Path $outDir "run1-default-applog.txt")
    if ($launchB) {
        $launchB.Stderr | Out-File (Join-Path $outDir "run2-opengl-stderr.txt")
        $launchB.Stdout | Out-File (Join-Path $outDir "run2-opengl-stdout.txt")
        $launchB.AppLog | Out-File (Join-Path $outDir "run2-opengl-applog.txt")
    }

    $zip = "$outDir.zip"
    if (Test-Path $zip) { Remove-Item $zip -Force }
    Compress-Archive -Path "$outDir\*" -DestinationPath $zip -Force

    Write-Section "ATTACH THIS TO THE BUG REPORT"
    Write-Host "  $zip"
    return $zip
}

# ─── Main ───────────────────────────────────────────────────────────────────
Write-Host "NereusSDR Windows spectrum-render diagnostic" -ForegroundColor Green
Write-Host ""

$exe = Resolve-Exe $ExePath
Write-Host "Using NereusSDR at: $exe"

# Run 1 — default backend, full RHI debug logging
$envA = @{
    "QT_LOGGING_RULES"  = "qt.rhi.*=true;qt.qpa.*=true;default.debug=false"
    "QT_DEBUG_PLUGINS"  = "1"
}
$runA = Run-Launch -exe $exe -envOverrides $envA -label "default backend (Direct3D11)"

# Run 2 — force OpenGL, same RHI debug logging
$envB = @{
    "QT_LOGGING_RULES"  = "qt.rhi.*=true;qt.qpa.*=true;default.debug=false"
    "QT_RHI_BACKEND"    = "opengl"
}
$runB = Run-Launch -exe $exe -envOverrides $envB -label "forced OpenGL backend (QT_RHI_BACKEND=opengl)"

Diagnose -launchA $runA -launchB $runB
$bundle = Save-Bundle -launchA $runA -launchB $runB -exe $exe

Write-Host ""
Write-Host "Done." -ForegroundColor Green
Write-Host "Paste the VERDICT section back into the chat so we can move to Phase 3."
