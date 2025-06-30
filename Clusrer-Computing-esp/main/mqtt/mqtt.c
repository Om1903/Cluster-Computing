#include "mqtt.h"
#include "driver/gpio.h"
#include "esp_wifi.h"

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data);

static bool img_received = false;
static size_t buf_idx = 0;
static uint8_t img_buffer[IMG_SIZE];

void mqtt_init(esp_mqtt_client_config_t *mqtt_cfg, MQTT *mqtt) {
  uint8_t mac[6];
  char mac_str[18];

  esp_wifi_get_mac(WIFI_IF_STA, mac);
  snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],
           mac[1], mac[2], mac[3], mac[4], mac[5]);
  for (int i = 0; i < 18; i++) {
    mqtt->mac_addr[i] = mac_str[i];
  }
  mqtt->mac_addr[18] = '\0';

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 mqtt);
  esp_mqtt_client_start(client);
  mqtt->client_handle = client;
}

static void send_macaddr(esp_mqtt_client_handle_t client, MQTT *mqtt) {
  esp_mqtt_client_publish(client, "/topic/identity", mqtt->mac_addr, 0, 1, 0);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  MQTT mqtt = *(MQTT *)handler_args;
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    printf("Connected to MQTT\n");
    send_macaddr(client, handler_args);
    char buf[256];
    sprintf(buf, "/topic/%s", mqtt.mac_addr);
    esp_mqtt_client_subscribe(client, buf, 1);
    break;
  case MQTT_EVENT_DISCONNECTED:
    printf("Disconnected from mqtt\n");
    break;
  case MQTT_EVENT_DATA:
    if (buf_idx + event->data_len <= IMG_SIZE) {
      memcpy(&img_buffer[buf_idx], event->data, event->data_len);
      buf_idx += event->data_len;

      if (buf_idx == IMG_SIZE && !img_received) {
        img_received = true;
        printf("Recv...\n");
        gpio_set_level(2, 1);

        for (int i = 0; i < IMG_WIDTH; i++) {
          for (int j = 0; j < IMG_HEIGHT; j++) {
            int bidx = PIXEL(i, j, 0);
            int r = img_buffer[bidx], g = img_buffer[bidx + 1],
                b = img_buffer[bidx + 2];
            int avg = (r + g + b) / 3;
            img_buffer[bidx + 0] = avg;
            img_buffer[bidx + 1] = avg;
            img_buffer[bidx + 2] = avg;
          }
        }
        char buf[256];
        sprintf(buf, "/topic/%s/out", mqtt.mac_addr);
        esp_mqtt_client_publish(mqtt.client_handle, buf, (const char*)img_buffer, IMG_SIZE,
                                1, 0);
        gpio_set_level(2, 0);

        buf_idx = 0;
        img_received = false;
      }
    } else {
    }
    break;
  default:
    break;
  }
}
