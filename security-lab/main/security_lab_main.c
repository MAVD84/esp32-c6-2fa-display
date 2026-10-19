#include <stdbool.h>
#include <stdio.h>

#include "esp_efuse.h"
#include "esp_partition.h"
#include "esp_secure_boot.h"
#include "esp_system.h"
#include "nvs_flash.h"

void app_main(void)
{
    const bool secure_boot = esp_secure_boot_enabled();
    const bool flash_encryption = esp_efuse_is_flash_encryption_enabled();
    const esp_err_t nvs_result = nvs_flash_init();
    const esp_partition_t *nvs = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        ESP_PARTITION_SUBTYPE_DATA_NVS,
        "nvs");

    printf("ESP32-C6 security lab\n");
    printf("Secure Boot: %s\n", secure_boot ? "enabled" : "disabled");
    printf("Flash Encryption: %s\n", flash_encryption ? "enabled" : "disabled");
    printf("NVS init: %s\n", esp_err_to_name(nvs_result));
    printf("NVS partition encrypted: %s\n",
           nvs != NULL && nvs->encrypted ? "yes" : "no");

    if (!secure_boot || !flash_encryption || nvs_result != ESP_OK || nvs == NULL ||
        !nvs->encrypted) {
        printf("LAB CHECK FAILED: this image is not production-ready.\n");
        return;
    }

    printf("LAB CHECK PASSED: hardware protections are active.\n");
}
