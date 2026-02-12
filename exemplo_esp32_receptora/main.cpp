/*
Este código é somente um exemplo do uso do protocolo ESPNOW para controle do seguidor de linha com outra esp,
basicamente o código abaixo acende leds a depender da situação (calibração ou seguir linha).
*/


#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#define LINE_FOLLOW_PIN 14
#define CALIBRATION_PIN 27 
#define CONNECTION_PIN 26

// command  = 's' (segue)
//          = 'n' (não segue)
//          = 'c' (calibra)
//          = 'm' (não calibra)

char command;

void dataReceived (const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(char)){
    memcpy(&command, incomingData, sizeof(command));
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(LINE_FOLLOW_PIN, OUTPUT);
  pinMode(CALIBRATION_PIN, OUTPUT);
  pinMode(CONNECTION_PIN, OUTPUT);

  if(esp_now_init() == ESP_OK) {
    digitalWrite(CONNECTION_PIN, HIGH);
  }
  else{
    digitalWrite(CONNECTION_PIN, LOW);
    return;
  }
}

void loop() {
  esp_now_register_recv_cb(esp_now_recv_cb_t(dataReceived));  

  if (command == 's') {
    digitalWrite(LINE_FOLLOW_PIN, HIGH);
    digitalWrite(CALIBRATION_PIN, LOW);
  } else if (command == 'n') {
    digitalWrite(LINE_FOLLOW_PIN, LOW);
    digitalWrite(CALIBRATION_PIN, LOW);
  } else if (command == 'c') {
    digitalWrite(CALIBRATION_PIN, HIGH);
    digitalWrite(LINE_FOLLOW_PIN, LOW);
  } else if (command == 'm') {
    digitalWrite(CALIBRATION_PIN, LOW);
    digitalWrite(LINE_FOLLOW_PIN, LOW);
  } else {
    digitalWrite(LINE_FOLLOW_PIN, LOW);
    digitalWrite(CALIBRATION_PIN, LOW);
  }

  delay(10);
}
