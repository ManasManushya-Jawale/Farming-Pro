import threading
import time
import re

import sounddevice as sd
import serial
import pyttsx3

from faster_whisper import WhisperModel
from google import genai


# =========================
# Serial
# =========================

ser = serial.Serial(
    "/dev/ttyACM0",
    115200,
    timeout=1
)


# =========================
# Whisper
# =========================

model = WhisperModel(
    "small.en",
    device="cpu",
    compute_type="int8"
)


# =========================
# Gemini
# =========================

client = genai.Client()


# =========================
# Text-to-Speech
# =========================

engine = pyttsx3.init()
engine.setProperty("rate", 140)


# =========================
# Global state
# =========================

recording = False

temperature = 0
humidity = 0
moisture = 0


# =========================
# Startup message
# =========================

engine.say("I will speak this text")
engine.runAndWait()


# =========================
# Voice assistant
# =========================

def ask():
    global recording

    recording = True

    print("Recording...")

    try:
        # Record microphone
        audio = sd.rec(
            int(5 * 16000),
            samplerate=16000,
            channels=1,
            dtype="float32"
        )

        sd.wait()

        audio = audio.flatten()


        # =========================
        # Speech recognition
        # =========================

        segments, _ = model.transcribe(audio)

        question = "".join(
            segment.text for segment in segments
        ).strip()

        print("You:", question)

        lower_question = question.lower()


        # =========================
        # Detect motor commands
        # =========================

        motor_number = None
        state = None

# Normalize common speech-recognition variations
        text = lower_question.replace("-", " ")

# Convert spoken numbers to digits
        number_words = {
            "one": "1",
            "first": "1",
            "second": "2",
            "two": "2",
            "too": "2",
            "to": "2",
        }

        for word, number in number_words.items():
            text = re.sub(rf"\b{word}\b", number, text)

# Whisper sometimes hears "off" as "of"
        text = re.sub(r"\bturn\s+of\b", "turn off", text)

# Remove optional filler words
        text = re.sub(r"\b(the|number)\b", "", text)

# ---------------------------------
# Examples:
# turn on motor 1
# turn off motor 2
# turn motor 1 on
# motor 2 off
# pump 1 on
# ---------------------------------

        match = re.search(
            r"\b(?:turn\s+)?"
            r"(on|off)\s+"
            r"(?:motor|pump)\s+"
            r"(\d+)\b",
            text
        )

        if match:
            state = match.group(1)
            motor_number = int(match.group(2))

        else:
            match = re.search(
                r"\b(?:turn\s+)?"
                r"(?:motor|pump)\s+"
                r"(\d+)\s+"
                r"(on|off)\b",
                text
            )

            if match:
                motor_number = int(match.group(1))
                state = match.group(2)


# =========================
# Send motor command
# =========================

        if motor_number is not None:

            if motor_number in (1, 2):

                command = f"M{motor_number}: {'t' if state == 'on' else 'f'}"

                print("Detected motor command:", command)

                ser.write((command + "\n").encode())

                return

            else:
                print("Invalid motor number:", motor_number)
                return


        # =========================
        # Gemini
        # =========================

        response = client.models.generate_content(
            model="gemini-3.5-flash",
            contents=f"""
Answer briefly as a professional agriculturist.

Use short sentences.
Keep the response to 1-3 sentences.
Avoid unnecessary explanations.

If anything related to "my area" is asked,
use these sensor values:

Temperature: {temperature}
Humidity: {humidity}
Soil moisture: {moisture}

User: {question}
"""
        )

        print("Helper:", response.text)


        # =========================
        # Speak response
        # =========================

        engine.say(response.text)
        engine.runAndWait()


    except Exception as e:

        print("Error:", e)


    finally:

        recording = False


# =========================
# Serial reader
# =========================

def serial_loop():

    global temperature
    global humidity
    global moisture

    while True:

        line = ser.readline().decode(
            errors="ignore"
        ).strip()


        if not line:
            continue


        # =========================
        # Sensor values
        # =========================

        if line.startswith("H: "):

            try:
                humidity = float(line[3:])
            except ValueError:
                pass


        elif line.startswith("M: "):

            try:
                moisture = float(line[3:])
            except ValueError:
                pass


        elif line.startswith("T: "):

            try:
                temperature = float(line[3:])
            except ValueError:
                pass


        # =========================
        # Start voice assistant
        # =========================

        elif line == "*ih" and not recording:

            threading.Thread(
                target=ask,
                daemon=True
            ).start()


# =========================
# Start serial thread
# =========================

threading.Thread(
    target=serial_loop,
    daemon=True
).start()


# =========================
# Keep program alive
# =========================

while True:
    time.sleep(1)
