
#ifndef OLED_CONFIG_H
#define OLED_CONFIG_H
#include <Adafruit_SSD1306.h>

#define ENABLE_OLED

#define TEMP_BOUND_FROM -40
#define TEMP_BOUND_TO 80

#define HUMIDITY_BOUND_FROM 0
#define HUMIDITY_BOUND_TO 100

void show_bar(Adafruit_SSD1306& display,
              uint8_t x, uint8_t y,
              float from, float to,
              float value)
{
    const uint8_t width = 128 - x;
    const uint8_t height = 10;
    const uint8_t padding = 2;

    display.drawRect(x, y, width, height, WHITE);

    float percent = (value - from) / (to - from);
    percent = constrain(percent, 0.0f, 1.0f);

    uint8_t fillWidth = (width - 2 * padding) * percent;

    display.fillRect(x + padding,
                     y + padding,
                     fillWidth,
                     height - 2 * padding,
                     WHITE);
}

void show_Greeting(Adafruit_SSD1306& display) {
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(40, 5);
  display.print("Hi There!");

  display.fillCircle(64, 40, 22, WHITE);
  display.fillCircle(64, 40, 20, BLACK);

  display.drawCircle(64 + 14, 40, 5, WHITE);
  display.drawCircle(64 - 14, 40, 5, WHITE);

  display.drawLine(64 + 8, 50, 64 - 8, 50, WHITE);

  display.display();
}

void show_Temperature(Adafruit_SSD1306& display, float temperature) {
  display.clearDisplay();

  display.setTextSize(1.5);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Temperature:");

  display.setCursor(0, 15);
  display.print(temperature);
  display.print(" C");

  show_bar(display, 0, 30, TEMP_BOUND_FROM, TEMP_BOUND_TO, temperature);

  display.display();
}

void show_Humidity(Adafruit_SSD1306& display, float humidity) {
  display.clearDisplay();

  display.setTextSize(1.5);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Humidity:");

  display.setCursor(0, 15);
  display.print(humidity);
  display.print(" %");

  show_bar(display, 0, 30, HUMIDITY_BOUND_FROM, HUMIDITY_BOUND_TO, humidity);

  display.display();
}

void show_Moisture(Adafruit_SSD1306& display, float moisture) {
  display.clearDisplay();

  display.setTextSize(1.5);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Moisture:");

  display.setCursor(0, 15);
  display.print(moisture);
  display.print(" %");

  show_bar(display, 0, 30, 0, 100, moisture);

  display.display();
}

static uint8_t size = 0;

void show_Helper(Adafruit_SSD1306& display, bool speaking) {
  display.clearDisplay();

  display.fillCircle(64, 32, 16, WHITE);
  display.fillCircle(64, 32, 10, BLACK);

  display.setCursor(20, 5);
  display.print("AI helper, ask me anything");

  display.display();
}

#endif