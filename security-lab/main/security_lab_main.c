#include <stdbool.h>
#include <stdio.h>

#include "esp_efuse.h"
#include "esp_secure_boot.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

void app_main(void)
{
    const bool secure_boot = esp_secure_boot_enabled();
    const bool flash_encryption = esp_efuse_is_flash_encryption_enabled();
    const esp_err_t nvs_result = nvs_flash_init();
    const bool nvs_encryption = CONFIG_NVS_ENCRYPTION && nvs_result == ESP_OK;

    printf("ESP32-C6 security lab\n");
    printf("Secure Boot: %s\n", secure_boot ? "enabled" : "disabled");
    printf("Flash Encryption: %s\n", flash_encryption ? "enabled" : "disabled");
    printf("NVS init: %s\n", esp_err_to_name(nvs_result));
    printf("NVS encryption: %s\n", nvs_encryption ? "enabled" : "disabled");

    if (!secure_boot || !flash_encryption || !nvs_encryption) {
        printf("LAB CHECK FAILED: this image is not production-ready.\n");
        return;
    }

    printf("LAB CHECK PASSED: hardware protections are active.\n");
}
