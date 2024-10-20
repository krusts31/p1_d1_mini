#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tcpip_adapter.h"
#include "tcp_server.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs.h"
#include "data_sender.h"
#include "wifi.h"

static const char *TAG = "MAIN";

int global_socket = -1;

void app_main()
{
	tcpip_adapter_init();
	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	//TODO use spiffs to check if there are wifi credentils embede
	/*
	httpd_handle_t *ret_2 = wrapper_start_webserver();
	if (ret_2 != NULL) {
		printf("started web server!");
	} else {
		printf("Failed to start web server.");
		fflush(stdout);
		return ;
	}
	*/
	//if not start in paring mode?
	ESP_ERROR_CHECK(wifi_connect());

	xTaskCreate(tcp_server, "tcp_server", 4096, NULL, 5, NULL);
	xTaskCreate(data_sender_task, "data_sender", 4096, NULL, 5, NULL);

	while (1) {
		ESP_LOGI(TAG, "Looping");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	ESP_LOGI(TAG, "Restart");
	fflush(stdout);
	esp_restart();
}
