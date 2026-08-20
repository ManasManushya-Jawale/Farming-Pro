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

#define WATER_PUMP_BUTTON 10
#define PLANT_PUMP_BUTTON 11

#define MODE_L 6
#define MODE_R 7

#define TRIG_PIN 3
#define ECHO_PIN 4
#define ULTRASONIC_LIMIT 400
#define ULTRASONIC_MIN 2
#define ULTRASONIC_TIMEOUT_US 25000
#define ULTRASONIC_SAMPLE_GAP_MS 60

#define WELL_HEIGHT 7.5

#define DRY_SOIL 20
#define WET_SOIL 80

uint8_t display_mode = 0;

float last_water_reading = 1;
float current_water_reading = 1;
float water_level = 1;

bool helper = false;
bool speaking = false;
bool helperActive = false;

bool waterPumpState = false;
bool plantPumpState = false;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,
  U8X8_PIN_NONE);

DHT dht(5, DHT22);  // Change to DHT11 when uploading IRL

bool previous_L = HIGH;
bool previous_R = HIGH;

bool previous_M1 = HIGH;
bool previous_M2 = HIGH;

float temperature = 0;
float humidity = 0;
float moisture = 0;


void setup() {
  // Establish on-board serial communication with the Linux kernel
  Serial.begin(115200);

  dht.begin();
  u8g2.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(MODE_L, INPUT_PULLUP);
  pinMode(MODE_R, INPUT_PULLUP);

  pinMode(WATER_PUMP_BUTTON, INPUT_PULLUP);
  pinMode(PLANT_PUMP_BUTTON, INPUT_PULLUP);

  pinMode(WATER_PUMP, OUTPUT);
  pinMode(PLANT_PUMP, OUTPUT);
}


float get_distance() {
  // Send ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo time
  unsigned long duration = pulseIn(
    ECHO_PIN,
    HIGH,
    ULTRASONIC_TIMEOUT_US);

  // No echo
  if (duration == 0) {
    return -1;
  }

  // Convert time to distance in cm
  float distance = duration * 0.0343 / 2.0;

  // Ignore invalid measurements
  if (distance < ULTRASONIC_MIN || distance > ULTRASONIC_LIMIT) {
    return -1;
  }

  return distance;
}


float get_water_level() {
  float distance = get_distance();

  if (distance < 0) {
    return -1;
  }

  // Distance from sensor to water surface
  float level = WELL_HEIGHT - distance;

  // Clamp the result
  level = constrain(level, 0.0, WELL_HEIGHT);

  // Convert to percentage
  float percentage = (level / WELL_HEIGHT) * 100.0;

  return percentage;
}


void read_Buttons() {

  // -------------------------
  // Display mode - Left
  // -------------------------

  bool current_L = digitalRead(MODE_L);

  if (previous_L == HIGH && current_L == LOW) {
    display_mode = display_mode > 0
                     ? display_mode - 1
                     : 5;
  }

  previous_L = current_L;


  // -------------------------
  // Display mode - Right
  // -------------------------

  bool current_R = digitalRead(MODE_R);

  if (previous_R == HIGH && current_R == LOW) {
    display_mode = display_mode < 5
                     ? display_mode + 1
                     : 0;
  }

  previous_R = current_R;


  // -------------------------
  // Water pump button
  // -------------------------

  bool current_M1 = digitalRead(WATER_PUMP_BUTTON);

  if (previous_M1 == HIGH && current_M1 == LOW) {
    waterPumpState = !waterPumpState;

    digitalWrite(
      WATER_PUMP,
      waterPumpState ? HIGH : LOW);
  }

  previous_M1 = current_M1;


  // -------------------------
  // Plant pump button
  // -------------------------

  bool current_M2 = digitalRead(PLANT_PUMP_BUTTON);

  if (previous_M2 == HIGH && current_M2 == LOW) {
    plantPumpState = !plantPumpState;

    digitalWrite(
      PLANT_PUMP,
      plantPumpState ? HIGH : LOW);
  }

  previous_M2 = current_M2;


  // -------------------------
  // Linux / Serial commands
  // -------------------------

  if (Serial.available()) {

    String data = Serial.readStringUntil('\n');
    data.trim();

    // Water pump
    if (data == "M1: t") {
      waterPumpState = true;
      digitalWrite(WATER_PUMP, HIGH);
    } else if (data == "M1: f") {
      waterPumpState = false;
      digitalWrite(WATER_PUMP, LOW);
    }

    // Plant pump
    else if (data == "M2: t") {
      plantPumpState = true;
      digitalWrite(PLANT_PUMP, HIGH);
    } else if (data == "M2: f") {
      plantPumpState = false;
      digitalWrite(PLANT_PUMP, LOW);
    }
  }
}


int get_moisture() {
  int raw = analogRead(SOIL_PIN);
  int moisture = map(raw, 1023, 200, 0, 100);
  moisture = constrain(moisture, 0, 100);

  // UNO Q / 12-bit ADC
  return moisture;
}


void loop() {

  // -------------------------
  // Helper screen
  // -------------------------

  if (display_mode == 5) {

    if (!helperActive) {
      Serial.println("*ih");
      helperActive = true;
    }

    show_Helper(u8g2, speaking);

  } else {

    helperActive = false;

    // Read sensors
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    moisture = get_moisture();


    // -------------------------
    // Display
    // -------------------------

    switch (display_mode) {

      case 0:
        show_Greeting(
          u8g2,
          temperature < 10 || temperature > 35,
          humidity < 20 || humidity > 90,
          moisture < DRY_SOIL || moisture > WET_SOIL,
          false);
        break;


      case 1:
        show_Temperature(
          u8g2,
          temperature);
        break;


      case 2:
        show_Humidity(
          u8g2,
          humidity);
        break;


      case 3:
        show_Moisture(
          u8g2,
          moisture);
        break;


      case 4:
        show_water_level(
          u8g2,
          get_water_level());
        break;
    }
  }


  // -------------------------
  // Send sensor data to Linux
  // -------------------------

  Serial.print("H: ");
  Serial.println(humidity);

  Serial.print("M: ");
  Serial.println(moisture);

  Serial.print("T: ");
  Serial.println(temperature);


  // Handle buttons and commands
  read_Buttons();
}
