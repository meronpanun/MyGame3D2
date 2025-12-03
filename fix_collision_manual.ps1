$csvPath = ".\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"
$tempPath = $csvPath + ".tmp"

Write-Host "Processing $csvPath"

if (-not (Test-Path $csvPath)) {
    Write-Error "File not found: $csvPath"
    exit 1
}

$lines = Get-Content $csvPath -Encoding UTF8
$newLines = @()

foreach ($line in $lines) {
    if ([string]::IsNullOrWhiteSpace($line)) {
        continue
    }

    $parts = $line -split ","
    
    # Check if it's a UNIConcrete object (first column)
    # Handle potential quotes if any
    $name = $parts[0].Trim('"')
    
    if ($name -eq "UNIConcrete" -and $parts.Count -ge 10) {
        # Indices 2, 5, 8 are Y coordinates (0-based index)
        # v1y (index 2)
        $y1 = [double]$parts[2]
        if ($y1 -gt 50.0) {
            $parts[2] = "{0:F4}" -f ($y1 + 1000.0)
        }

        # v2y (index 5)
        $y2 = [double]$parts[5]
        if ($y2 -gt 50.0) {
            $parts[5] = "{0:F4}" -f ($y2 + 1000.0)
        }

        # v3y (index 8)
        $y3 = [double]$parts[8]
        if ($y3 -gt 50.0) {
            $parts[8] = "{0:F4}" -f ($y3 + 1000.0)
        }
        
        $newLine = $parts -join ","
        $newLines += $newLine
    } else {
        $newLines += $line
    }
}

$newLines | Set-Content $tempPath -Encoding UTF8

# Force move
Remove-Item $csvPath -Force -ErrorAction SilentlyContinue
Move-Item $tempPath $csvPath -Force
Write-Host "Done."
