## Simple Electronic Load

# Authors
- Ziyad Eslam
- Omar Ramadan
- Mostafa Khashan
- Retaj Tarek
- Rowan Ashraf

**Description:**

This is a simple electronic load designed for testing power supplies and batteries combined with a current and voltage meter (2in1) to get realtime measurement
on the current and voltages to get a better experience and better ability to apply control system as PID system.
It can sink up to 5A of current (software limited) and input voltage up to 30V.

**Features:**

* Constant current control using PID feedback loop
* Power calculation
* Setpoint adjustment via buttons
* LCD display for displaying measured values and setpoint
* Voltage and current measurement

**Specifications:**
| spec | range |
| ---- | ---- |
| Input voltage range | 5V - 30V |
| Output current range | 0A - 5A |
| Power handling capacity | 60W (TESTED) | 
| Measurement accuracy | ±1% |
| Interface | ATMEGA8 for controlling current draw, PID and LCD for feedback |

**Software:**

The electronic load is controlled through AVR controller through dedicated push buttons for increasing and decreasing current draw by adjusting the PWM signal 
that we then pass through RC filter to get its average that is between 0V and 5V. Also, the current & voltages feedback are output on the LCD.

**Hardware:**
* ATmega8 microcontroller
* LCD display
* DAC converter
* Current sense resistor
* PWM output for controlling load

**Usage:**
1. Assemble the electronic load according to the included instructions.
2. Connect the power supply to the input terminals.
3. Connect the device under test (DUT) to the output terminals.
4. Inspect the data through the LCD.


**Additional Information:**

* For troubleshooting tips, please refer to the repo files hardware and software files are all included.
* The hardware used are picked for both avalibilty and functionality .
* Demo video is presented under Demo.mp4 in the img section in the repo
