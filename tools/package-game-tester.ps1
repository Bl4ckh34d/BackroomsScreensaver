param(
    [string]$Configuration = "Release",
    [string]$BuildPath = "build",
    [switch]$SkipBuild,
    [switch]$SkipSelfTest
)

$ErrorActionPreference = "Stop"

$ascii = [System.Text.ASCIIEncoding]::new()
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$buildDir = if ([System.IO.Path]::IsPathRooted($BuildPath)) {
    [System.IO.Path]::GetFullPath($BuildPath)
} else {
    [System.IO.Path]::GetFullPath((Join-Path $root $BuildPath))
}
$distDir = Join-Path $root "dist"
$stageDir = Join-Path $distDir "BackroomsMazeGameTester"
$tempDir = Join-Path $distDir "_game-package-tmp"
$zipPath = Join-Path $distDir "BackroomsMazeGameTester-latest.zip"

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

function Copy-RuntimeFile {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath
    )

    $source = Join-Path $root $RelativePath
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
        throw "Required runtime file is missing: $RelativePath"
    }

    $destination = Join-Path $stageDir $RelativePath
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destination) | Out-Null
    Copy-Item -LiteralPath $source -Destination $destination -Force
}

function Copy-RuntimeDirectory {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath
    )

    $source = Join-Path $root $RelativePath
    if (-not (Test-Path -LiteralPath $source -PathType Container)) {
        throw "Required runtime directory is missing: $RelativePath"
    }

    $destinationParent = Join-Path $stageDir (Split-Path -Parent $RelativePath)
    New-Item -ItemType Directory -Force -Path $destinationParent | Out-Null
    Copy-Item -LiteralPath $source -Destination $destinationParent -Recurse -Force
}

