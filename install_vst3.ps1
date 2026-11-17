# Ejecutar en PowerShell COMO ADMINISTRADOR, desde esta carpeta, despues de compilar.
$src = Get-ChildItem -Path "build" -Recurse -Directory -Filter "*.vst3" | Select-Object -First 1
if (-not $src) { Write-Host "No encontre ningun .vst3 en build/. Compilo bien?"; exit 1 }
$dst = "C:\Program Files\Common Files\VST3"
Copy-Item $src.FullName -Destination $dst -Recurse -Force
Write-Host "Copiado: $($src.Name) -> $dst"
Write-Host "Ahora en FL Studio: Options > Manage plugins > Find installed plugins"
