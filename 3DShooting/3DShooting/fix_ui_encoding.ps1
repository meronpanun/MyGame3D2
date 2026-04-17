$utf8BOM = New-Object System.Text.UTF8Encoding $true
$files = @(
    "Scene/SceneMain.cpp",
    "GameObject/PlayerUI.cpp",
    "TutorialManager.cpp",
    "TaskTutorialManager.cpp"
)

foreach ($f in $files) {
    if (Test-Path $f) {
        try {
            # ReadAllText は自動的にBOMを認識し、文字列からは除外します
            $fullPath = Resolve-Path $f
            $text = [System.IO.File]::ReadAllText($fullPath)
            
            # UTF-8 with BOM で保存
            [System.IO.File]::WriteAllText($fullPath, $text, $utf8BOM)
            Write-Output "Successfully re-encoded: $f"
        } catch {
            Write-Error "Failed to re-encode: $f - $($_.Exception.Message)"
        }
    } else {
        Write-Warning "File not found: $f"
    }
}
