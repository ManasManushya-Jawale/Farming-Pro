
# Requirements
## Dependencies
The dependencies of this project are:
* python
* arduino-ide
* 1× Arduino Uno Q
* 1× HC-SR04 ultrasonic distance sensor
* 1× soil-moisture sensor
* 1× DHT22 temperature/humidity sensor
* 1× 0.96" I²C OLED display (SSD1306)
* 2× motor/pump modules — shown as Motor 1 and Motor 2
* 4× push buttons
* 1× breadboard
* Jumper wires
* USB cable / suitable power supply for Arduino Uno

## Calibration
The project uses some hardcoded values that you need to change before using it. To do so,  follow the following steps:
* Change the line no. 18 of the `Transriber.py` to the serial port that your Arduino is connected to.
* Calibrate the `DRY_SOIL` (line 31) and `WET_SOIL` (line 32) values with the amounts that you think are good for your surrioundings.
* Set the value of `WELL_HEIGHT` (line 29) in arduino code with the height of your well

## Prequisties
In order to use this in your own circuit, first follow the following steps:
1. Copy the git HTTPs URL of this repository
2. Clone this repo using `git clone <repository-link>`
3. Create a venv in the repository folder using `python -m venv .venv`
4. Activate the venv using:
    * `source .venv/bin/activate` if you are using bash
    * `.venv\Scripts\activate.bat` for command prompt 
    * `.venv\Scripts\activate.ps1` for powershell.
5. Install the dependencies using `pip install -r requirements.txt`, this installs all the required python dependencies needed.

Along with that, you would also need to give a Google gemini API key to the program, so remember to have an API key in your [google AI studio](https://aistudio.google.com).
