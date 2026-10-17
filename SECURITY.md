# Política de seguridad

## Estado del proyecto

Este repositorio es un prototipo educativo y no ha sido auditado ni certificado.
No debe considerarse equivalente a una llave FIDO2/WebAuthn ni utilizarse como
único método de recuperación para cuentas críticas.

## Modelo de amenazas actual

El proyecto intenta evitar la publicación accidental de credenciales y rechaza
configuraciones básicas inseguras durante la compilación. El Wi-Fi se utiliza solo
para obtener la hora y se apaga después de sincronizar.

La versión Arduino actual **no protege** contra un atacante con acceso físico capaz
de leer la memoria flash. La clave TOTP permanece en el firmware mientras no se
habiliten Flash Encryption y Secure Boot. NTP tampoco autentica criptográficamente
la fuente de tiempo, por lo que una red hostil puede afectar disponibilidad o
provocar una hora incorrecta; no debería poder obtener la clave HMAC por ese medio.

## Divulgación responsable

No publiques vulnerabilidades acompañadas de claves, códigos TOTP, imágenes QR,
credenciales Wi-Fi ni firmware extraído de un dispositivo real. Repórtalas mediante
un aviso privado al propietario del repositorio en GitHub e incluye únicamente los
pasos mínimos para reproducir el problema con credenciales de prueba.

## Antes de un uso sensible

- Implementar aprovisionamiento local sin secretos compilados en el repositorio.
- Migrar la compilación de producción a ESP-IDF.
- Activar y validar Secure Boot y Flash Encryption en una placa de prueba.
- Definir una estrategia de actualización firmada y recuperación.
- Deshabilitar interfaces de depuración solo después de validar la recuperación.
- Sustituir NTP sin autenticación por una fuente de tiempo autenticada o un diseño
  con RTC y detección de manipulación adecuado al modelo de amenazas.

Los eFuses son irreversibles. Este proyecto no los programa automáticamente.
