param(
    [string]$ObsPath = "C:\Program Files\obs-studio"
)

$marker = Join-Path $ObsPath "data\obs-plugins\kori\version.txt"
if (Test-Path -LiteralPath $marker) {
    $version = (Get-Content -LiteralPath $marker -Raw).Trim()
    if ($version) {
        Write-Output $version
        exit 0
    }
}

$dll = Join-Path $ObsPath "obs-plugins\64bit\kori.dll"
if (-not (Test-Path -LiteralPath $dll)) {
    Write-Output "Not installed"
    exit 0
}

# Older Kori builds did not install a version marker. Their compiled module
# contains the semantic version used by the OBS load log, so recover it once
# during the upgrade to the marker-based installer.
$text = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($dll))
$versions = @([regex]::Matches(
    $text,
    '(?<![0-9])0\.[0-9]{1,3}\.[0-9]{1,3}(?![0-9])'
) | ForEach-Object { $_.Value } | Sort-Object -Unique)

if ($versions.Count -eq 1) {
    Write-Output $versions[0]
}
else {
    Write-Output "Unknown"
}
