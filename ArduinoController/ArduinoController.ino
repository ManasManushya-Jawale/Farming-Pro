/* WRITER'S NOTE

*/

#include "OLED_Mods.h"
#include <U8g2lib.h>
#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>

#define SOIL_PIN A0

#define WATER_PUMP 8
#define PLANT_PUMP 9

#define MODE_L 6
#define MODE_R 7

#define TRIG_PIN 3
#define ECHO_PIN 4
#define ULTRASONIC_LIMIT 400          // max usable range in cm
#define ULTRASONIC_MIN 2              // min usable range in cm
#define ULTRASONIC_TIMEOUT_US 25000   // ~400cm round trip + margin
#define ULTRASONIC_SAMPLE_GAP_MS 60   // datasheet-recommended gap between pings

uint8_t display_mode = 0;

float last_water_reading = 1;
float current_water_reading = 1;
float water_level = 1;

bool helper = false;
bool speaking = false;
bool helperActive = false;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,
  U8X8_PIN_NONE);

DHT dht(5, DHT22);  // change to DHT11 on IRL uploading

void setup() {
  // Establish on-board serial communication speeds with the Linux kernel
  Serial.begin(115200);
  dht.begin();

  u8g2.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(MODE_L, INPUT_PULLUP);
  pinMode(MODE_R, INPUT_PULLUP);
  pinMode(WATER_PUMP, OUTPUT);
  pinMode(PLANT_PUMP, OUTPUT);
}

#define TANK_HEIGHT 10.0  // Tank height in cm

float get_distance() {
  // Send ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo time
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // No echo
  if (duration == 0) {
    return -1;
  }

  // Convert time to distance
  float distance = duration * 0.0343 / 2.0;

  return distance;
}

float get_water_level() {
  float distance = get_distance();

  if (distance < 0) {
    return -1;
  }

  // Distance from sensor to water surface
  float level = TANK_HEIGHT - distance;

  // Clamp the result
  level = constrain(level, 0.0, TANK_HEIGHT);

  // Convert to percentage
  float percentage = (level / TANK_HEIGHT) * 100.0;

  return percentage;
}

bool previous_L = LOW;
bool previous_R = LOW;

bool previous_M1 = LOW;
bool previous_M2 = LOW;

void read_Buttons() {

  bool current_L = digitalRead(MODE_L);

  if (previous_L == HIGH && current_L == LOW) {
    display_mode = display_mode > 0 ? (display_mode - 1) : 5;
  }

  previous_L = current_L;

  bool current_R = digitalRead(MODE_R);

  if (previous_R == HIGH && current_R == LOW) {
    display_mode = display_mode < 5 ? (display_mode + 1) : 0;
  }

  previous_R = current_R;

  bool current_M1 = digitalRead(WATER_PUMP);

  if (previous_M1 == HIGH && current_M1 == LOW) {
    digitalWrite(WATER_PUMP, HIGH);
  } else if (previous_M1 == LOW && current_M1 == HIGH) {
    digitalWrite(WATER_PUMP, HIGH);
  }

  previous_M1 = current_M1;

  bool current_M2 = digitalRead(PLANT_PUMP);

  if (previous_M2 == HIGH && current_M1 == LOW) {
    digitalWrite(PLANT_PUMP, HIGH);
  } else if (previous_M2 == LOW && current_M1 == HIGH) {
    digitalWrite(PLANT_PUMP, HIGH);
  }

  previous_M2 = current_M2;
}

int get_moisture() {
  int raw = analogRead(SOIL_PIN);
  Serial.print("RAW moisture: ");
  Serial.println(raw);
  return map(raw, 0, 1023, 255, 0);
  // return raw;
}

float temperature = 0, humidity = 0, moisture = 0;

void loop() {
  if (display_mode == 5) {
    if (!helperActive) {
      Serial.println("*ih");
      helperActive = true;
    }

    show_Helper(u8g2, speaking);
  } else {
    helperActive = false;

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    moisture = get_moisture();

    switch (display_mode) {
      case 0:
        show_Greeting(u8g2,
                      temperature < 10 || temperature > 35,
                      humidity < 20 || humidity > 90,
                      moisture < 20 || moisture > 90,
                      false);
        break;
      case 1:
        show_Temperature(u8g2, temperature);
        break;
      case 2:
        show_Humidity(u8g2, humidity);
        break;
      case 3:
        show_Moisture(u8g2, moisture);
        break;
      case 4:
        show_water_level(u8g2, get_water_level());
        break;
    }
  }
  
  Serial.print("H: ");
  Serial.println(humidity);

  Serial.print("M: ");
  Serial.println(moisture);

  Serial.print("T: ");
  Serial.println(temperature);

  read_Buttons();
}
