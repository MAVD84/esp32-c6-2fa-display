# Seguridad y puesta en marcha

## Antes de compilar

- `secrets.h` contiene datos privados y está excluido de Git. Solo se incluye en la
  carga temporal de importación. Con `ALLOW_BUILD_SECRET_IMPORT 0`, la aplicación
  usa NVS y el binario normal deja de contener esos valores.
- NVS todavía es extraíble físicamente hasta habilitar Flash Encryption.
- El código permanece visible y su barra indica la vigencia del intervalo TOTP.

## RTC opcional

Para conservar una hora aproximada durante cortes de red o energía, conecta un
DS3231 con batería, instala `RTClib` desde el Library Manager y cambia
`USE_DS3231` a `1`. La hora del RTC permite arrancar, pero NTP seguirá corrigiéndola
cada seis horas. Si el RTC pierde alimentación, no se generan códigos hasta obtener
una hora válida.

## Flash Encryption y Secure Boot

Estas protecciones no deben activarse desde el sketch ni mediante código que queme
eFuses automáticamente. Se configuran durante la compilación y el flasheo, y los
pasos exactos dependen del modelo de ESP32 y de la versión del core/IDE.

Para una instalación definitiva, usa ESP-IDF y sigue la documentación oficial de
Espressif para tu chip:

1. Genera y respalda fuera del equipo las claves de firma y cifrado.
2. Activa primero Flash Encryption en modo de desarrollo y valida el dispositivo.
3. Activa Secure Boot con una imagen firmada y valida las actualizaciones.
4. Solo entonces migra al modo de producción y deshabilita las interfaces de
   depuración recomendadas para ese chip.

Quemar eFuses es irreversible. No uses comandos preparados para otro modelo de
ESP32 y no pases a producción sin conservar las claves y una vía de actualización
firmada.

## Comportamiento de recuperación

La conexión Wi-Fi y NTP tienen tiempos límite. Si no hay una hora fiable, el
dispositivo oculta el TOTP, muestra un error y vuelve a intentar cada minuto. Tras
sincronizar, apaga el radio Wi-Fi y lo reactiva cada seis horas para corregir el
reloj.
