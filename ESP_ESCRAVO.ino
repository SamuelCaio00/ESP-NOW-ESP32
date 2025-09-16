#include <WiFi.h>       // Biblioteca para controlar Wi-Fi do ESP32
#include <esp_now.h>    // Biblioteca para comunicação ESP-NOW
#include <esp_wifi.h>   // Biblioteca para configuração avançada do Wi-Fi

// Define a estrutura da mensagem recebida
typedef struct struct_message {
  int ledState; // Estado do LED: 1 = ligado, 0 = desligado
} struct_message;

struct_message incomingData; // Variável que armazenará os dados recebidos

// Callback chamado sempre que uma mensagem ESP-NOW é recebida
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataBytes, int len) {
  // Copia os dados recebidos para a estrutura
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

  // Exibe o estado recebido do LED
  Serial.print("Mensagem recebida. LED = ");
  Serial.println(incomingData.ledState);

  // Converte o endereço MAC do dispositivo que enviou a mensagem em uma string legível
  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);

  // Mostra o MAC do remetente
  Serial.print("Mensagem recebida do MAC: ");
  Serial.println(macStr);

  // Atualiza o LED físico no GPIO 32 de acordo com a mensagem recebida
  digitalWrite(32, incomingData.ledState);
}

void setup() {
  Serial.begin(115200);     // Inicializa a comunicação serial
  Serial.print(WiFi.macAddress()); // Mostra o MAC do ESP32 receptor no Serial Monitor

  pinMode(32, OUTPUT);      // Configura o pino do LED como saída

  // Configura o ESP32 em modo estação e desliga o Wi-Fi para uso exclusivo do ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Define o canal Wi-Fi para comunicação ESP-NOW (deve ser o mesmo do transmissor)
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  // Inicializa o protocolo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.print("Erro ao iniciar ESP-NOW");
    return; // Sai do setup se houver erro
  }

  // Registra a função de callback para receber mensagens
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Loop vazio, o ESP32 só precisa aguardar mensagens recebidas
  // Toda ação é feita na função OnDataRecv
}
