$ErrorActionPreference = 'Stop'

$labRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$privateDirectory = Join-Path $env:LOCALAPPDATA 'MAVD\esp32-c6-2fa-security-lab'
$signingKey = Join-Path $privateDirectory 'secure_boot_signing_key.pem'
$localDefaults = Join-Path $labRoot 'sdkconfig.local'
$env:IDF_TARGET = 'esp32c6'
$env:PYTHONUTF8 = '1'

New-Item -ItemType Directory -Force -Path $privateDirectory | Out-Null

if (-not (Test-Path -LiteralPath $signingKey)) {
    eim run "espsecure generate-signing-key --version 2 --scheme rsa3072 `"$signingKey`"" v6.1
    if ($LASTEXITCODE -ne 0) {
        throw 'No se pudo generar la clave privada de firma.'
    }
}

$keyForConfig = $signingKey.Replace('\', '/')
[System.IO.File]::WriteAllText(
    $localDefaults,
    "CONFIG_SECURE_BOOT_SIGNING_KEY=`"$keyForConfig`"`n",
    [System.Text.UTF8Encoding]::new($false))

$buildStarted = Get-Date
eim run "idf.py -B build-c6-safe -D SDKCONFIG=sdkconfig.safe -D SDKCONFIG_DEFAULTS=`"sdkconfig.defaults;sdkconfig.local`" build" v6.1

$applicationBinary = Join-Path $labRoot 'build-c6-safe/esp32c6_2fa_security_lab.bin'
$bootloaderBinary = Join-Path $labRoot 'build-c6-safe/bootloader/bootloader.bin'
$freshArtifacts = @($applicationBinary, $bootloaderBinary) | Where-Object {
    -not (Test-Path -LiteralPath $_) -or
    (Get-Item -LiteralPath $_).LastWriteTime -lt $buildStarted
}

if ($LASTEXITCODE -ne 0 -or $freshArtifacts.Count -ne 0) {
    throw 'La compilacion del laboratorio fallo.'
}

Write-Host ''
Write-Host 'Compilacion terminada. No se escribio nada en la placa.' -ForegroundColor Green
Write-Host 'No ejecutes idf.py flash con esta imagen en tu placa principal.' -ForegroundColor Yellow
