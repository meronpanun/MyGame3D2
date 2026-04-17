$utf8BOM = New-Object System.Text.UTF8Encoding $true

Get-ChildItem -Path . -Include *.cpp,*.h -Recurse | ForEach-Object {
    try {
        # Read the text. Powershell removes the leading BOM automatically.
        $text = [System.IO.File]::ReadAllText($_.FullName)
        
        # Remove any lingering BOM characters (U+FEFF) that got embedded into the string.
        $text = $text.Replace([char]0xFEFF, "")
        
        # Write it back out with exactly one BOM.
        [System.IO.File]::WriteAllText($_.FullName, $text, $utf8BOM)
    } catch {
        Write-Error "Failed to process $($_.FullName)"
    }
}
Write-Output "Double BOMs cleaned up."
