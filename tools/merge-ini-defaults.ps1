param(
    [string]$ScrPath,
    [string]$GeneratedIni,
    [Parameter(Mandatory = $true)][string]$PreferredIni,
    [Parameter(Mandatory = $true)][string]$OutputIni,
    [switch]$PackagePaths
)

$ErrorActionPreference = "Stop"
$utf8NoBom = [System.Text.UTF8Encoding]::new($false)

function Get-IniValues {
    param([Parameter(Mandatory = $true)][string]$Path)

    $values = @{}
    $section = ""
    foreach ($line in [System.IO.File]::ReadLines($Path)) {
        $trimmed = $line.Trim()
        if ($trimmed.Length -eq 0 -or $trimmed.StartsWith(";") -or $trimmed.StartsWith("#")) {
            continue
        }
        if ($trimmed.StartsWith("[") -and $trimmed.EndsWith("]")) {
            $section = $trimmed.Substring(1, $trimmed.Length - 2)
            continue
        }
        $equals = $line.IndexOf("=")
        if ($equals -lt 0 -or $section.Length -eq 0) {
            continue
        }
        $key = $line.Substring(0, $equals).Trim()
        $value = $line.Substring($equals + 1)
        $values["$section`0$key"] = $value
    }
    return $values
}

function New-GeneratedIniFromScr {
    param([Parameter(Mandatory = $true)][string]$Path)

    $tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("BackroomsMazeIniGen-" + [System.Guid]::NewGuid().ToString("N"))
    New-Item -ItemType Directory -Force -Path $tempRoot | Out-Null
    try {
        $testExe = Join-Path $tempRoot "BackroomsMazeTest.exe"
        Copy-Item -LiteralPath $Path -Destination $testExe -Force
        $proc = Start-Process -FilePath $testExe -ArgumentList "/makeini" -WorkingDirectory $tempRoot -Wait -PassThru
        if ($proc.ExitCode -ne 0) {
            throw "Default INI generation failed with exit code $($proc.ExitCode)"
        }
        $generated = Join-Path $tempRoot "BackroomsMaze.ini"
        if (-not (Test-Path -LiteralPath $generated -PathType Leaf)) {
            throw "Default INI was not generated: $generated"
        }
        return @{
            TempRoot = $tempRoot
            Ini = $generated
        }
    } catch {
        if (Test-Path -LiteralPath $tempRoot) {
            Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
        }
        throw
    }
}

if (-not $GeneratedIni) {
    if (-not $ScrPath) {
        throw "Either -GeneratedIni or -ScrPath is required."
    }
    $generatedState = New-GeneratedIniFromScr -Path $ScrPath
    $GeneratedIni = $generatedState.Ini
}

try {
    if (-not (Test-Path -LiteralPath $GeneratedIni -PathType Leaf)) {
        throw "Generated INI not found: $GeneratedIni"
    }
    if (-not (Test-Path -LiteralPath $PreferredIni -PathType Leaf)) {
        throw "Preferred INI not found: $PreferredIni"
    }

    $preferredValues = Get-IniValues -Path $PreferredIni
    $section = ""
    $outputLines = New-Object System.Collections.Generic.List[string]

    foreach ($line in [System.IO.File]::ReadLines($GeneratedIni)) {
        $trimmed = $line.Trim()
        if ($trimmed.StartsWith("[") -and $trimmed.EndsWith("]")) {
            $section = $trimmed.Substring(1, $trimmed.Length - 2)
            $outputLines.Add($line)
            continue
        }

        $equals = $line.IndexOf("=")
        if ($equals -ge 0 -and $section.Length -gt 0) {
            $key = $line.Substring(0, $equals).Trim()
            $lookup = "$section`0$key"
            if ($preferredValues.ContainsKey($lookup)) {
                $outputLines.Add("$key=$($preferredValues[$lookup])")
                continue
            }
        }
        $outputLines.Add($line)
    }

    $text = [string]::Join("`r`n", $outputLines) + "`r`n"
    if ($PackagePaths) {
        $text = $text.Replace("AssetFolder=..\..\assets\PBRs", "AssetFolder=assets\PBRs")
        $text = $text.Replace("SkullMesh=..\..\assets\White-Tailed Deer Skull.obj", "SkullMesh=assets\White-Tailed Deer Skull.obj")
    }

    $parent = Split-Path -Parent $OutputIni
    if ($parent) {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }
    [System.IO.File]::WriteAllText($OutputIni, $text, $utf8NoBom)
} finally {
    if ($generatedState -and $generatedState.TempRoot -and (Test-Path -LiteralPath $generatedState.TempRoot)) {
        Remove-Item -LiteralPath $generatedState.TempRoot -Recurse -Force -ErrorAction SilentlyContinue
    }
}
