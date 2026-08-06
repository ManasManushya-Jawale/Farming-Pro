import threading

recording = False

def ask():
    global recording

    recording = True

    print("Recording...")
    audio = sd.rec(int(5 * 16000), samplerate=16000,
                   channels=1, dtype="float32")
    sd.wait()

    audio = audio.flatten()

    segments, _ = model.transcribe(audio)
    question = "".join(segment.text for segment in segments)

    print("You:", question)

    if "pump on" in question.lower():
        ser.write(b"*p\n")

    try:
        response = client.models.generate_content(
            model="gemini-2.5-flash",
            contents=question,
        )
        print("Helper:", response.text)
    except Exception as e:
        print(e)

    recording = False


def serial_loop():
    while True:
        line = ser.readline().decode().strip()

        if line == "*ih" and not recording:
            threading.Thread(target=ask, daemon=True).start()


threading.Thread(target=serial_loop, daemon=True).start()

while True:
    time.sleep(1)