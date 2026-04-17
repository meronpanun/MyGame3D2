Get-ChildItem -Path . -Include *.h,*.cpp -Recurse | ForEach-Object {
    $FilePath = $_.FullName
    try {
        $Bytes = [System.IO.File]::ReadAllBytes($FilePath)
        $modified = $false
        
        # Check and remove all extra BOMs at the start.
        # Loop to remove multiple BOMs if they somehow got stacked more than twice.
        while ($Bytes.Length -ge 6 -and $Bytes[0] -eq 0xEF -and $Bytes[1] -eq 0xBB -and $Bytes[2] -eq 0xBF -and $Bytes[3] -eq 0xEF -and $Bytes[4] -eq 0xBB -and $Bytes[5] -eq 0xBF) {
            $NewBytes = New-Object byte[] ($Bytes.Length - 3)
            [System.Array]::Copy($Bytes, 3, $NewBytes, 0, $NewBytes.Length)
            $Bytes = $NewBytes
            $modified = $true
        }
        
        if ($modified) {
            [System.IO.File]::WriteAllBytes($FilePath, $Bytes)
            Write-Output "Fixed double BOM in $($_.Name)"
        }
    } catch {
        Write-Error "Error on $($FilePath)"
    }
}
Write-Output "Fix script complete."
