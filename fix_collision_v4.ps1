$csvPath = ".\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"
$tempPath = $csvPath + ".tmp"

Write-Host "Processing $csvPath"

Get-Content $csvPath -Encoding UTF8 | ForEach-Object {
    $line = $_
    if ($line -match "UNIConcrete") {
        $parts = $line -split ","
        $name = $parts[0].Trim('"')
        
        if ($name -eq "UNIConcrete") {
            # v1y (index 2)
            $y1 = [double]$parts[2]
            if ($y1 -gt 50.0) { $parts[2] = "{0:F4}" -f ($y1 + 1000.0) }

            # v2y (index 5)
            $y2 = [double]$parts[5]
            if ($y2 -gt 50.0) { $parts[5] = "{0:F4}" -f ($y2 + 1000.0) }

            # v3y (index 8)
            $y3 = [double]$parts[8]
            if ($y3 -gt 50.0) { $parts[8] = "{0:F4}" -f ($y3 + 1000.0) }
            
            $parts -join ","
        } else {
            $line
        }
    } else {
        $line
    }
} | Set-Content $tempPath -Encoding UTF8

Remove-Item $csvPath -Force
Move-Item $tempPath $csvPath -Force
Write-Host "Done."
