#include "DisplayManager.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

DHT dht(2, DHT22); // Przykład czujnika DHT22 podłączonego do pinu 2

void SensorManager::init() {
    dht.begin();
}

float SensorManager::readTemperature() {
    return dht.readTemperature();
}
