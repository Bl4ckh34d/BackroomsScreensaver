param(
    [string]$Configuration = "Release",
    [string]$BuildPath = "build",
    [switch]$SkipBuild,
    [switch]$SkipSelfTest
)

$ErrorActionPreference = "Stop"

$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
$ascii = [System.Text.ASCIIEncoding]::new()
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$buildDir = if ([System.IO.Path]::IsPathRooted($BuildPath)) {
    [System.IO.Path]::GetFullPath($BuildPath)
} else {
    [System.IO.Path]::GetFullPath((Join-Path $root $BuildPath))
}
$distDir = Join-Path $root "dist"
$stageDir = Join-Path $distDir "BackroomsMazeScreensaver"
$tempDir = Join-Path $distDir "_package-tmp"
$zipPath = Join-Path $distDir "BackroomsMazeScreensaver-latest.zip"
$legacyZipPath = Join-Path $distDir "BackroomsMazeScreensaver-package.zip"

function Assert-UnderPath {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Base
    )

    $fullPath = [System.IO.Path]::GetFullPath($Path)
    $fullBase = [System.IO.Path]::GetFullPath($Base).TrimEnd('\') + '\'
    if (-not $fullPath.StartsWith($fullBase, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to operate outside ${fullBase}: ${fullPath}"
    }
}

function Write-AsciiFile {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Text
    )

    [System.IO.File]::WriteAllText($Path, $Text, $ascii)
}

Add-Type -TypeDefinition @'
using System.Runtime.InteropServices;
public static class BackroomsMazeIniNative {
  [DllImport("kernel32.dll", CharSet=CharSet.Unicode, SetLastError=true)]
  public static extern bool WritePrivateProfileString(string section, string key, string value, string fileName);
}
'@

function Set-IniValue {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Section,
        [Parameter(Mandatory = $true)][string]$Key,
        [Parameter(Mandatory = $true)][AllowEmptyString()][string]$Value
    )

    [BackroomsMazeIniNative]::WritePrivateProfileString($Section, $Key, $Value, $Path) | Out-Null
}

function Copy-RuntimeFile {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath
    )

    $source = Join-Path $root $RelativePath
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
        throw "Required runtime file is missing: $RelativePath"
    }

    $destination = Join-Path $stageDir $RelativePath
    $destinationParent = Split-Path -Parent $destination
    New-Item -ItemType Directory -Force -Path $destinationParent | Out-Null
    Copy-Item -LiteralPath $source -Destination $destination -Force
}

function Invoke-StageSelfTest {
    param(
        [Parameter(Mandatory = $true)][string]$ExePath,
        [Parameter(Mandatory = $true)][string]$WorkingDirectory,
        [Parameter(Mandatory = $true)][string]$FailureMessage
    )

    $run = Start-Process -FilePath $ExePath -ArgumentList "/selftest" -WorkingDirectory $WorkingDirectory -PassThru
    if (-not $run.WaitForExit(120000)) {
        try { Stop-Process -Id $run.Id -Force -ErrorAction SilentlyContinue } catch {}
        throw "$FailureMessage timed out"
    }
    if ($run.ExitCode -ne 0) {
        throw "$FailureMessage with exit code $($run.ExitCode)"
    }
}

function Copy-CacheFiles {
    param(
        [Parameter(Mandatory = $true)][string]$SourceCache,
        [Parameter(Mandatory = $true)][string]$StagePath
    )

    $shaderCache = Join-Path $StagePath "ShaderCache"
    $textureCache = Join-Path $StagePath "TextureCache"
    New-Item -ItemType Directory -Force -Path $shaderCache | Out-Null
    New-Item -ItemType Directory -Force -Path $textureCache | Out-Null

    if (Test-Path -LiteralPath $SourceCache -PathType Container) {
        Get-ChildItem -LiteralPath $SourceCache -Filter "BackroomsMaze_*.cso" -File |
            Copy-Item -Destination $shaderCache -Force
        Get-ChildItem -LiteralPath $SourceCache -Filter "BackroomsMaze_textures.bin" -File |
            Copy-Item -Destination $textureCache -Force
    }
}

