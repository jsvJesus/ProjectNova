$ProjectRoot = "Z:\Epic\Games\ProjectNova"
$OutFile = Join-Path $ProjectRoot "ProjectNova_FileIndex.txt"

$IncludeExt = @(
    ".h",
    ".cpp",
    ".cs",
    ".uproject",
    ".ini",
    ".md",
    ".txt"
)

$SkipDirs = @(
    "\Binaries\",
    "\Intermediate\",
    "\Saved\",
    "\DerivedDataCache\",
    "\.vs\",
    "\Content\",
    "\Plugins\"
)

$OutFileFull = [System.IO.Path]::GetFullPath($OutFile)
$ScriptFileFull = [System.IO.Path]::GetFullPath($PSCommandPath)

$Files = Get-ChildItem $ProjectRoot -Recurse -File | Where-Object {
    $FileFull = [System.IO.Path]::GetFullPath($_.FullName)
    $Ext = $_.Extension

    if ($FileFull -eq $OutFileFull) {
        return $false
    }

    if ($FileFull -eq $ScriptFileFull) {
        return $false
    }

    if ($IncludeExt -notcontains $Ext) {
        return $false
    }

    foreach ($Dir in $SkipDirs) {
        if ($FileFull.Contains($Dir)) {
            return $false
        }
    }

    return $true
} | Sort-Object FullName

if (Test-Path $OutFile) {
    Remove-Item $OutFile -Force
}

$Encoding = New-Object System.Text.UTF8Encoding($false)
$Writer = New-Object System.IO.StreamWriter($OutFile, $false, $Encoding)

try {
    $Writer.WriteLine("PROJECTNOVA FILE INDEX")
    $Writer.WriteLine("Generated: $(Get-Date)")
    $Writer.WriteLine("Root: $ProjectRoot")
    $Writer.WriteLine("Files: $($Files.Count)")
    $Writer.WriteLine("")
    $Writer.WriteLine("================ FILES ================")
    $Writer.WriteLine("")

    foreach ($File in $Files) {
        $Relative = $File.FullName.Replace($ProjectRoot + "\", "")
        $SizeKB = [Math]::Round($File.Length / 1KB, 2)
        $Modified = $File.LastWriteTime.ToString("yyyy-MM-dd HH:mm:ss")

        $Writer.WriteLine("$Relative | Size: ${SizeKB} KB | Modified: $Modified")
    }
}
finally {
    $Writer.Close()
}

Write-Host "DONE: $OutFile"
Write-Host "FILES: $($Files.Count)"