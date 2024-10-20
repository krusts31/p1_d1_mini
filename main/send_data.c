#include "lwip/sockets.h"
#include "esp_timer.h"
#include "driver/uart.h"
#include "esp_system.h"  // Include this for esp_restart()
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define PORT CONFIG_EXAMPLE_PORT

static const char	*TAG = "DATA_SENDER";

extern int global_socket;

char telegram[1024];
unsigned int currentCRC;
long TIME, DEV_ID, CURRENT_TARIFF, A_PLUS, A_PLUS_TARIFF_1, A_PLUS_TARIFF_2;
long A_PLUS_TARIFF_3, A_PLUS_TARIFF_4, A_MINUS, A_MINUS_TARIFF_1, A_MINUS_TARIFF_2;
long A_MINUS_TARIFF_3, A_MINUS_TARIFF_4, INST_ACTIV_POWER_A_PLUS, INST_ACTIV_POWER_A_MINUS;
long U1, U2, U3, I1, I2, I3, SHORT_FAILURES, LONG_FAILURES;
long SAGS_L1, SAGS_L2, SAGS_L3, SWELLS_L1, SWELLS_L2, SWELLS_L3;

// UART initialization for ESP-IDF
void init_uart()
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_param_config(UART_NUM, &uart_config);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);


static int FindCharInArrayRev(char *array, char c, int len)
{
	for (int i = len - 1; i >= 0; i--)
	{
		if (array[i] == c)
			return i;
	}
	return -1;
}

static bool isNumber(char *res, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != 0))
			return false;
	}
	return true;
}

static long getValue(char *buffer, int maxlen, char startchar, char endchar)
{
	// Find positions of start and end characters
	int s = FindCharInArrayRev(buffer, startchar, maxlen - 2);
	int l = FindCharInArrayRev(buffer, endchar, maxlen - 2) - s - 1;

	// Buffer to store the extracted value
	char res[16];
	memset(res, 0, sizeof(res));

	// Extract substring between startchar and endchar
	if (s >= 0 && l > 0 && s + 1 + l <= maxlen) {
		strncpy(res, buffer + s + 1, l);

		if (endchar == '*') {
			if (isNumber(res, l))
				return (long)(1000 * atof(res));
		} else if (endchar == ')') {
			if (isNumber(res, l))
				return (long)atof(res);
		}
	}

	// Return 0 if no valid value found
	return 0;
}


unsigned int CRC16(unsigned int crc, unsigned char *buf, int len) {
	for (int pos = 0; pos < len; pos++) {
		crc ^= (unsigned int)buf[pos];  // XOR the byte with the current CRC
		for (int i = 8; i != 0; i--) {  // Process each bit
			if ((crc & 0x0001) != 0) {  // If LSB is set
				crc >>= 1;			  // Shift right
				crc ^= 0xA001;		  // XOR with polynomial
			} else {
				crc >>= 1;			  // Just shift right if no LSB is set
			}
		}
	}
	return crc;
}


