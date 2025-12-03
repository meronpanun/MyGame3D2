$csvPath = ".\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"
$absPath = Convert-Path $csvPath
Write-Host "Processing $absPath"

try {
    $lines = [System.IO.File]::ReadAllLines($absPath, [System.Text.Encoding]::UTF8)
    $newLines = new-object System.Collections.Generic.List[string]
    $modifiedCount = 0

    foreach ($line in $lines) {
        if ($line.Contains("UNIConcrete")) {
            $parts = $line.Split(',')
            $name = $parts[0].Trim('"')
            
            if ($name -eq "UNIConcrete") {
                $y1 = [double]$parts[2]
                if ($y1 -gt 50.0) { $parts[2] = "{0:F4}" -f ($y1 + 1000.0) }
                
                $y2 = [double]$parts[5]
                if ($y2 -gt 50.0) { $parts[5] = "{0:F4}" -f ($y2 + 1000.0) }
                
                $y3 = [double]$parts[8]
                if ($y3 -gt 50.0) { $parts[8] = "{0:F4}" -f ($y3 + 1000.0) }
                
                $newLines.Add([string]::Join(",", $parts))
                $modifiedCount++
            } else {
                $newLines.Add($line)
            }
        } else {
            $newLines.Add($line)
        }
    }

    Write-Host "Modified $modifiedCount lines in memory."
    
    # Write back directly
    [System.IO.File]::WriteAllLines($absPath, $newLines, [System.Text.Encoding]::UTF8)
    Write-Host "Successfully wrote to file."

} catch {
    Write-Error "Error: $_"
    exit 1
}
