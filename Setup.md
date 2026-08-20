In order to make the circuit yourself, follow the given steps:
1. Make the following circuit:
<img src="image/Circuit.png">
2. Open ArduinoController in arduino IDE and upload `ArduinoController.ino`.
3. Make sure you have both `ArduinoController.ino` and `OLED_Mods.h` opened up in your IDE's same window
4. upload the Arduino code and **close the arduino IDE**. This is a crucial step.
5. Export your API key using:
    * `$env:API_KEY="your_api_key_here"` on windows.
    * `export API_KEY="your_api_key_here"` in linux.
6. Run the [Transcriber](/Transcriber.py)