bool decode_telegram(int len)
{
	int startChar = FindCharInArrayRev(telegram, '/', len);
	int endChar = FindCharInArrayRev(telegram, '!', len);
	bool validCRCFound = false;

	for (int cnt = 0; cnt < len; cnt++) {
		printf("%c", telegram[cnt]);
	}
	printf("\n");

	if (startChar >= 0)
	{
		// Start found, reset CRC calculation
		currentCRC = CRC16(0x0000, (unsigned char *) telegram + startChar, len - startChar);
	}
	else if (endChar >= 0)
	{
		// Add to CRC calculation
		currentCRC = CRC16(currentCRC, (unsigned char *)telegram + endChar, 1);

		char messageCRC[5];
		strncpy(messageCRC, telegram + endChar + 1, 4);
		messageCRC[4] = '\0';  // Null-terminate the CRC string

		validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);

		if (validCRCFound)
			printf("CRC Valid!\n");
		else
			printf("CRC Invalid!\n");

		currentCRC = 0;
	}
	else
	{
		currentCRC = CRC16(currentCRC, (unsigned char *)telegram, len);
	}

	// TIME
	if (strncmp(telegram, "0-0:1.0.0", strlen("0-0:1.0.0")) == 0)
	{
		TIME = getValue(telegram, len, '(', 'S');
		printf("TIME: %ld\n", TIME);
	}

	// DEV_ID (Commented out)
	// if (strncmp(telegram, "0-0:96.1.1", strlen("0-0:96.1.1")) == 0)
	// {
	//	 DEV_ID = getValue(telegram, len, '(', ')');
	//	 printf("DEV_ID: %ld\n", DEV_ID);
	// }

	// CURRENT_TARIFF
	if (strncmp(telegram, "0-0:96.14.0", strlen("0-0:96.14.0")) == 0)
	{
		CURRENT_TARIFF = getValue(telegram, len, '(', ')');
	}

	// A_PLUS
	if (strncmp(telegram, "1-0:1.8.0", strlen("1-0:1.8.0")) == 0)
	{
		A_PLUS = getValue(telegram, len, '(', '*');
		printf("A_PLUS: %ld\n", A_PLUS);
	}

	// A_PLUS_TARIFF_1
	if (strncmp(telegram, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0)
	{
		A_PLUS_TARIFF_1 = getValue(telegram, len, '(', '*');
	}

	// A_PLUS_TARIFF_2
	if (strncmp(telegram, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0)
	{
		A_PLUS_TARIFF_2 = getValue(telegram, len, '(', '*');
	}

	// A_PLUS_TARIFF_3
	if (strncmp(telegram, "1-0:1.8.3", strlen("1-0:1.8.3")) == 0)
	{
		A_PLUS_TARIFF_3 = getValue(telegram, len, '(', '*');
	}

	// A_PLUS_TARIFF_4
	if (strncmp(telegram, "1-0:1.8.4", strlen("1-0:1.8.4")) == 0)
	{
		A_PLUS_TARIFF_4 = getValue(telegram, len, '(', '*');
	}

	// A_MINUS
	if (strncmp(telegram, "1-0:2.8.0", strlen("1-0:2.8.0")) == 0)
	{
		A_MINUS = getValue(telegram, len, '(', '*');
	}

	// A_MINUS_TARIFF_1
	if (strncmp(telegram, "1-0:2.8.1", strlen("1-0:2.8.1")) == 0)
	{
		A_MINUS_TARIFF_1 = getValue(telegram, len, '(', '*');
	}

	// A_MINUS_TARIFF_2
	if (strncmp(telegram, "1-0:2.8.2", strlen("1-0:2.8.2")) == 0)
	{
		A_MINUS_TARIFF_2 = getValue(telegram, len, '(', '*');
	}

	// A_MINUS_TARIFF_3
	if (strncmp(telegram, "1-0:2.8.3", strlen("1-0:2.8.3")) == 0)
	{
		A_MINUS_TARIFF_3 = getValue(telegram, len, '(', '*');
	}

	// A_MINUS_TARIFF_4
	if (strncmp(telegram, "1-0:2.8.4", strlen("1-0:2.8.4")) == 0)
	{
		A_MINUS_TARIFF_4 = getValue(telegram, len, '(', '*');
	}

	// INST_ACTIV_POWER_A_PLUS
	if (strncmp(telegram, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0)
	{
		INST_ACTIV_POWER_A_PLUS = getValue(telegram, len, '(', '*');
	}

	// INST_ACTIV_POWER_A_MINUS
	if (strncmp(telegram, "1-0:2.7.0", strlen("1-0:2.7.0")) == 0)
	{
		INST_ACTIV_POWER_A_MINUS = getValue(telegram, len, '(', '*');
	}

	// U1, U2, U3 (Voltages)
	if (strncmp(telegram, "1-0:32.7.0", strlen("1-0:32.7.0")) == 0)
	{
		U1 = getValue(telegram, len, '(', '*');
	}
	if (strncmp(telegram, "1-0:52.7.0", strlen("1-0:52.7.0")) == 0)
	{
		U2 = getValue(telegram, len, '(', '*');
	}
	if (strncmp(telegram, "1-0:72.7.0", strlen("1-0:72.7.0")) == 0)
	{
		U3 = getValue(telegram, len, '(', '*');
	}

	// I1, I2, I3 (Currents)
	if (strncmp(telegram, "1-0:31.7.0", strlen("1-0:31.7.0")) == 0)
	{
		I1 = getValue(telegram, len, '(', '*');
	}
	if (strncmp(telegram, "1-0:51.7.0", strlen("1-0:51.7.0")) == 0)
	{
		I2 = getValue(telegram, len, '(', '*');
	}
	if (strncmp(telegram, "1-0:71.7.0", strlen("1-0:71.7.0")) == 0)
	{
		I3 = getValue(telegram, len, '(', '*');
	}

	// SHORT_FAILURES, LONG_FAILURES
	if (strncmp(telegram, "0-0:96.7.21", strlen("0-0:96.7.21")) == 0)
	{
		SHORT_FAILURES = getValue(telegram, len, '(', ')');
	}
	if (strncmp(telegram, "0-0:96.7.9", strlen("0-0:96.7.9")) == 0)
	{
		LONG_FAILURES = getValue(telegram, len, '(', ')');
	}

	// SAGS_L1, SAGS_L2, SAGS_L3 (Voltage Sags)
	if (strncmp(telegram, "1-0:32.32.0", strlen("1-0:32.32.0")) == 0)
	{
		SAGS_L1 = getValue(telegram, len, '(', ')');
	}
	if (strncmp(telegram, "1-0:52.32.0", strlen("1-0:52.32.0")) == 0)
	{
		SAGS_L2 = getValue(telegram, len, '(', ')');
	}
	if (strncmp(telegram, "1-0:72.32.0", strlen("1-0:72.32.0")) == 0)
	{
		SAGS_L3 = getValue(telegram, len, '(', ')');
	}

	// SWELLS_L1, SWELLS_L2, SWELLS_L3 (Voltage Swells)
	if (strncmp(telegram, "1-0:32.36.0", strlen("1-0:32.36.0")) == 0)
	{
		SWELLS_L1 = getValue(telegram, len, '(', ')');
	}
	if (strncmp(telegram, "1-0:52.36.0", strlen("1-0:52.36.0")) == 0)
	{
		SWELLS_L2 = getValue(telegram, len, '(', ')');
	}
	if (strncmp(telegram, "1-0:72.36.0", strlen("1-0:72.36.0")) == 0)
	{
		SWELLS_L3 = getValue(telegram, len, '(', ')');
	}

	return validCRCFound;
}

void read_p1_hardwareserial()
{
    uint8_t data[BUF_SIZE];
    int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, 20 / portTICK_RATE_MS);

    if (len > 0)
    {
        memset(telegram, 0, sizeof(telegram));
        memcpy(telegram, data, len);
        processLine(len);  // Process the line
    }
}

void processLine(int len)
{
    telegram[len] = '\n';  // Append newline
    telegram[len + 1] = 0;  // Null-terminate the string

    bool result = decode_telegram(len + 1);  // Decode the telegram
    if (result) {
        send_data_to_socket();  // Send the data if decoding was successful
        LAST_UPDATE_SENT = esp_timer_get_time() / 1000;  // Update timestamp in milliseconds
    }
}

void send_metric_to_socket(const char *name, long metric)
{
	char output[10];   // Buffer for the metric value as a string
	char buffer[128];  // Buffer for the full message

	// Convert the metric value to a string
	snprintf(output, sizeof(output), "%ld", metric);

	// Format the message as "name=metric"
	snprintf(buffer, sizeof(buffer), "%s=%s\n", name, output);

	// Send the formatted message over the socket
	if (global_socket != -1) {
		int err = send(global_socket, buffer, strlen(buffer), 0);
		if (err < 0) {
			        ESP_LOGE(TAG, "Error sending data: errno %d. Restarting ESP...", errno);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
			        esp_restart();  // Restart the ESP
		} else {
			ESP_LOGI(TAG, "Data sent: %s", buffer);
		}
	}
}

void send_data_to_socket()
{
	// Send each metric over the socket using send_metric_to_socket()
	send_metric_to_socket("TIME", TIME);
	//send_metric_to_socket("DEV_ID", DEV_ID);
	send_metric_to_socket("CURRENT_TARIFF", CURRENT_TARIFF);
	send_metric_to_socket("A_PLUS", A_PLUS);
	send_metric_to_socket("A_PLUS_TARIFF_1", A_PLUS_TARIFF_1);
	send_metric_to_socket("A_PLUS_TARIFF_2", A_PLUS_TARIFF_2);
	send_metric_to_socket("A_PLUS_TARIFF_3", A_PLUS_TARIFF_3);
	send_metric_to_socket("A_PLUS_TARIFF_4", A_PLUS_TARIFF_4);
	send_metric_to_socket("A_MINUS", A_MINUS);
	send_metric_to_socket("A_MINUS_TARIFF_1", A_MINUS_TARIFF_1);
	send_metric_to_socket("A_MINUS_TARIFF_2", A_MINUS_TARIFF_2);
	send_metric_to_socket("A_MINUS_TARIFF_3", A_MINUS_TARIFF_3);
	send_metric_to_socket("A_MINUS_TARIFF_4", A_MINUS_TARIFF_4);
	send_metric_to_socket("INST_ACTIV_POWER_A_PLUS", INST_ACTIV_POWER_A_PLUS);
	send_metric_to_socket("INST_ACTIV_POWER_A_MINUS", INST_ACTIV_POWER_A_MINUS);
	send_metric_to_socket("U1", U1);
	send_metric_to_socket("U2", U2);
	send_metric_to_socket("U3", U3);
	send_metric_to_socket("I1", I1);
	send_metric_to_socket("I2", I2);
	send_metric_to_socket("I3", I3);
	send_metric_to_socket("SHORT_FAILURES", SHORT_FAILURES);
	send_metric_to_socket("LONG_FAILURES", LONG_FAILURES);
	send_metric_to_socket("SAGS_L1", SAGS_L1);
	send_metric_to_socket("SAGS_L2", SAGS_L2);
	send_metric_to_socket("SAGS_L3", SAGS_L3);
	send_metric_to_socket("SWELLS_L1", SWELLS_L1);
	send_metric_to_socket("SWELLS_L2", SWELLS_L2);
	send_metric_to_socket("SWELLS_L3", SWELLS_L3);
}

// Periodically reads and decodes telegram data, then sends metrics
void data_sender_task(void *pvParameters)
{
	while (1) {
		ESP_LOGE(TAG, "Reading and sending data...");

		if (global_socket != -1) {
			// Simulate reading data into the telegram array
			// You would have actual code to read the telegram from a serial device or other source
			int telegram_length = 1024; // Example length, replace with actual telegram length

			// Call decode_telegram which will decode and send data
			read_p1_hardwareserial();
			decode_telegram(telegram_length);
			send_data_to_socket();
		}

		vTaskDelay(5000 / portTICK_PERIOD_MS);  // Wait for 5 seconds
	}
}
