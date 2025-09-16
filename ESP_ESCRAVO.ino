#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h> 

typedef struct struct_message {
  int ledState;
} struct_message;

struct_message incomingData;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  Serial.print("Mensagem recebida. LED = ");
  Serial.print(incomingData.ledState);

  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);
  Serial.print("Mensagem recebida do MAC: ");
  Serial.println(macStr);

  Serial.print("LED = ");
  Serial.println(incomingData.ledState);
  digitalWrite(32, incomingData.ledState);
}

void setup() {
  Serial.begin(115200);
  Serial.print(WiFi.macAddress());
  pinMode(32, OUTPUT);
 WiFi.mode(WIFI_STA);
 WiFi.disconnect();
 esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.print("Erro ao iniciar ESP-NOW");
    return;
  }

  // Registra função de callback para quando dados forem recebidos
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Nada necessário no loop principal
}
