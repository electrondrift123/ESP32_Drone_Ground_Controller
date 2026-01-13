#include "oled.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void oled_init(void){
  Wire.begin(21, 22);  // Your ESP32 pins: SDA=21, SCL=22
  delay(100);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println(F("LyGAPID"));
  display.println(F("by: Ivan"));
  display.display();

  delay(1000);
}

void oled_displayTelemetry(float roll, float pitch, float heading, float alt){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(F("R: ")); display.println(roll, 2);
    display.print(F("P: ")); display.println(pitch, 2);
    display.print(F("Y: ")); display.println(heading, 2);
    display.print(F("h: ")); display.print(alt, 1); display.println(F(" m"));
    display.display();
}
void oled_displayNoConnection(void){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("No Connection!!!"));
    display.display();
}
void oled_displayCmd(float T, float Y, float P, float R){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(F("T: ")); display.print(T, 0); display.println(F(" %"));
    display.print(F("Y: ")); display.println(Y, 2);
    display.print(F("P: ")); display.println(P, 2);
    display.print(F("R: ")); display.println(R, 2);
    display.display();
}