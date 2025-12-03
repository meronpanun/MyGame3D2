$csvPath = "c:\Users\由留部 瑛人.FICPCROOMB14\Documents\GitHub\MyGame3D2\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv"
Write-Host "Debugging $csvPath"

$data = Import-Csv -Path $csvPath -Encoding UTF8
if ($data.Count -gt 0) {
    $firstRow = $data[0]
    Write-Host "First Row Properties:"
    $firstRow.PSObject.Properties | ForEach-Object {
        Write-Host "  Name: '$($_.Name)', Value: '$($_.Value)'"
    }
    
    Write-Host "Checking ObjectName match:"
    if ($firstRow.ObjectName -eq "UNIConcrete") {
        Write-Host "  Match confirmed."
    } else {
        Write-Host "  Match FAILED. Expected 'UNIConcrete', got '$($firstRow.ObjectName)'"
    }
} else {
    Write-Host "CSV is empty."
}
