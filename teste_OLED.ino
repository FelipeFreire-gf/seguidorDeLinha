#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void setup() {
  // Inicializa comunicação serial apenas se necessário para depuração
  // Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    // Serial.println(F("Falha ao iniciar display OLED"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Teste OLED 0.96\""));
  display.println(F("SSD1306 c/ Orangutan"));
  display.display();
  delay(2000);

  for (int i = 0; i <= 10; i++) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Contando:");
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.println(i);
    display.display();
    delay(1000);
  }
}

void loop() {
  // Nada no loop
}
