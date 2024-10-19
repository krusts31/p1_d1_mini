#include "esp_event.h"
#include "tcpip_adapter.h"

#include "protocol_examples_common.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "esp_netif.h"

void app_main()
{
	//esp_chip_info_t chip_info;
	//esp_chip_info(&chip_info);

	//printf("This is ESP8266 chip with %d CPU cores, WiFi, ",
	//chip_info.cores);
	//printf("silicon revision %d, ", chip_info.revision);
	//printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),


	//needed for WIFI

	tcpip_adapter_init();
	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	esp_err_t ret = example_connect();

	if (ret == ESP_OK) {
		printf("Connected to Wi-Fi!\n");
	} else {
		printf("Failed to connect to Wi-Fi.\n");
		fflush(stdout);
		return ;
	}

	httpd_handle_t *ret_2 = wrapper_start_webserver();
	if (ret_2 != NULL) {
		printf("started web server!\n");
	} else {
		printf("Failed to start web server.\n");
		fflush(stdout);
		return ;
	}

	while (1) {
		printf("looping\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	printf("Restarting now.\n");
	fflush(stdout);
	esp_restart();
}
