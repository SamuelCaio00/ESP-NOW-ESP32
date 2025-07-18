#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#define LED_BUILTIN 2

uint8_t broadcastAddress[] = {0xB0, 0xA7, 0x32, 0x17, 0x9C, 0xA0};// substitua pelo MAC do seu esp32

typedef struct struct_message {
  int ledState;
} struct_message;

struct_message myData;
bool lastState = HIGH;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Envio para MAC ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2],
           mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" - Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("SUCESSO");
  } else {
    Serial.println("FALHA");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(4, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE); // Defina o canal desejado

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
    
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1; // Mesmo canal dos dois ESP32
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;
  digitalWrite(2, HIGH);
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

void piscar (){
  
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
  }
