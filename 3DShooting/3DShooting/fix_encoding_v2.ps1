$utf8BOM = New-Object System.Text.UTF8Encoding $true
# Get-ChildItem to find files regardless of Japanese path issues in the script itself
$targets = @("SceneMain.cpp", "PlayerUI.cpp", "TutorialManager.cpp", "TaskTutorialManager.cpp")

Get-ChildItem -Path . -Recurse | Where-Object { $targets -contains $_.Name } | ForEach-Object {
    try {
        $path = $_.FullName
        $text = [System.IO.File]::ReadAllText($path)
        [System.IO.File]::WriteAllText($path, $text, $utf8BOM)
        Write-Output "Fixed: $($_.Name) at $path"
    } catch {
        Write-Error "Failed: $($_.Name) - $($_.Exception.Message)"
    }
}
