/* WRITER'S NOTE

*/

#include "OLED_Mods.h"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DHT.h>
#include <TTS.h>
#include <Wire.h>

#define SPEAKER_PIN 0 // Must be a supported PWM pin
#define SOIL_PIN A0

#define WATER_PUMP_PIN 8

#define MODE_L 6
#define MODE_R 7

#define TRIG_PIN 3
#define ECHO_PIN 4
#define ULTRASONIC_LIMIT 400

uint8_t display_mode = 0;

float last_water_reading = 1;
float current_water_reading = 1;
float water_level = 1;

bool helper = false;
bool speaking = false;
bool helperActive = false;

Adafruit_SSD1306 screen(128, 64, &Wire);
TTS tts(SPEAKER_PIN);
DHT dht(5, DHT22); // change to DHT11 on IRL uploading

void setup() {
  // Establish on-board serial communication speeds with the Linux kernel
  Serial.begin(115200);
  dht.begin();
  screen.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  screen.clearDisplay();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(MODE_L, INPUT_PULLUP);
  pinMode(MODE_R, INPUT_PULLUP);
  pinMode(WATER_PUMP_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  digitalWrite(WATER_PUMP_PIN,
               HIGH); // Default: Keeps active-low relay safe/OFF
}

float get_distance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  int duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.0343 / 2;
}

void process_distance_input() {

  float reading = get_distance();

  if (reading != 0) {
    current_water_reading = reading;
    last_water_reading = reading;
  } else {
    current_water_reading = last_water_reading;
  }
  water_level = ULTRASONIC_LIMIT - current_water_reading;
}

bool previous_L = LOW;
bool previous_R = LOW;

void read_Buttons() {

  bool current_L = digitalRead(MODE_L);

  if (previous_L == HIGH && current_L == LOW) {
    display_mode = display_mode > 0 ? (display_mode - 1) : 4;
  }

  previous_L = current_L;

  bool current_R = digitalRead(MODE_R);

  if (previous_R == HIGH && current_R == LOW) {
    display_mode = display_mode < 4 ? (display_mode + 1) : 0;
  }

  previous_R = current_R;
}

void handle_speaker() {
  if (!Serial.available() && !helper)
    return;

  String text = Serial.readStringUntil('\n');
  text.trim();

  if (text.length() <= 0)
    return;

  if (text == "*s") {
    speaking = !speaking;
    return;
  }

  if (text == "*p") {
    digitalWrite(WATER_PUMP_PIN, HIGH);
  }

  if (text[0] == '*')
    return;

  Serial.println("*LOG: Saying");
  tts.sayText(text.c_str());
}

float get_moisture() {
  int raw = analogRead(SOIL_PIN);
  int moisture = map(raw, 820, 340, 0, 100);
  moisture = constrain(moisture, 0, 100);
  return moisture;
}

void loop() {
  if (display_mode == 4) {
    if (!helperActive) {
      Serial.println("*ih");
      helperActive = true;
    }

    show_Helper(screen, speaking);
  } else {
    helperActive = false;

    switch (display_mode) {
    case 0:
      show_Greeting(screen);
      break;
    case 1:
      show_Temperature(screen, dht.readTemperature());
      break;
    case 2:
      show_Humidity(screen, dht.readHumidity());
      break;
    case 3:
      show_Moisture(screen, get_moisture());
      break;
    }
  }

  read_Buttons();
  handle_speaker();
  process_distance_input();
}
