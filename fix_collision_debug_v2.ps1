$csvPath = ".\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"
$absPath = Convert-Path $csvPath
Write-Host "Processing $absPath"

$lines = [System.IO.File]::ReadAllLines($absPath, [System.Text.Encoding]::UTF8)

Write-Host "Total lines: $($lines.Count)"

for ($i = 0; $i -lt 50; $i++) {
    $line = $lines[$i]
    if ($line.Contains("UNIConcrete")) {
        Write-Host "Found 'UNIConcrete' at line $i"
        Write-Host "Line content: '$line'"
        
        $parts = $line.Split(',')
        $name = $parts[0].Trim('"')
        Write-Host "Name part: '$name'"
        Write-Host "Name length: $($name.Length)"
        Write-Host "Equals 'UNIConcrete': $($name -eq 'UNIConcrete')"
        
        # Check bytes of name
        $bytes = [System.Text.Encoding]::UTF8.GetBytes($name)
        $hex = $bytes | ForEach-Object { "{0:X2}" -f $_ }
        Write-Host "Name bytes: $($hex -join ' ')"
        
        break
    }
}
