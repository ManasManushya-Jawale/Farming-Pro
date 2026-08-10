#ifndef OLED_CONFIG_H
#define OLED_CONFIG_H

#include <U8g2lib.h>

#define TEMP_BOUND_FROM -40
#define TEMP_BOUND_TO 80

#define HUMIDITY_BOUND_FROM 0
#define HUMIDITY_BOUND_TO 100

uint8_t warnings;

void show_bar(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display,
              uint8_t x, uint8_t y,
              float from, float to,
              float level) {
  const uint8_t width = 128 - x;
  const uint8_t height = 10;
  const uint8_t padding = 2;

  display.drawFrame(x, y, width, height);

  float percent = (level - from) / (to - from);
  percent = constrain(percent, 0.0f, 1.0f);

  uint8_t fillWidth =
    (width - 2 * padding) * percent;

  display.drawBox(
    x + padding,
    y + padding,
    fillWidth,
    height - 2 * padding);
}


void show_Greeting(
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display,
  bool temp, bool humid, bool moist, bool water_level) {
  display.clearBuffer();

  display.setFont(u8g2_font_6x10_tf);

  display.drawStr(40, 10, "Hi There!");


  if (temp || humid || moist || water_level) {
    int y = 20;
    if (temp) {
      display.drawStr(10, y, "TEMP HIGH/LOW");
      y += 10;
    }
    if (humid) {
      display.drawStr(10, y, "HUMIDITY EXTREME");
      y += 10;
    }
    if (moist) {
      display.drawStr(10, y, "EXTREME MOISTURE");
      y += 10;
    }
    if (water_level) {
      display.drawStr(10, y, "check the WELL");
    }
  } else {
    // Head
    display.drawCircle(64, 40, 20);

    // Eyes
    display.setDrawColor(1);
    display.drawCircle(64 - 10, 36, 4);
    display.drawCircle(64 + 10, 36, 4);

    // Mouth
    display.drawLine(56, 50, 72, 50);
  }
  display.sendBuffer();
}


void show_Temperature(
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display,
  float temperature) {
  display.clearBuffer();

  display.setFont(u8g2_font_6x10_tf);

  display.drawStr(0, 10, "Temperature:");

  display.setCursor(0, 25);
  display.print(temperature);
  display.print(" C");

  show_bar(
    display,
    0, 30,
    TEMP_BOUND_FROM,
    TEMP_BOUND_TO,
    temperature);

  display.sendBuffer();
}


void show_Humidity(
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display,
  float humidity) {
  display.clearBuffer();

  display.setFont(u8g2_font_6x10_tf);

  display.drawStr(0, 10, "Humidity:");

  display.setCursor(0, 25);
  display.print(humidity);
  display.print(" %");

  show_bar(
    display,
    0, 30,
    HUMIDITY_BOUND_FROM,
    HUMIDITY_BOUND_TO,
    humidity);

  display.sendBuffer();
}


void show_Moisture(
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display,
  float moisture) {
  display.clearBuffer();

  display.setFont(u8g2_font_6x10_tf);

  display.drawStr(0, 10, "Moisture:");

  display.setCursor(0, 25);
  display.print(moisture);
  display.print(" %");

  show_bar(
    display,
    0, 30,
    0,
    100,
    moisture);

  display.sendBuffer();
}


void show_Helper(
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display,
  bool speaking) {
  display.clearBuffer();

  display.setFont(u8g2_font_6x10_tf);

  display.drawStr(
    20,
    10,
    "AI helper, ask me anything");

  // Main circle
  display.drawDisc(64, 32, 16);

  // Inner cutout
  display.setDrawColor(0);
  display.drawDisc(64, 32, 10);

  display.setDrawColor(1);

  display.sendBuffer();
}

void show_water_level(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display,
                      float level) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(20, 10, "Water-Level in well");

  const uint8_t x = 0;
  const uint8_t y = 30;
  const uint8_t width = 40;
  const uint8_t height = 64 - y;
  const uint8_t padding = 2;

  display.drawFrame(x, y, width, height);

  // level = distance from sensor to water in cm
  float percent = level / 40;
  percent = constrain(percent, 0.0f, 1.0f);

  uint8_t fillHeight =
      (height - 2 * padding) * percent;

  display.drawBox(
      x + padding,
      y + height - padding - fillHeight,
      width - 2 * padding,
      fillHeight);

  display.sendBuffer();
}

#endif