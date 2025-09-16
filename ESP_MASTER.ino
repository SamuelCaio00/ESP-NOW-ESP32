#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#define LED_BUILTIN 2

uint8_t broadcastAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Substitua pelo MAC do ESP32 receptor

typedef struct struct_message {
  int ledState;
} struct_message;

struct_message myData;
bool lastState = HIGH;

// Callback de envio
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Envio para MAC: ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           info->des_addr[0], info->des_addr[1], info->des_addr[2],
           info->des_addr[3], info->des_addr[4], info->des_addr[5]);
  Serial.print(macStr);
  Serial.print(" - Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Sucesso");
  } else {
    Serial.println("Falha");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(4, INPUT_PULLUP);  // GPIO conectado ao botão
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE); // Canal definido

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Erro ao adicionar peer");
    piscar();
  } else {
    Serial.println("Peer adicionado com sucesso");
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void loop() {
  bool currentState = digitalRead(4);

  if (currentState == LOW && lastState == HIGH) {
    myData.ledState = 1;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    Serial.println(result == ESP_OK ? "Enviando LIGAR LED..." : "Erro ao enviar comando");
    delay(200);
  }

  if (currentState == HIGH && lastState == LOW) {
    myData.ledState = 0;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    Serial.println(result == ESP_OK ? "Enviando DESLIGAR LED..." : "Erro ao enviar comando");
    delay(200);
  }

  lastState = currentState;
}

void piscar() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}
