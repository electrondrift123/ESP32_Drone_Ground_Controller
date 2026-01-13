#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64          // ← Changed to 64!
#define OLED_RESET     -1         // No reset pin
#define SCREEN_ADDRESS 0x3C       // Usually 0x3C; try 0x3D if blank

// Declare the display object as extern (defined in oled.cpp)
extern Adafruit_SSD1306 display;

void oled_init(void);

void oled_displayTelemetry(float roll, float pitch, float heading, float alt);
void oled_displayNoConnection(void);
void oled_displayCmd(float T, float Y, float P, float R);


#endif