function New-PackagedSkullMesh {
    param(
        [Parameter(Mandatory = $true)][string]$StagePath,
        [Parameter(Mandatory = $true)][string]$StageExe,
        [Parameter(Mandatory = $true)][string]$StageIni,
        [Parameter(Mandatory = $true)][string]$SourceCache,
        [Parameter(Mandatory = $true)][string]$OutputRelativePath,
        [Parameter(Mandatory = $true)][string]$AltChance
    )

    New-Item -ItemType Directory -Force -Path $SourceCache | Out-Null
    if (Test-Path -LiteralPath $SourceCache -PathType Container) {
        Get-ChildItem -LiteralPath $SourceCache -Filter "BackroomsMaze_skullmesh_*.bin" -File |
            Remove-Item -Force
    }
    Set-IniValue -Path $StageIni -Section "Monster" -Key "AlternateSkullChance" -Value $AltChance
    Invoke-StageSelfTest -ExePath $StageExe -WorkingDirectory $StagePath -FailureMessage "Skull mesh cache generation failed"

    $meshCache = Get-ChildItem -LiteralPath $SourceCache -Filter "BackroomsMaze_skullmesh_*.bin" -File |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
    if (-not $meshCache) {
        throw "Skull mesh cache was not generated for AlternateSkullChance=$AltChance"
    }

    $destination = Join-Path $StagePath $OutputRelativePath
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destination) | Out-Null
    Copy-Item -LiteralPath $meshCache.FullName -Destination $destination -Force

    $meshCacheDestination = Join-Path (Join-Path $StagePath "MeshCache") $meshCache.Name
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $meshCacheDestination) | Out-Null
    Copy-Item -LiteralPath $meshCache.FullName -Destination $meshCacheDestination -Force
}

if (-not $SkipBuild) {
    if (-not (Test-Path -LiteralPath (Join-Path $buildDir "CMakeCache.txt") -PathType Leaf)) {
        & cmake -S $root -B $buildDir
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configure failed with exit code $LASTEXITCODE"
        }
    }
    & cmake --build $buildDir --config $Configuration --parallel
    if ($LASTEXITCODE -ne 0) {
        throw "Release build failed with exit code $LASTEXITCODE"
    }
}

$releaseDir = Join-Path $buildDir $Configuration
$scrPath = Join-Path $releaseDir "BackroomsMaze.scr"
if (-not (Test-Path -LiteralPath $scrPath -PathType Leaf)) {
    throw "Built screensaver was not found: $scrPath"
}

$buildTestExe = Join-Path $releaseDir "BackroomsMazeTest.exe"
Copy-Item -LiteralPath $scrPath -Destination $buildTestExe -Force
if (-not $SkipSelfTest) {
    $buildTest = Start-Process -FilePath $buildTestExe -ArgumentList "/selftest" -PassThru
    if (-not $buildTest.WaitForExit(120000)) {
        try { Stop-Process -Id $buildTest.Id -Force -ErrorAction SilentlyContinue } catch {}
        throw "Build self-test timed out"
    }
    if ($buildTest.ExitCode -ne 0) {
        throw "Build self-test failed with exit code $($buildTest.ExitCode)"
    }
}

New-Item -ItemType Directory -Force -Path $distDir | Out-Null
Assert-UnderPath -Path $stageDir -Base $distDir
Assert-UnderPath -Path $tempDir -Base $distDir

