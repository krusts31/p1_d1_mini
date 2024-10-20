#ifndef ESP_DATA_SENDER_H
#define ESP_DATA_SENDER_H

extern int global_socket;

void data_sender_task(void *pvParameters);

#endif
