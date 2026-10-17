$ErrorActionPreference = 'Stop'

Write-Host 'Conversor local de secreto TOTP Base32 a bytes hexadecimales'
Write-Host 'La clave no se guarda ni se envía por Internet.'
$secureSecret = Read-Host 'Pega la clave Base32' -AsSecureString
$secretPointer = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secureSecret)
try {
    $secret = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($secretPointer)
} finally {
    [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($secretPointer)
}
$normalized = ($secret.ToUpperInvariant() -replace '[\s\-=]', '')

if ([string]::IsNullOrWhiteSpace($normalized)) {
    throw 'La clave está vacía.'
}

$alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567'
$buffer = 0
$bits = 0
$bytes = [System.Collections.Generic.List[byte]]::new()

foreach ($character in $normalized.ToCharArray()) {
    $value = $alphabet.IndexOf($character)
    if ($value -lt 0) {
        throw "Carácter Base32 no válido: $character"
    }

    $buffer = ($buffer -shl 5) -bor $value
    $bits += 5
    while ($bits -ge 8) {
        $bits -= 8
        $bytes.Add([byte](($buffer -shr $bits) -band 0xFF))
        if ($bits -eq 0) {
            $buffer = 0
        } else {
            $buffer = $buffer -band ((1 -shl $bits) - 1)
        }
    }
}

if ($bytes.Count -eq 0) {
    throw 'La clave no produjo ningún byte.'
}

if ($bytes.Count -lt 20) {
    throw 'La clave debe producir al menos 20 bytes (160 bits).'
}

$hex = ($bytes | ForEach-Object { '0x{0:X2}' -f $_ }) -join ', '
Write-Host ''
Write-Host 'Copia esta línea en secrets.h:' -ForegroundColor Cyan
Write-Host "#define TOTP_HMAC_KEY {$hex}" -ForegroundColor Green
Write-Host ''
Write-Host 'Cierra esta ventana y no guardes la salida en servicios públicos.'
