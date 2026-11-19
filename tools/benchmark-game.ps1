param(
    [string]$Game = (Join-Path $PSScriptRoot '..\build\Release\BackroomsMazeGame.exe'),
    [ValidateSet('Flythrough', 'LateLevel')]
    [string]$Scenario = 'Flythrough',
    [ValidateRange(5, 300)]
    [int]$Seconds = 60,
    [switch]$NativeFullscreen
)

$ErrorActionPreference = 'Stop'
$gamePath = (Resolve-Path -LiteralPath $Game).Path
$gameDirectory = Split-Path -Parent $gamePath
$output = Join-Path $gameDirectory ('benchmark-' + [Guid]::NewGuid().ToString('N').Substring(0, 8))
$settingsDirectory = Join-Path $output 'state\BackroomsMazeScreensaver'
New-Item -ItemType Directory -Path $settingsDirectory | Out-Null
$statePath = (Resolve-Path -LiteralPath (Join-Path $output 'state')).Path

$ini = Get-Content -LiteralPath (Join-Path $gameDirectory 'BackroomsMaze.ini') -Raw
$ini = $ini -replace '(?m)^Fullscreen=.*$', "Fullscreen=$([int]$NativeFullscreen.IsPresent)"
$ini = $ini -replace '(?m)^ResolutionWidth=.*$', 'ResolutionWidth=1920'
$ini = $ini -replace '(?m)^ResolutionHeight=.*$', 'ResolutionHeight=1080'
$ini = $ini -replace '(?m)^FrameRateLimit=.*$', 'FrameRateLimit=144'
$ini = $ini -replace '(?m)^RandomSeed=.*$', 'RandomSeed=325127'
$ini = $ini -replace '(?m)^RunVariation=.*$', 'RunVariation=0'
$ini = $ini -replace '(?m)^MapOverlay=.*$', 'MapOverlay=0'
Set-Content -LiteralPath (Join-Path $settingsDirectory 'BackroomsMaze.ini') -Value $ini -Encoding ASCII

$start = New-Object System.Diagnostics.ProcessStartInfo
$start.FileName = $gamePath
$start.WorkingDirectory = $gameDirectory
$start.UseShellExecute = $false
$start.EnvironmentVariables['LOCALAPPDATA'] = $statePath
$start.EnvironmentVariables['BACKROOMS_PROFILE_DIRECTORY'] = $output
if ($Scenario -eq 'Flythrough') {
    $start.EnvironmentVariables.Remove('BACKROOMS_AUTOPLAY_BENCHMARK')
    $start.EnvironmentVariables['BACKROOMS_BENCHMARK_DEMO'] = '1'
    $start.EnvironmentVariables['BACKROOMS_BENCHMARK_DEMO_SECONDS'] = [string]$Seconds
    $timeColumn = 'benchmark_seconds'
} else {
    $start.EnvironmentVariables.Remove('BACKROOMS_BENCHMARK_DEMO')
    $start.EnvironmentVariables['BACKROOMS_AUTOPLAY_BENCHMARK'] = '1'
    $start.EnvironmentVariables['BACKROOMS_AUTOPLAY_BENCHMARK_SECONDS'] = [string]$Seconds
    $start.EnvironmentVariables['BACKROOMS_AUTOPLAY_START_LEVEL'] = '5'
    $start.EnvironmentVariables['BACKROOMS_AUTOPLAY_BOSS_LEVEL'] = '5'
    $start.EnvironmentVariables['BACKROOMS_AUTOPLAY_EXPLORE_LEVEL'] = '1'
    $timeColumn = 'autoplay_seconds'
}

$process = [System.Diagnostics.Process]::Start($start)
try {
    if (-not $process.WaitForExit((120 + 4 * $Seconds) * 1000)) {
        $process.CloseMainWindow() | Out-Null
        if (-not $process.WaitForExit(5000)) {
            $process.Kill()
            $process.WaitForExit()
        }
        throw 'The benchmark did not finish before the timeout.'
    }
    if ($process.ExitCode -ne 0) { throw "Backrooms returned exit code $($process.ExitCode)." }
} finally {
    $process.Dispose()
    $state = Get-Item -LiteralPath $statePath
    $statePrefix = $state.FullName.TrimEnd('\') + '\'
    $outputPrefix = (Resolve-Path -LiteralPath $output).Path.TrimEnd('\') + '\'
    if ($statePrefix -ne ($outputPrefix + 'state\') -or $state.Attributes.HasFlag([IO.FileAttributes]::ReparsePoint)) {
        throw "Unexpected benchmark state path $statePath"
    }
    if (Get-ChildItem -LiteralPath $statePath -Recurse -Force | Where-Object { $_.Attributes.HasFlag([IO.FileAttributes]::ReparsePoint) }) {
        throw "Unexpected link in benchmark state $statePath"
    }
    Remove-Item -LiteralPath $statePath -Recurse -Force
}

$frames = @(Import-Csv -LiteralPath (Join-Path $output 'BackroomsMaze.runtime_profile.csv') |
    Where-Object { [double]$_.$timeColumn -ge 2.0 -and $_.level_running -eq '1' })
$intervals = @($frames | ForEach-Object { [double]$_.frame_interval_ms } | Where-Object { $_ -gt 0 } | Sort-Object)
if ($intervals.Count -eq 0) { throw "No timed gameplay frames in $output." }
if ($Scenario -eq 'Flythrough' -and ($frames | Where-Object { $_.camera_in_open_tile -ne '1' })) {
    throw "Fly-through passed through a closed tile. See $output."
}
[pscustomobject]@{
    Scenario = $Scenario
    Frames = $intervals.Count
    FPS = [math]::Round(1000.0 / ($intervals | Measure-Object -Average).Average, 1)
    FrameP95Ms = $intervals[[int][math]::Ceiling(0.95 * $intervals.Count) - 1]
    FrameP99Ms = $intervals[[int][math]::Ceiling(0.99 * $intervals.Count) - 1]
    Profiles = $output
}
