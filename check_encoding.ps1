$csvPath = ".\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"
$bytes = Get-Content $csvPath -Encoding Byte -TotalCount 100
$hex = $bytes | ForEach-Object { "{0:X2}" -f $_ }
Write-Host "First 100 bytes:"
Write-Host ($hex -join " ")
