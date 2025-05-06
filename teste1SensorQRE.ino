#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <QTRSensors.h>

// ------------------- Configuração do OLED -------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// ------------------- Configuração do sensor -------------------
#define SENSOR_PIN 4     // PD4: OUT do QRE-8D
#define LED_ON_PIN 2     // PD2: LED ON do QRE-8D

QTRSensorsRC qtr;
unsigned char sensorPins[] = {SENSOR_PIN};
unsigned int sensorValues[1];

// ------------------- Calibração Manual -------------------
unsigned int minReflect = 1023;
unsigned int maxReflect = 0;
bool calibrating = true;
unsigned long calibrationStart = 0;

// ------------------- Setup -------------------
void setup() {
  pinMode(LED_ON_PIN, OUTPUT);
  digitalWrite(LED_ON_PIN, HIGH);  // Liga os emissores IR

  qtr.init(sensorPins, 1);  // Inicia o sensor

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    while (true);  // Travar se o OLED falhar
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Sensor QRE-8D"));
  display.println(F("Inicializado!"));
  display.display();
  delay(1500);

  calibrationStart = millis();  // Inicia contagem para calibração
}

// ------------------- Loop -------------------
void loop() {
  qtr.read(sensorValues);  // Lê o sensor
  int raw = sensorValues[0];

  if (calibrating) {
    // Atualiza min/max
    if (raw < minReflect) minReflect = raw;
    if (raw > maxReflect) maxReflect = raw;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("CALIBRANDO...");
    display.setCursor(0, 10);
    display.print("Atual: ");
    display.println(raw);
    display.setCursor(0, 20);
    display.print("Min: ");
    display.println(minReflect);
    display.setCursor(0, 30);
    display.print("Max: ");
    display.println(maxReflect);
    display.display();

    // Termina após 5 segundos
    if (millis() - calibrationStart > 5000) {
      calibrating = false;
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Calibracao");
      display.println("finalizada!");
      display.display();
      delay(1500);
    }

    return;
  }

  // Após calibração: normaliza valor
  int normalized = map(raw, minReflect, maxReflect, 0, 1000);
  normalized = constrain(normalized, 0, 1000);  // Garante que fique no intervalo

  // Exibe no OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Sensor:");
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print(normalized);
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.print("Raw: ");
  display.print(raw);
  display.display();

  delay(200);
}
