#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h> 

// Modos do controle:
// '0' = modo 'calibração'
// '1' = modo 'seguir linha'

#define MODE_PIN 23
#define ACTION_PIN 22

uint8_t receiverAddress[] = {0x08, 0xA6, 0xF7, 0xB1, 0xDA, 0x54};

// command  = 's' (segue)
//          = 'n' (não segue)
//          = 'c' (calibra)
//          = 'm' (não calibra)

char command = 'n';     // começa em 'não segue'
esp_now_peer_info_t peerInfo;

void dataSent(const uint8_t *mac, esp_now_send_status_t status){
  if(status != ESP_NOW_SEND_SUCCESS) {
     Serial.println("Falha de envio");
  }
}

void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(MODE_PIN, INPUT_PULLUP);
  pinMode(ACTION_PIN, INPUT_PULLUP);
  
  if (esp_now_init() != ESP_OK){
    Serial.println("Erro ao inicializar ESP-NOW");
    return;
  }

  esp_now_register_send_cb(esp_now_send_cb_t(dataSent));

  memcpy(peerInfo.peer_addr, receiverAddress, 6);         // copia o endereço do receiver em peer_addr
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Falha ao adicionar peer");
    return;
  }
}

void loop(){
  
  int modeState = digitalRead(MODE_PIN);
  int actionState = digitalRead(ACTION_PIN);

  // MODO 'SEGUIR LINHA'
  if(modeState == LOW){
    if(actionState == LOW){
      command = 's';
    }else{
      command = 'n';
    }
  // MODO 'CALIBRAÇÃO'
  }else{
    if(actionState == LOW){
      command = 'c';
    }else{
      command = 'm';
    }
  }
  Serial.print("Command: ");
  Serial.println(command);
  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *) &command, sizeof(command));

  if(result != ESP_OK){
    Serial.println("Erro de envio");
  }

  delay(50);
}