if (Test-Path -LiteralPath $stageDir) {
    Remove-Item -LiteralPath $stageDir -Recurse -Force
}
if (Test-Path -LiteralPath $tempDir) {
    Remove-Item -LiteralPath $tempDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $stageDir | Out-Null
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null

Copy-Item -LiteralPath $scrPath -Destination (Join-Path $stageDir "BackroomsMaze.scr") -Force

$iniGenDir = Join-Path $tempDir "ini-gen"
New-Item -ItemType Directory -Force -Path $iniGenDir | Out-Null
$iniGenExe = Join-Path $iniGenDir "BackroomsMazeTest.exe"
Copy-Item -LiteralPath $scrPath -Destination $iniGenExe -Force
$iniGen = Start-Process -FilePath $iniGenExe -ArgumentList "/makeini" -WorkingDirectory $iniGenDir -Wait -PassThru
if ($iniGen.ExitCode -ne 0) {
    throw "Default INI generation failed with exit code $($iniGen.ExitCode)"
}

$generatedIni = Join-Path $iniGenDir "BackroomsMaze.ini"
if (-not (Test-Path -LiteralPath $generatedIni -PathType Leaf)) {
    throw "Default INI was not generated: $generatedIni"
}

$preferredIni = @(
    (Join-Path $root "BackroomsMaze_backup.ini"),
    (Join-Path $buildDir "BackroomsMaze_backup.ini"),
    (Join-Path (Join-Path $root "build") "BackroomsMaze_backup.ini")
) | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1

$mergeIniScript = Join-Path $PSScriptRoot "merge-ini-defaults.ps1"
$iniSource = if ($preferredIni) { $preferredIni } else { $generatedIni }
& powershell -NoProfile -ExecutionPolicy Bypass -File $mergeIniScript `
    -GeneratedIni $generatedIni `
    -PreferredIni $iniSource `
    -OutputIni (Join-Path $stageDir "BackroomsMaze.ini") `
    -PackagePaths
if ($LASTEXITCODE -ne 0) {
    throw "Default INI merge failed with exit code $LASTEXITCODE"
}

$runtimeFiles = @(
    "assets\branding\NeuralForge_Solutions.png",
    "assets\branding\NeuralForge_Solutions.svg",
    "assets\PBRs\backrooms_wall_color_4k.jpg",
    "assets\PBRs\backrooms_wall_height_4k.png",
    "assets\PBRs\backrooms_wall_normal_directx_4k.png",
    "assets\PBRs\backrooms_wall_roughness_4k.jpg",
    "assets\PBRs\backrooms_carpet_color_4k.jpg",
    "assets\PBRs\backrooms_carpet_height_4k.png",
    "assets\PBRs\backrooms_carpet_normal_directx_4k.png",
    "assets\PBRs\backrooms_carpet_roughness_4k.jpg",
    "assets\PBRs\backrooms_ceiling_color_4k.jpg",
    "assets\PBRs\backrooms_ceiling_height_4k.png",
    "assets\PBRs\backrooms_ceiling_normal_directx_4k.png",
    "assets\PBRs\backrooms_ceiling_roughness_4k.jpg",
    "assets\PBRs\downloads\Others001_4k\others_0001_ao_4k.jpg",
    "assets\PBRs\downloads\Others001_4k\others_0001_color_4k.jpg",
    "assets\PBRs\downloads\Others001_4k\others_0001_height_4k.png",
    "assets\PBRs\downloads\Others001_4k\others_0001_normal_directx_4k.png",
    "assets\PBRs\downloads\Others001_4k\others_0001_roughness_4k.jpg",
    "assets\PBRs\downloads\t_flashlightpattern.png",
    "assets\images\8pages\page1.jpg",
    "assets\images\8pages\page2.jpg",
    "assets\images\8pages\page3.jpg",
    "assets\images\8pages\page4.jpg",
    "assets\images\8pages\page5.jpg",
    "assets\images\8pages\page6.jpg",
    "assets\images\8pages\page7.jpg",
    "assets\images\8pages\page8.jpg",
    "assets\models\monster_face_mask\horror_mask.obj",
    "assets\models\monster_face_mask\horror_mask.mtl",
    "assets\models\monster_face_mask\horror_mask_baseColor.png",
    "assets\models\monster_face_mask\horror_mask_normal.png",
    "assets\models\monster_face_mask\horror_mask_metallicRoughness.png",
    "assets\models\runtime\office_chair_modern.brmesh",
    "assets\models\runtime\office_chair_classic.brmesh",
    "assets\models\runtime\office_chair_task.brmesh",
    "assets\models\runtime\filing_cabinet.brmesh",
    "assets\models\runtime\office_desk.brmesh",
    "assets\models\runtime\trashbin.brmesh",
    "assets\models\runtime\desklamp.brmesh",
    "assets\models\runtime\audio_caset.brmesh",
    "assets\models\runtime\emergency_exit_sign.brmesh",
    "assets\models\runtime\ceiling_lamp_01.brmesh",
    "assets\models\runtime\ceiling_lamp_02.brmesh",
    "assets\models\runtime\ceiling_lamp_03.brmesh",
    "assets\models\runtime\ceiling_lamp_04.brmesh",
    "assets\models\runtime\textures\emergency_exit_sign_diffuse.jpeg",
    "assets\models\runtime\textures\office_chair_modern_diffuse.jpg",
    "assets\models\runtime\textures\office_chair_classic_2209.jpg",
    "assets\models\runtime\textures\office_chair_classic_textiles.png",
    "assets\models\runtime\textures\office_chair_task_diffuse.png"
)

foreach ($relativePath in $runtimeFiles) {
    Copy-RuntimeFile -RelativePath $relativePath
}

$installCmd = @'
@echo off
setlocal EnableExtensions
set "SOURCE=%~dp0"
set "DEST=%LOCALAPPDATA%\BackroomsMazeScreensaver"

if "%LOCALAPPDATA%"=="" (
  echo LOCALAPPDATA is not set. Cannot install for the current user.
  pause
  exit /b 1
)

if not exist "%SOURCE%BackroomsMaze.scr" (
  echo BackroomsMaze.scr was not found next to this installer.
  pause
  exit /b 1
)

if not exist "%DEST%" mkdir "%DEST%"

for %%I in ("%SOURCE%.") do set "SRCFULL=%%~fI"
for %%I in ("%DEST%\.") do set "DESTFULL=%%~fI"

if /I not "%SRCFULL%"=="%DESTFULL%" (
  robocopy "%SOURCE%" "%DEST%" /E /XD Cache /XF BackroomsMaze.profile.log >nul
  if errorlevel 8 (
    echo Failed to copy screensaver files to "%DEST%".
    pause
    exit /b 1
  )
)

set "SCR=%DEST%\BackroomsMaze.scr"
if not exist "%SCR%" (
  echo Installed screensaver was not found at "%SCR%".
  pause
  exit /b 1
)

reg add "HKCU\Control Panel\Desktop" /v SCRNSAVE.EXE /t REG_SZ /d "%SCR%" /f >nul
reg add "HKCU\Control Panel\Desktop" /v ScreenSaveActive /t REG_SZ /d 1 /f >nul

echo Backrooms Maze is installed for the current Windows user.
echo Installed location: "%DEST%"
echo.
echo Opening Windows Screen Saver Settings...
control desk.cpl,,1
pause
'@

$configureCmd = @'
@echo off
setlocal
set "SCR=%~dp0BackroomsMaze.scr"
if not exist "%SCR%" set "SCR=%LOCALAPPDATA%\BackroomsMazeScreensaver\BackroomsMaze.scr"
if not exist "%SCR%" (
  echo BackroomsMaze.scr was not found.
  pause
  exit /b 1
)
start "" "%SCR%" /c
'@

$previewCmd = @'
@echo off
setlocal
set "SCR=%~dp0BackroomsMaze.scr"
if not exist "%SCR%" set "SCR=%LOCALAPPDATA%\BackroomsMazeScreensaver\BackroomsMaze.scr"
if not exist "%SCR%" (
  echo BackroomsMaze.scr was not found.
  pause
  exit /b 1
)
start "" "%SCR%" /s
'@

$debugEffectsCmd = @'
@echo off
setlocal
set "SCR=%~dp0BackroomsMaze.scr"
if not exist "%SCR%" set "SCR=%LOCALAPPDATA%\BackroomsMazeScreensaver\BackroomsMaze.scr"
if not exist "%SCR%" (
  echo BackroomsMaze.scr was not found.
  pause
  exit /b 1
)
start "" "%SCR%" /effectdebug
'@

$uninstallCmd = @'
@echo off
setlocal EnableExtensions
set "DEST=%LOCALAPPDATA%\BackroomsMazeScreensaver"
set "SCR=%DEST%\BackroomsMaze.scr"
set "CURRENT="

for /f "tokens=2,*" %%A in ('reg query "HKCU\Control Panel\Desktop" /v SCRNSAVE.EXE 2^>nul ^| find /I "SCRNSAVE.EXE"') do set "CURRENT=%%B"

if /I "%CURRENT%"=="%SCR%" (
  reg delete "HKCU\Control Panel\Desktop" /v SCRNSAVE.EXE /f >nul 2>nul
  reg add "HKCU\Control Panel\Desktop" /v ScreenSaveActive /t REG_SZ /d 0 /f >nul
  echo Backrooms Maze was removed from the current user's screen saver setting.
) else (
  echo Backrooms Maze is not the current screen saver for this user.
)

if exist "%DEST%" (
  choice /M "Remove installed files from %DEST%"
  if errorlevel 2 exit /b 0
  rmdir /S /Q "%DEST%"
  echo Installed files removed.
)

pause
'@

$readme = @'
Backrooms Maze Screensaver
==========================

Quick install:

1. Extract this ZIP anywhere.
2. Double-click Install-BackroomsMaze.cmd.
3. Choose Backrooms Maze in the Screen Saver Settings window if Windows does not select it automatically.

The installer copies the screensaver to:

%LOCALAPPDATA%\BackroomsMazeScreensaver

No administrator rights are required. The install is for the current Windows user only.

Useful files:

- Install-BackroomsMaze.cmd installs or updates the current user's screensaver.
- Configure-BackroomsMaze.cmd opens the settings window.
- Preview-BackroomsMaze.cmd starts the screensaver immediately.
- Debug-Effects-BackroomsMaze.cmd opens the well-lit 1x1-5x5 effect slice viewer.
- Uninstall-BackroomsMaze.cmd unregisters it and can remove the installed files.
- BackroomsMaze.ini contains the packaged default settings.

Important:

- Keep BackroomsMaze.scr, BackroomsMaze.ini, and the assets folder together.
- If Windows blocks the downloaded files, right-click the ZIP or BackroomsMaze.scr, open Properties, choose Unblock, then apply.

Requirements:

- Windows 10 or Windows 11 with Direct3D 11 support.
'@

Write-AsciiFile -Path (Join-Path $stageDir "Install-BackroomsMaze.cmd") -Text $installCmd
Write-AsciiFile -Path (Join-Path $stageDir "Configure-BackroomsMaze.cmd") -Text $configureCmd
Write-AsciiFile -Path (Join-Path $stageDir "Preview-BackroomsMaze.cmd") -Text $previewCmd
Write-AsciiFile -Path (Join-Path $stageDir "Debug-Effects-BackroomsMaze.cmd") -Text $debugEffectsCmd
Write-AsciiFile -Path (Join-Path $stageDir "Uninstall-BackroomsMaze.cmd") -Text $uninstallCmd
Write-AsciiFile -Path (Join-Path $stageDir "README-install.txt") -Text $readme

$stageTestExe = Join-Path $stageDir "BackroomsMazeTest.exe"
Copy-Item -LiteralPath (Join-Path $stageDir "BackroomsMaze.scr") -Destination $stageTestExe -Force
$stageIni = Join-Path $stageDir "BackroomsMaze.ini"
$cacheLocalAppData = Join-Path $tempDir "cache-localappdata"
$sourceCache = Join-Path $cacheLocalAppData "BackroomsMazeScreensaver\Cache"
New-Item -ItemType Directory -Force -Path $sourceCache | Out-Null
$previousLocalAppData = $env:LOCALAPPDATA
$originalStageIni = [System.IO.File]::ReadAllText($stageIni)
try {
    $env:LOCALAPPDATA = $cacheLocalAppData
    if (-not $SkipSelfTest) {
        Invoke-StageSelfTest -ExePath $stageTestExe -WorkingDirectory $stageDir -FailureMessage "Packaged self-test failed"
    }
    Set-IniValue -Path $stageIni -Section "Monster" -Key "SkullMesh" -Value "assets\models\monster_face_mask\horror_mask.obj"
    Set-IniValue -Path $stageIni -Section "Monster" -Key "AlternateSkullMesh" -Value ""
    Set-IniValue -Path $stageIni -Section "Monster" -Key "SkullMaxTriangles" -Value "16000"
    New-PackagedSkullMesh `
        -StagePath $stageDir `
        -StageExe $stageTestExe `
        -StageIni $stageIni `
        -SourceCache $sourceCache `
        -OutputRelativePath "assets\models\runtime\monster_mask_mesh.bin" `
        -AltChance "0"
    Copy-CacheFiles -SourceCache $sourceCache -StagePath $stageDir
}
finally {
    if ($null -eq $previousLocalAppData) {
        Remove-Item Env:\LOCALAPPDATA -ErrorAction SilentlyContinue
    } else {
        $env:LOCALAPPDATA = $previousLocalAppData
    }
    [System.IO.File]::WriteAllText($stageIni, $originalStageIni, $ascii)
}

Set-IniValue -Path $stageIni -Section "Monster" -Key "SkullMesh" -Value "assets\models\monster_face_mask\horror_mask.obj"
Set-IniValue -Path $stageIni -Section "Monster" -Key "AlternateSkullMesh" -Value ""
Set-IniValue -Path $stageIni -Section "Monster" -Key "AlternateSkullChance" -Value "0"
Set-IniValue -Path $stageIni -Section "Monster" -Key "SkullMaxTriangles" -Value "16000"

foreach ($rawMesh in @(
    "assets\White-Tailed Deer Skull.obj",
    "assets\models\Ram_Skull\Ram_Skull_Scan.OBJ",
    "assets\models\runtime\monster_mask_mesh.bin"
)) {
    $rawMeshPath = Join-Path $stageDir $rawMesh
    if (Test-Path -LiteralPath $rawMeshPath -PathType Leaf) {
        Remove-Item -LiteralPath $rawMeshPath -Force
    }
}

if (-not $SkipSelfTest) {
    Invoke-StageSelfTest -ExePath $stageTestExe -WorkingDirectory $stageDir -FailureMessage "Packaged optimized self-test failed"
}
Remove-Item -LiteralPath $stageTestExe -Force
$profileLog = Join-Path $stageDir "BackroomsMaze.profile.log"
if (Test-Path -LiteralPath $profileLog) {
    Remove-Item -LiteralPath $profileLog -Force
}

$scrHash = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $stageDir "BackroomsMaze.scr")).Hash
$packageFiles = Get-ChildItem -LiteralPath $stageDir -Recurse -File |
    Sort-Object FullName |
    ForEach-Object {
        $_.FullName.Substring($stageDir.Length + 1)
    }
