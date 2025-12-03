$csvPath = ".\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"
$absPath = Convert-Path $csvPath
Write-Host "Processing $absPath"

$lines = [System.IO.File]::ReadAllLines($absPath, [System.Text.Encoding]::UTF8)
$newLines = new-object System.Collections.Generic.List[string]

$count = 0
foreach ($line in $lines) {
    if ($line.Contains("UNIConcrete")) {
        $parts = $line.Split(',')
        # Check if first column is UNIConcrete (ignoring quotes)
        $name = $parts[0].Trim('"')
        
        if ($name -eq "UNIConcrete" -and $parts.Length -ge 10) {
            # v1y (index 2)
            $y1 = [double]$parts[2]
            if ($y1 -gt 50.0) { $parts[2] = "{0:F4}" -f ($y1 + 1000.0) }

            # v2y (index 5)
            $y2 = [double]$parts[5]
            if ($y2 -gt 50.0) { $parts[5] = "{0:F4}" -f ($y2 + 1000.0) }

            # v3y (index 8)
            $y3 = [double]$parts[8]
            if ($y3 -gt 50.0) { $parts[8] = "{0:F4}" -f ($y3 + 1000.0) }
            
            $newLine = [string]::Join(",", $parts)
            $newLines.Add($newLine)
            $count++
        } else {
            $newLines.Add($line)
        }
    } else {
        $newLines.Add($line)
    }
}

Write-Host "Modified $count lines."
[System.IO.File]::WriteAllLines($absPath, $newLines, [System.Text.Encoding]::UTF8)
Write-Host "Done."