function Copy-CacheFiles {
    param(
        [Parameter(Mandatory = $true)][string]$SourceCache,
        [Parameter(Mandatory = $true)][string]$StagePath
    )

    if (-not (Test-Path -LiteralPath $SourceCache -PathType Container)) {
        return
    }

    $shaderCache = Join-Path $StagePath "ShaderCache"
    $textureCache = Join-Path $StagePath "TextureCache"
    New-Item -ItemType Directory -Force -Path $shaderCache | Out-Null
    New-Item -ItemType Directory -Force -Path $textureCache | Out-Null

    Get-ChildItem -LiteralPath $SourceCache -Filter "BackroomsMaze_*.cso" -File |
        Copy-Item -Destination $shaderCache -Force
    Get-ChildItem -LiteralPath $SourceCache -Filter "BackroomsMazeGame_textures.bin" -File |
        Copy-Item -Destination $textureCache -Force
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
$gameExe = Join-Path $releaseDir "BackroomsMazeGame.exe"
$gameIni = Join-Path $releaseDir "BackroomsMaze.ini"
if (-not (Test-Path -LiteralPath $gameExe -PathType Leaf)) {
    throw "Built game executable was not found: $gameExe"
}
if (-not (Test-Path -LiteralPath $gameIni -PathType Leaf)) {
    throw "Built game INI was not found: $gameIni"
}

New-Item -ItemType Directory -Force -Path $distDir | Out-Null
Assert-UnderPath -Path $stageDir -Base $distDir
Assert-UnderPath -Path $tempDir -Base $distDir
Assert-UnderPath -Path $zipPath -Base $distDir

if (Test-Path -LiteralPath $stageDir) {
    Remove-Item -LiteralPath $stageDir -Recurse -Force
}
if (Test-Path -LiteralPath $tempDir) {
    Remove-Item -LiteralPath $tempDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $stageDir | Out-Null
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null

Copy-Item -LiteralPath $gameExe -Destination (Join-Path $stageDir "BackroomsMazeGame.exe") -Force
Copy-Item -LiteralPath $gameIni -Destination (Join-Path $stageDir "BackroomsMaze.ini") -Force

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
    "assets\images\title.png",
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

Copy-RuntimeDirectory -RelativePath "assets\images\randomPages"
Copy-RuntimeDirectory -RelativePath "assets\sounds"

$runCmd = @'
@echo off
setlocal
start "" "%~dp0BackroomsMazeGame.exe"
'@

$readme = @'
Backrooms Maze Game - Tester Build
==================================

How to play:
1. Extract this ZIP to a folder.
2. Double-click BackroomsMazeGame.exe or Run-BackroomsMazeGame.cmd.

Keep BackroomsMazeGame.exe, BackroomsMaze.ini, and the assets folder together.
This package includes the curated runtime textures, models, page-image library, branding, and sounds used by the current Release build.

If Windows blocks the download, right-click the ZIP or EXE, open Properties, choose Unblock, then Apply.

Requirements:
- Windows 10 or Windows 11
- Direct3D 11-capable GPU
'@

Write-AsciiFile -Path (Join-Path $stageDir "Run-BackroomsMazeGame.cmd") -Text $runCmd
Write-AsciiFile -Path (Join-Path $stageDir "README-TESTER.txt") -Text $readme

if (-not $SkipSelfTest) {
    $stageExe = Join-Path $stageDir "BackroomsMazeGame.exe"
    $cacheLocalAppData = Join-Path $tempDir "cache-localappdata"
    $sourceCache = Join-Path $cacheLocalAppData "BackroomsMazeScreensaver\Cache"
    New-Item -ItemType Directory -Force -Path $sourceCache | Out-Null
    $previousLocalAppData = $env:LOCALAPPDATA
    try {
        $env:LOCALAPPDATA = $cacheLocalAppData
        $lastExitCode = 0
        for ($attempt = 1; $attempt -le 2; ++$attempt) {
            $selfTest = Start-Process -FilePath $stageExe -ArgumentList "/selftest" -WorkingDirectory $stageDir -PassThru
            if (-not $selfTest.WaitForExit(120000)) {
                try { Stop-Process -Id $selfTest.Id -Force -ErrorAction SilentlyContinue } catch {}
                throw "Packaged game self-test timed out"
            }
            $lastExitCode = $selfTest.ExitCode
            if ($lastExitCode -eq 0) {
                break
            }
            if ($attempt -lt 2) {
                Write-Warning "Packaged game self-test failed with exit code $lastExitCode; retrying once."
                Start-Sleep -Seconds 2
            }
        }
        if ($lastExitCode -ne 0) {
            throw "Packaged game self-test failed with exit code $lastExitCode"
        }
        Copy-CacheFiles -SourceCache $sourceCache -StagePath $stageDir
    }
    finally {
        if ($null -eq $previousLocalAppData) {
            Remove-Item Env:\LOCALAPPDATA -ErrorAction SilentlyContinue
        } else {
            $env:LOCALAPPDATA = $previousLocalAppData
        }
    }
} else {
    $defaultCache = Join-Path $env:LOCALAPPDATA "BackroomsMazeScreensaver\Cache"
    Copy-CacheFiles -SourceCache $defaultCache -StagePath $stageDir
}

$exeHash = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $stageDir "BackroomsMazeGame.exe")).Hash
$packageFiles = Get-ChildItem -LiteralPath $stageDir -Recurse -File |
    Sort-Object FullName |
    ForEach-Object {
        $_.FullName.Substring($stageDir.Length + 1)
    }
$manifestLines = @(
    "Backrooms Maze Game tester package",
    "Generated: $((Get-Date).ToString('yyyy-MM-dd HH:mm:ss zzz'))",
    "Configuration: $Configuration",
    "BackroomsMazeGame.exe SHA256: $exeHash",
    "",
    "Files:"
)
$manifestLines += $packageFiles | ForEach-Object { "- $_" }
Write-AsciiFile -Path (Join-Path $stageDir "PACKAGE-MANIFEST.txt") -Text (($manifestLines -join [Environment]::NewLine) + [Environment]::NewLine)

if (Test-Path -LiteralPath $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}
Compress-Archive -LiteralPath $stageDir -DestinationPath $zipPath -CompressionLevel Optimal

$zipItem = Get-Item -LiteralPath $zipPath
$zipHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $zipPath).Hash
Write-Host "Created package: $($zipItem.FullName)"
Write-Host "Package size: $([Math]::Round($zipItem.Length / 1MB, 2)) MB"
Write-Host "ZIP SHA256: $zipHash"

Remove-Item -LiteralPath $tempDir -Recurse -Force
