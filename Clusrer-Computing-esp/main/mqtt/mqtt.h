#pragma once
#include "mqtt_client.h"

#define IMG_WIDTH 128
#define IMG_HEIGHT 128
#define IMG_SIZE IMG_WIDTH * IMG_HEIGHT * 3
#define PIXEL(x, y, c) ((x * IMG_WIDTH + y) * 3 + c)

typedef struct {
  char mac_addr[19];
  esp_mqtt_client_handle_t client_handle;
} MQTT;

void mqtt_init(esp_mqtt_client_config_t* mqtt_cfg, MQTT* mqtt);
