#include "wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

bool init_wifi() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    CHECK_ERR(nvs_flash_erase());
    ret = nvs_flash_init();
    CHECK_ERR(ret);
  }

  CHECK_ERR(esp_netif_init());
  CHECK_ERR(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  CHECK_ERR(esp_wifi_init(&cfg));
  return true;
}

void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                   void *event_data) {
  WiFi wifi = *(WiFi*)arg;
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (wifi.retry_num < MAX_RETRY) {
      esp_wifi_connect();
      wifi.retry_num++;
      printf("retry to connect to the AP\n");
    } else {
      xEventGroupSetBits(wifi.event_group, BIT1);
    }
    printf("connect to the AP fail");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    char buf[128];
    sprintf(buf, IPSTR, IP2STR(&event->ip_info.ip));
    strcpy(wifi.ip, buf);
    wifi.retry_num = 0;
    xEventGroupSetBits(wifi.event_group, BIT0);
  }
}

WiFi create_wifi(wifi_config_t config) {
  WiFi wifi = {
      .wifi_config = config,
      .retry_num = 0,
  };
  wifi.ip = (char*)malloc(sizeof(char) * 128);

  wifi.event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, &wifi,
      &wifi.instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, &wifi,
      &wifi.instance_got_ip));

  if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
    printf("Cannot WiFi setmode\n");
    esp_restart();
  }
  if (esp_wifi_set_config(WIFI_IF_STA, &wifi.wifi_config)) {
    printf("Cannot set wifi config\n");
    esp_restart();
  }
  if (esp_wifi_start()) {
    printf("cannot start wifi");
    esp_restart();
  }

  unsigned int bits = xEventGroupWaitBits(wifi.event_group, BIT0 | BIT1,
                                          pdFALSE, pdFALSE, portMAX_DELAY);
  if (bits & BIT0) {
    printf("Connected to Wifi with SSID: %s and password: %s\n",
           config.sta.ssid, config.sta.password);
  } else if (bits & BIT1) {
    printf("Can't connect to Wifi with SSID: %s and password: %s\n",
           config.sta.ssid, config.sta.password);
  }

  return wifi;
}

void destroy_wifi(WiFi* wifi) {
  free(wifi->ip);
}
