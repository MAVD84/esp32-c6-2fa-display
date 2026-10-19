# Laboratorio de seguridad para ESP32-C6

> **PELIGRO — SOLO COMPILACIÓN. NO FLASHEAR EN LA PLACA PRINCIPAL.**

Este proyecto mínimo permite comprobar que ESP-IDF puede construir para ESP32-C6
una imagen con Secure Boot V2, Flash Encryption en modo de desarrollo y NVS
cifrada. No contiene el generador TOTP ni ningún secreto real.

`build-only.ps1` hace solamente tres cosas: genera una clave privada local cuando
no existe, selecciona ESP32-C6 y compila. No contiene órdenes de carga, borrado de
flash ni escritura de eFuses.

## Uso seguro

1. Instala ESP-IDF 6.1 mediante la herramienta oficial de Espressif.
2. Abre PowerShell en esta carpeta.
3. Ejecuta `./build-only.ps1`.
4. El script guarda la clave de prueba en
   `%LOCALAPPDATA%\MAVD\esp32-c6-2fa-security-lab`, fuera de OneDrive y del
   repositorio. Para producción utiliza una clave distinta y conserva una copia de
   respaldo cifrada y desconectada.
5. Usa una **segunda placa desechable** para cualquier ensayo de encendido. La
   primera ejecución de una imagen configurada de este modo puede programar eFuses
   de forma irreversible.

## Lo que aún falta para producción

- Portar la pantalla, sincronización de hora y TOTP desde Arduino a ESP-IDF.
- Diseñar un aprovisionamiento que escriba el secreto directamente en NVS cifrada.
- Diseñar y probar actualización firmada y recuperación.
- Verificar todo primero en una placa de prueba y conservar las claves fuera del
  repositorio y de carpetas sincronizadas.
- Crear después un perfil de producción en modo Release y revisar el estado final
  de cada eFuse. Release no debe ser el primer experimento.

No copies comandos de `efuse-burn` de otros modelos. La secuencia final debe seguir
la documentación oficial específica del ESP32-C6 y de la versión de ESP-IDF usada.
