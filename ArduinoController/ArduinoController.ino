/* WRITER'S NOTE

*/

#include "OLED_Mods.h"
#include <U8g2lib.h>
#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>

#define SPEAKER_PIN 0  // Must be a supported PWM pin
#define SOIL_PIN A0

#define WATER_PUMP_IN 8
#define WATER_PUMP_OUT 9

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
  u8g2.clearBuffer();
  u8g2.drawBox(20, 20, 20, 20);
  u8g2.sendBuffer();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);

  pinMode(MODE_L, INPUT_PULLUP);
  pinMode(MODE_R, INPUT_PULLUP);
  pinMode(WATER_PUMP_IN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
}


float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  int duration = pulseIn(ECHO_PIN, HIGH, ULTRASONIC_TIMEOUT_US);

  if (duration == 0)
    return -1;  // no echo received (out of range / timeout)

  return duration * 0.034f / 2.0f;
}

float get_stable_distance() {
  const int samples = 7;
  float readings[samples];
  int valid = 0;

  for (int i = 0; i < samples; i++) {
    float reading = readDistanceCM();

    if (reading > ULTRASONIC_MIN && reading < ULTRASONIC_LIMIT) {
      readings[valid++] = reading;
    }

    delay(ULTRASONIC_SAMPLE_GAP_MS);
  }

  if (valid == 0)
    return -1;  // signal "no reliable reading" instead of 0

  // Sort
  for (int i = 0; i < valid - 1; i++) {
    for (int j = i + 1; j < valid; j++) {
      if (readings[j] < readings[i]) {
        float temp = readings[i];
        readings[i] = readings[j];
        readings[j] = temp;
      }
    }
  }

  // Median
  return readings[valid / 2];
}

bool previous_L = LOW;
bool previous_R = LOW;

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
}

int get_moisture() {
  int raw = analogRead(SOIL_PIN);
  return map(raw, 0, 4095, 0, 100);
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
        show_water_level(u8g2, get_stable_distance());
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
