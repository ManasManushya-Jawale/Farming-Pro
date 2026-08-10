import threading
import time

import sounddevice as sd
import serial
import pyttsx3

from faster_whisper import WhisperModel
from google import genai


# Serial
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)

# Whisper
model = WhisperModel(
    "small.en",
    device="cpu",
    compute_type="int8"
)

# Gemini
client = genai.Client()

# TTS
engine = pyttsx3.init()

recording = False

pyttsx3.speak("I will speak this text")

global temperature, humidity, moisture
temperature=0
humidity=0
moisture=0

def ask():
    global recording

    recording = True

    print("Recording...")

    audio = sd.rec(
        int(5 * 16000),
        samplerate=16000,
        channels=1,
        dtype="float32"
    )
    sd.wait()

    audio = audio.flatten()

    segments, _ = model.transcribe(audio)
    question = "".join(segment.text for segment in segments)

    print("You:", question)

    if "pump on" in question.lower():
        ser.write(b"*p\n")

    try:
        response = client.models.generate_content(
            model="gemini-3.5-flash",
            contents=f"""
Answer briefly as a professional agriculturist.
Use short sentences.
Keep the response to 1-3 sentences.
Avoid unnecessary explanations.
If asks anything related to 'my area',
answer his question with respect to the following values:
Temperature: {temperature}
Humidity: {humidity}
Soil moisture: {moisture}

User: {question}
""",
        )
        print("Helper:", response.text)
        pyttsx3.speak(response.text)

    except Exception as e:
        print(e)

    recording = False


def serial_loop():
    global temperature, humidity, moisture

    while True:
        line = ser.readline().decode(errors="ignore").strip()

        if line.startswith("H: "):
            humidity = float(line[3:])

        elif line.startswith("M: "):
            moisture = float(line[3:])

        elif line.startswith("T: "):
            temperature = float(line[3:])

        if line == "*ih" and not recording:
            threading.Thread(
                target=ask,
                daemon=True
            ).start()


threading.Thread(
    target=serial_loop
).start()

while True:
    time.sleep(1)