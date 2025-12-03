$csvPath = ".\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"
$newCsvPath = ".\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData_New.csv"

Write-Host "Reading $csvPath"

$lines = Get-Content $csvPath -Encoding UTF8
$newLines = new-object System.Collections.Generic.List[string]
$modifiedCount = 0

foreach ($line in $lines) {
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
            
            $newLines.Add(($parts -join ","))
            $modifiedCount++
        } else {
            $newLines.Add($line)
        }
    } else {
        $newLines.Add($line)
    }
}

Write-Host "Modified $modifiedCount lines."
[System.IO.File]::WriteAllLines((Convert-Path $newCsvPath -ErrorAction SilentlyContinue | Select-Object -First 1 -ErrorAction SilentlyContinue) -or $newCsvPath, $newLines, [System.Text.Encoding]::UTF8)
Write-Host "Created $newCsvPath"
