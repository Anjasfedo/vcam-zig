$ErrorActionPreference = "Stop"

$path = Join-Path (Resolve-Path ".") "third_party\baseclasses\wxdebug.cpp"

if (!(Test-Path $path)) {
    throw "Cannot find third_party\baseclasses\wxdebug.cpp. Run this from C:\codes\zig\vcam."
}

$s = Get-Content $path -Raw

$old = "        CDisp::CDisp(pp);"
$new = "        CDisp pinDisp(pp);`r`n        (void)StringCchCopy(m_String, NUMELMS(m_String), (LPCTSTR)pinDisp);"

if ($s -notlike "*$old*") {
    Write-Host "Target line was not found. It may already be patched."
} else {
    $s = $s.Replace($old, $new)
    Set-Content -Path $path -Value $s -Encoding ASCII
    Write-Host "Patched wxdebug.cpp CDisp::CDisp(pp) for Clang/Zig."
}
