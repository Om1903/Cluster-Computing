#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "mqtt/mqtt.h"
#include "mqtt_client.h"
#include "portmacro.h"
#include "wifi/wifi.h"
#include <memory.h>
#include <string.h>

#define SSID "gp"
#define PASSWORD "hellhell"
#define BROKER_URI "mqtt://192.168.132.195:1883"

void app_main(void) {
  gpio_reset_pin(2);
  gpio_set_direction(2, GPIO_MODE_OUTPUT);

  if (!init_wifi()) {
    printf("Cannot intialize wifi\n");
    esp_restart();
  }
  wifi_config_t config = {.sta = {
                              .ssid = SSID,
                              .password = PASSWORD,
                          }};
  WiFi wifi = create_wifi(config);
  if (wifi.ip == NULL) {
    printf("Cannot connect to WIFI\n");
    esp_restart();
  }
  printf("ip-%s\n", wifi.ip);

  esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = BROKER_URI,
      .broker.address.port = 1883,
  };
  MQTT mqtt;
  mqtt_init(&mqtt_cfg, &mqtt);

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  destroy_wifi(&wifi);
}
