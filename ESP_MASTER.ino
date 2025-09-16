#include <WiFi.h>       // Biblioteca para controle do Wi-Fi do ESP32
#include <esp_now.h>    // Biblioteca para comunicação ESP-NOW
#include <esp_wifi.h>   // Biblioteca para configurar canais e modos do Wi-Fi

#define LED_BUILTIN 2   // Pino do LED interno do ESP32 (GPIO2)

// Endereço MAC do ESP32 receptor
uint8_t broadcastAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
// Substitua pelo MAC real do dispositivo receptor

// Estrutura que define a mensagem enviada
typedef struct struct_message {
  int ledState; // Estado do LED: 1 = ligado, 0 = desligado
} struct_message;

struct_message myData;  // Variável que armazenará a mensagem
bool lastState = HIGH;  // Armazena o último estado do botão

// Callback chamado após o envio de uma mensagem
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Envio para MAC: ");
  
  // Formata o endereço MAC do peer
  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           info->des_addr[0], info->des_addr[1], info->des_addr[2],
           info->des_addr[3], info->des_addr[4], info->des_addr[5]);
  Serial.print(macStr);

  // Exibe o status do envio
  Serial.print(" - Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Sucesso");
  } else {
    Serial.println("Falha");
  }
}

void setup() {
  Serial.begin(115200);       // Inicializa a comunicação serial
  pinMode(4, INPUT_PULLUP);   // Define o botão como entrada com resistor pull-up
  pinMode(LED_BUILTIN, OUTPUT); // Define o LED interno como saída

  // Configura o ESP32 em modo estação e desliga o Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // Define o canal de comunicação ESP-NOW (deve ser o mesmo do receptor)
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  // Inicializa o protocolo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }

  // Registra a função de callback para envio
  esp_now_register_send_cb(OnDataSent);

  // Configura o peer (dispositivo receptor)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6); // MAC do peer
  peerInfo.channel = 1;       // Canal deve ser o mesmo do receptor
  peerInfo.encrypt = false;   // Sem criptografia
  peerInfo.ifidx = WIFI_IF_STA;

  // Adiciona o peer ao ESP-NOW
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Erro ao adicionar peer");
    piscar(); // Indica falha piscando o LED
  } else {
    Serial.println("Peer adicionado com sucesso");
    digitalWrite(LED_BUILTIN, HIGH); // Acende LED indicando sucesso
  }
}

void loop() {
  bool currentState = digitalRead(4); // Lê o estado do botão

  // Detecta transição do botão para pressionado
  if (currentState == LOW && lastState == HIGH) {
    myData.ledState = 1; // Define LED como ligado
    // Envia mensagem para o peer
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    Serial.println(result == ESP_OK ? "Enviando LIGAR LED..." : "Erro ao enviar comando");
    delay(200); // Debounce simples do botão
  }

  // Detecta transição do botão para liberado
  if (currentState == HIGH && lastState == LOW) {
    myData.ledState = 0; // Define LED como desligado
    // Envia mensagem para o peer
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    Serial.println(result == ESP_OK ? "Enviando DESLIGAR LED..." : "Erro ao enviar comando");
    delay(200); // Debounce
  }

  lastState = currentState; // Atualiza o último estado do botão
}

// Função que pisca o LED interno em caso de erro
void piscar() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}
