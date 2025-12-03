Write-Host "Current Directory: $(Get-Location)"
$csvPath = ".\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"

if (-not (Test-Path $csvPath)) {
    Write-Error "File not found: $csvPath"
    exit 1
}

Get-Content $csvPath -Encoding UTF8 | Select-Object -First 50 | ForEach-Object {
    $line = $_
    if ($line -match "UNIConcrete") {
        Write-Host "Found 'UNIConcrete'"
        Write-Host "Line: '$line'"
        
        $parts = $line -split ","
        $name = $parts[0].Trim('"')
        Write-Host "Name: '$name'"
        Write-Host "Name Length: $($name.Length)"
        
        $bytes = [System.Text.Encoding]::UTF8.GetBytes($name)
        $hex = $bytes | ForEach-Object { "{0:X2}" -f $_ }
        Write-Host "Bytes: $($hex -join ' ')"
        
        # Check Y value
        $y1 = $parts[2]
        Write-Host "Y1 string: '$y1'"
        $y1val = [double]$y1
        Write-Host "Y1 value: $y1val"
        Write-Host "Y1 > 50: $($y1val -gt 50.0)"
    }
}
