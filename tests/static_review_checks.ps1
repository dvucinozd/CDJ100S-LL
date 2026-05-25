$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$rekordbox = Get-Content -Raw (Join-Path $repoRoot 'Src/rekordbox.c')
$display = Get-Content -Raw (Join-Path $repoRoot 'Src/display.c')
$it = Get-Content -Raw (Join-Path $repoRoot 'Src/stm32f7xx_it.c')
$audio = Get-Content -Raw (Join-Path $repoRoot 'Src/stm32746g_discovery_audio.c')
$sd = Get-Content -Raw (Join-Path $repoRoot 'Src/bsp_driver_sd.c')

$failures = @()

if ($rekordbox -match 'f_read\s*\(\s*&MyFile\s*,\s*&lowp_wavebuffer\[0\]\s*,\s*rekordbox\.lowp_spectrum_size') {
    $failures += 'Rekordbox low-resolution spectrum read still uses unbounded lowp_spectrum_size.'
}

if ($display -match 'strcat\s*\(') {
    $failures += 'Display menu still uses strcat for menu labels.'
}

$extiMatch = [regex]::Match($it, 'void\s+EXTI15_10_IRQHandler\s*\([^)]*\)\s*\{(?<body>[\s\S]*?)\n\}')
if (-not $extiMatch.Success) {
    $failures += 'EXTI15_10_IRQHandler was not found.'
} else {
    $extiBody = $extiMatch.Groups['body'].Value
    foreach ($pattern in @('GoToPosition\s*\(', 'BSP_AUDIO_OUT_', '\(float\)', 'Track_number\s*=', 'menu_mode\s*(\+\+|--|=)')) {
        if ($extiBody -match $pattern) {
            $failures += "EXTI15_10_IRQHandler still contains foreground touch logic matching '$pattern'."
        }
    }
}

if ($it -notmatch 'ProcessPendingTouch\s*\(') {
    $failures += 'Foreground touch processor ProcessPendingTouch is missing.'
}

if ($audio -match 'memset\s*\(\s*BufferCtl\.buff\s*\+\s*unInDataLeft\s*\+\s*unRead\s*,\s*unSpaceLeft\s*-\s*unRead\s*,\s*0\s*\)') {
    $failures += 'Mp3FillReadBuffer still passes memset fill value and length in the wrong order.'
}

foreach ($callback in @('BSP_SD_AbortCallback', 'BSP_SD_WriteCpltCallback', 'BSP_SD_ReadCpltCallback')) {
    if ($sd -notmatch "__weak\s+void\s+$callback\s*\(void\)\s*;") {
        $failures += "$callback is missing a forward prototype before HAL callback use."
    }
}

if ($failures.Count -gt 0) {
    $failures | ForEach-Object { Write-Host "FAIL: $_" }
    exit 1
}

Write-Host 'Static review checks passed.'