$manifestLines = @(
    "Backrooms Maze Screensaver package",
    "Generated: $((Get-Date).ToString('yyyy-MM-dd HH:mm:ss zzz'))",
    "Configuration: $Configuration",
    "BackroomsMaze.scr SHA256: $scrHash",
    "",
    "Files:"
)
$manifestLines += $packageFiles | ForEach-Object { "- $_" }
$manifest = $manifestLines -join [Environment]::NewLine
Write-AsciiFile -Path (Join-Path $stageDir "PACKAGE-MANIFEST.txt") -Text ($manifest + [Environment]::NewLine)

foreach ($zip in @($zipPath, $legacyZipPath)) {
    Assert-UnderPath -Path $zip -Base $distDir
    if (Test-Path -LiteralPath $zip) {
        Remove-Item -LiteralPath $zip -Force
    }
}

Compress-Archive -Path $stageDir -DestinationPath $zipPath -CompressionLevel Optimal
Copy-Item -LiteralPath $zipPath -Destination $legacyZipPath -Force

Remove-Item -LiteralPath $tempDir -Recurse -Force

$zipItem = Get-Item -LiteralPath $zipPath
Write-Host "Created package: $($zipItem.FullName)"
Write-Host "Package size: $([Math]::Round($zipItem.Length / 1MB, 2)) MB"
Write-Host "SCR SHA256: $scrHash"
