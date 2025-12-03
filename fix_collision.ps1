$csvPath = "c:\Users\由留部 瑛人.FICPCROOMB14\Documents\GitHub\MyGame3D2\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"
$tempPath = $csvPath + ".tmp"

Write-Host "Processing $csvPath"

$data = Import-Csv -Path $csvPath -Encoding UTF8

foreach ($row in $data) {
    if ($row.ObjectName -eq "UNIConcrete") {
        # v1y
        $y1 = [double]$row.v1y
        if ($y1 -gt 50.0) {
            $row.v1y = "{0:F4}" -f ($y1 + 1000.0)
        }

        # v2y
        $y2 = [double]$row.v2y
        if ($y2 -gt 50.0) {
            $row.v2y = "{0:F4}" -f ($y2 + 1000.0)
        }

        # v3y
        $y3 = [double]$row.v3y
        if ($y3 -gt 50.0) {
            $row.v3y = "{0:F4}" -f ($y3 + 1000.0)
        }
    }
}

$data | Export-Csv -Path $tempPath -NoTypeInformation -Encoding UTF8

# Remove quotes from numeric values if Export-Csv added them (optional, but cleaner)
# Actually Export-Csv adds quotes to everything. The game parser might handle it or might not.
# The original file didn't have quotes.
# Let's try to remove quotes.
(Get-Content $tempPath) | ForEach-Object { $_ -replace '"', "" } | Set-Content $tempPath

Move-Item -Path $tempPath -Destination $csvPath -Force
Write-Host "Done."
