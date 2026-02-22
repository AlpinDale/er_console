$ErrorActionPreference = "Stop"

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptRoot "..")

$extensions = @(
  "*.c",
  "*.cc",
  "*.cpp",
  "*.cxx",
  "*.h",
  "*.hh",
  "*.hpp",
  "*.hxx"
)

$srcRoot = Resolve-Path (Join-Path $repoRoot "src")
$files = Get-ChildItem -Path $srcRoot -Recurse -File -Include $extensions

if (-not $files -or $files.Count -eq 0) {
  Write-Host "No C/C++ files found."
  exit 0
}

$changed = @()

foreach ($file in $files) {
  $before = (Get-FileHash -Algorithm SHA256 -Path $file.FullName).Hash
  & clang-format -i $file.FullName
  if ($LASTEXITCODE -ne 0) {
    throw "clang-format failed on $($file.FullName)"
  }
  $after = (Get-FileHash -Algorithm SHA256 -Path $file.FullName).Hash
  if ($before -ne $after) {
    $changed += $file.FullName
  }
}

if ($changed.Count -eq 0) {
  Write-Host "No formatting changes."
  exit 0
}

Write-Host "Formatted $($changed.Count) file(s):"
foreach ($path in $changed) {
  Write-Host $path
}
