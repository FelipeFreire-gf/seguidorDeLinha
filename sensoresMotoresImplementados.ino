#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <QTRSensors.h>
#include <OrangutanMotors.h>
#include <OrangutanLEDs.h>

// ------------------- OLED -------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// ------------------- QTR -------------------
#define LED_ON_PIN 2
QTRSensorsRC qtr;
unsigned char sensorPins[] = {1, 4, 7, 8, 9, 10, A0, A1};
const uint8_t NUM_SENSORS = sizeof(sensorPins);
unsigned int sensorValues[NUM_SENSORS];

// ------------------- Calibração -------------------
unsigned int minReflect[NUM_SENSORS];
unsigned int maxReflect[NUM_SENSORS];
bool calibrating = true;
unsigned long calibrationStart = 0;

// ------------------- Orangutan -------------------
OrangutanMotors motors;
OrangutanLEDs leds;

// ------------------- PID -------------------
const float Kp = 0.042;             // Valor dado por baseSpeed(150)/3500
const float Kd = 0.5;               // Valor que há de ser ajustado ainda
double lastError = 0;               // Primeira inicialização do ultimo erro é 0

// ------------------- Setup -------------------
void setup() {
  pinMode(LED_ON_PIN, OUTPUT);
  digitalWrite(LED_ON_PIN, HIGH);
  qtr.init(sensorPins, NUM_SENSORS);

  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    minReflect[i] = 1023;
    maxReflect[i] = 0;
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("QRE-8D x8 Inicializando"));
  display.display();
  delay(1500);

  calibrationStart = millis();
}

// ------------------- Loop -------------------
void loop() {
  qtr.read(sensorValues);

  if (calibrating) {
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
      if (sensorValues[i] < minReflect[i]) minReflect[i] = sensorValues[i];
      if (sensorValues[i] > maxReflect[i]) maxReflect[i] = sensorValues[i];
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("CALIBRANDO...");
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
      display.setCursor(0, 10 + i * 6);
      display.print("S");
      display.print(i);
      display.print(": ");
      display.print(sensorValues[i]);
    }
    display.display();

    if (millis() - calibrationStart > 5000) {
      calibrating = false;
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Calibracao OK");
      display.display();
      delay(1500);
    }
    return;
  }

  // Normaliza os sensores e calcula posição
  int weightedSum = 0;
  int sum = 0;
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    int value = map(sensorValues[i], minReflect[i], maxReflect[i], 0, 1000);
    value = constrain(value, 0, 1000);
    weightedSum += value * i * 1000;
    sum += value;
  }

  int position = (sum != 0) ? weightedSum / sum : 3500;

  // Controle PID
  int error = position - 3500;
  int turn = error * Kp + (error - lastError) * Kd ;          // Computagem do ajustamento
  lastError = error;                                          // Armazenamento do erro

  int baseSpeed = 150;
  int leftSpeed = baseSpeed + turn;
  int rightSpeed = baseSpeed - turn;

  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  motors.setSpeeds(leftSpeed, rightSpeed);  // Aqui usamos OrangutanMotors

  // Exibe no display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Pos: ");
  display.println(position);
  display.setCursor(0, 10);
  display.print("Err: ");
  display.println(error);
  display.setCursor(0, 20);
  display.print("L: ");
  display.print(leftSpeed);
  display.print(" R: ");
  display.println(rightSpeed);
  display.display();

  delay(50);
}
