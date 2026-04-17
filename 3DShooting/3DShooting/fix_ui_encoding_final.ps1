$utf8BOM = New-Object System.Text.UTF8Encoding $true
$filePaths = @(
    "c:\Users\sho24\OneDrive\ドキュメント\GitHub\MyGame3D2\3DShooting\3DShooting\Scene\SceneMain.cpp",
    "c:\Users\sho24\OneDrive\ドキュメント\GitHub\MyGame3D2\3DShooting\3DShooting\GameObject\PlayerUI.cpp",
    "c:\Users\sho24\OneDrive\ドキュメント\GitHub\MyGame3D2\3DShooting\3DShooting\TutorialManager.cpp",
    "c:\Users\sho24\OneDrive\ドキュメント\GitHub\MyGame3D2\3DShooting\3DShooting\TaskTutorialManager.cpp"
)

foreach ($f in $filePaths) {
    if (Test-Path $f) {
        try {
            # Read the file content
            $content = [System.IO.File]::ReadAllText($f)
            # Write it back with UTF-8 BOM
            [System.IO.File]::WriteAllText($f, $content, $utf8BOM)
            Write-Output "Successfully fixed encoding for: $f"
        } catch {
            Write-Error "Failed to fix encoding for $f : $($_.Exception.Message)"
        }
    } else {
        Write-Warning "File not found: $f"
    }
}
