$ErrorActionPreference = "Stop"

$path = Join-Path (Resolve-Path ".") "src\VirtualCam.cpp"

if (!(Test-Path $path)) {
    throw "Cannot find src\VirtualCam.cpp. Run this from C:\codes\zig\vcam."
}

$s = Get-Content $path -Raw

# Fix accidental literal `r`n inserted by previous patch.
$s = $s.Replace(
    "*ppmt = CreateMediaType(&mt);``r``n    return *ppmt ? S_OK : E_OUTOFMEMORY;",
    "*ppmt = CreateMediaType(&mt);`r`n    return *ppmt ? S_OK : E_OUTOFMEMORY;"
)

$s = $s.Replace(
    "*ppmt = CreateMediaType(&mt);``r``n    hr = *ppmt ? S_OK : E_OUTOFMEMORY;",
    "*ppmt = CreateMediaType(&mt);`r`n    hr = *ppmt ? S_OK : E_OUTOFMEMORY;"
)

# Also handle the exact broken text if it appears with single backtick escapes as raw characters.
$s = $s.Replace(
    "*ppmt = CreateMediaType(&mt);`r`n    return *ppmt ? S_OK : E_OUTOFMEMORY;",
    "*ppmt = CreateMediaType(&mt);`r`n    return *ppmt ? S_OK : E_OUTOFMEMORY;"
)

$s = $s.Replace(
    "*ppmt = CreateMediaType(&mt);`r`n    hr = *ppmt ? S_OK : E_OUTOFMEMORY;",
    "*ppmt = CreateMediaType(&mt);`r`n    hr = *ppmt ? S_OK : E_OUTOFMEMORY;"
)

# Direct line-level fallback: replace any line containing the bad token.
$lines = $s -split "`r?`n"
$out = New-Object System.Collections.Generic.List[string]

foreach ($line in $lines) {
    if ($line -like "*CreateMediaType(&mt);``r``n*return *ppmt ? S_OK : E_OUTOFMEMORY;*") {
        $out.Add("    *ppmt = CreateMediaType(&mt);")
        $out.Add("    return *ppmt ? S_OK : E_OUTOFMEMORY;")
    }
    elseif ($line -like "*CreateMediaType(&mt);``r``n*hr = *ppmt ? S_OK : E_OUTOFMEMORY;*") {
        $out.Add("    *ppmt = CreateMediaType(&mt);")
        $out.Add("    hr = *ppmt ? S_OK : E_OUTOFMEMORY;")
    }
    else {
        $out.Add($line)
    }
}

$newText = [string]::Join("`r`n", $out)
Set-Content -Path $path -Value $newText -Encoding ASCII

Write-Host "Patched src\VirtualCam.cpp bad literal newline text."
