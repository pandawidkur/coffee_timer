#ifndef CONFIG_H
#define CONFIG_H

#define GREEN_BUTTON = 2;
#define YELLOW_BUTTON = 3;
#define RED_BUTTON = 4;
#define GREEN_LED = 5;
#define YELLOW_LED = 6;
#define RED_LED = 7;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#endif
