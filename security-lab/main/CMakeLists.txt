idf_component_register(
    SRCS "security_lab_main.c"
    PRIV_REQUIRES bootloader_support efuse esp_partition nvs_flash
    INCLUDE_DIRS ""
)
