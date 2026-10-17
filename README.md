# Generador 2FA con ESP32-C6 y pantalla ST7789

> **Prototipo educativo:** no auditado ni certificado. La versión Arduino no
> protege el secreto frente a extracción física de la flash. Consulta
> [SECURITY.md](SECURITY.md) antes de utilizarlo con una cuenta real.

Este proyecto muestra códigos TOTP de seis dígitos en una pantalla de 320 x 172.
Al encender, sincroniza la hora mediante la red Wi-Fi configurada, apaga el radio y
actualiza el código cada 30 segundos.

## Material necesario

- ESP32-C6 con pantalla ST7789 compatible con el cableado del sketch.
- Arduino IDE 2.x.
- Core `esp32` de Espressif (probado con 3.3.12).
- Biblioteca **GFX Library for Arduino** de Moon On Our Nation (probado con 1.6.9).
- Biblioteca **TOTP library** de Luca Dentella (probado con 1.1.0).

> Los pines de pantalla incluidos pertenecen al modelo usado por el autor. Otra
> placa o pantalla puede requerir cambiar los pines y la construcción de
> `Arduino_ESP32SPI`/`Arduino_ST7789` al inicio del archivo `.ino`.

## Configuración

1. Descomprime el ZIP conservando la carpeta `sketch_2fa`.
2. Abre `secrets.example.h`, completa el SSID y contraseña de tu Wi-Fi y guárdalo
   temporalmente con el nombre `secrets.h` en la misma carpeta del `.ino`.
3. Obtén la clave secreta Base32 al configurar TOTP en la cuenta. Guarda también
   los códigos de recuperación proporcionados por el servicio.
4. Ejecuta `convertir_base32_a_hex.ps1` localmente. Pega únicamente la clave
   Base32, sin la dirección `otpauth://` completa.
5. Copia la línea `#define TOTP_HMAC_KEY {...}` generada por el conversor y
   reemplaza esa línea en `secrets.h`.
6. En Arduino IDE instala las dos bibliotecas indicadas mediante el Administrador
   de bibliotecas.
7. Selecciona **ESP32C6 Dev Module** y el puerto correcto.
8. En `sketch_2fa.ino`, cambia temporalmente
   `ALLOW_BUILD_SECRET_IMPORT` de `0` a `1`, verifica y sube el firmware.
9. Espera a que aparezca un código válido: esto confirma que la configuración fue
   guardada en NVS.
10. Vuelve a cambiar `ALLOW_BUILD_SECRET_IMPORT` a `0`, verifica y sube de nuevo.
    Esta segunda carga genera el firmware normal sin incluir `secrets.h`.
11. Elimina o guarda fuera del proyecto el archivo `secrets.h` y compara varios
    códigos con otra app autenticadora antes de depender del equipo.

No distribuyas ni conserves como versión final el primer binario de importación.
El firmware con `ALLOW_BUILD_SECRET_IMPORT 0` arranca desde la configuración NVS y
puede compilarse aunque `secrets.h` no exista.

Si Windows indica que el puerto está ocupado, cierra el Monitor Serie antes de
intentar la carga.

## Seguridad y recuperación

- Nunca compartas `secrets.h`, capturas del QR, la clave Base32 ni un firmware
  compilado (`.bin`): todos contienen o permiten recuperar el secreto TOTP.
- La segunda compilación ya no contiene esos valores, pero NVS permanece sin cifrar
  hasta habilitar Flash Encryption; un atacante físico todavía puede extraerla.
- Conserva los códigos de recuperación de cada cuenta en un lugar separado.
- Si el dispositivo se pierde, es robado o se comparte su firmware, revoca ese
  autenticador desde la cuenta y registra uno nuevo.
- Limitar la sincronización a un Wi-Fi concreto no es una protección absoluta. Un
  SSID puede ser imitado y, mientras siga encendido con hora válida, el dispositivo
  continúa generando códigos sin conexión.
- El equipo reduce la dependencia del teléfono, pero no sustituye una llave FIDO2
  para cuentas que necesiten resistencia fuerte contra phishing.
- `secrets.h` está excluido por `.gitignore`, pero servicios como OneDrive pueden
  sincronizarlo. Revisa dónde guardas la carpeta.

## Comportamiento esperado

- Inicio: logotipo `2FA` y `BY MAVD`, sin barra.
- Operación: código TOTP y barra de vigencia de 30 segundos.
- Últimos cinco segundos: código y barra en rojo.
- Sin hora confiable: no muestra ningún código y reintenta la sincronización.

Consulta también `SEGURIDAD.md` para RTC opcional, Flash Encryption y Secure Boot.
