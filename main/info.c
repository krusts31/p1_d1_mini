#include "info.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

static const char *TAG = "ESP_INFO";

void print_esp_info(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    ESP_LOGI(TAG, "This is ESP8266 chip with CPU cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "Silicon revision: %d", chip_info.revision);
    ESP_LOGI(TAG, "%d MB flash", spi_flash_get_chip_size() / (1024 * 1024));
    ESP_LOGI(TAG, "Flash type: %s", (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
}
