#pragma once
#include "freertos/FreeRTOS.h"
#include "esp_wifi_types_generic.h"

#define APP_NAME "ESP32 Wifi"
#define MAX_RETRY 10
#define CHECK_ERR(x)                                                           \
  if (x != ESP_OK)                                                             \
    return false;

typedef struct {
  wifi_config_t wifi_config;
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  EventGroupHandle_t event_group;

  unsigned int retry_num;
  char* ip;
} WiFi;

bool init_wifi();

WiFi create_wifi(wifi_config_t config);
void destroy_wifi(WiFi* wifi);
