$csvPath = ".\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"
$tempPath = $csvPath + ".tmp"

Write-Host "Processing $csvPath with Regex"

if (-not (Test-Path $csvPath)) {
    Write-Error "File not found: $csvPath"
    exit 1
}

$content = Get-Content $csvPath -Raw -Encoding UTF8

# Regex to match UNIConcrete lines
# Matches: UNIConcrete, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z
# Captures: 1=v1x, 2=v1y, 3=v1z, 4=v2x, 5=v2y, 6=v2z, 7=v3x, 8=v3y, 9=v3z
$pattern = '(?m)^"?UNIConcrete"?,([^,\r\n]+),([^,\r\n]+),([^,\r\n]+),([^,\r\n]+),([^,\r\n]+),([^,\r\n]+),([^,\r\n]+),([^,\r\n]+),([^,\r\n]+)'

$newContent = [Regex]::Replace($content, $pattern, { param($match)
    $v1x = $match.Groups[1].Value
    $v1y = [double]$match.Groups[2].Value
    $v1z = $match.Groups[3].Value
    
    $v2x = $match.Groups[4].Value
    $v2y = [double]$match.Groups[5].Value
    $v2z = $match.Groups[6].Value
    
    $v3x = $match.Groups[7].Value
    $v3y = [double]$match.Groups[8].Value
    $v3z = $match.Groups[9].Value
    
    if ($v1y -gt 50.0) { $v1y += 1000.0 }
    if ($v2y -gt 50.0) { $v2y += 1000.0 }
    if ($v3y -gt 50.0) { $v3y += 1000.0 }
    
    # Reconstruct the line
    return "UNIConcrete,$v1x,{0:F4},$v1z,$v2x,{1:F4},$v2z,$v3x,{2:F4},$v3z" -f $v1y, $v2y, $v3y
})

$newContent | Set-Content $tempPath -Encoding UTF8 -NoNewline

Remove-Item $csvPath -Force -ErrorAction SilentlyContinue
Move-Item $tempPath $csvPath -Force
Write-Host "Done."
