$utf8NoBom = New-Object System.Text.UTF8Encoding $false
$utf8BOM = New-Object System.Text.UTF8Encoding $true

Get-ChildItem -Path . -Include *.cpp,*.h -Recurse | ForEach-Object {
    try {
        $bytes = [System.IO.File]::ReadAllBytes($_.FullName)
        $text = $utf8NoBom.GetString($bytes)
        [System.IO.File]::WriteAllText($_.FullName, $text, $utf8BOM)
    } catch {
        Write-Error "Failed to process $($_.FullName)"
    }
}
Write-Output "BOM appended successfully."
