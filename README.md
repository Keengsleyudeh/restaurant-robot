# Restaurant Robot (Arduino)

An Arduino-based line-following restaurant waiter robot that delivers food to tables, avoids obstacles with an ultrasonic sensor, and shows status on an I2C LCD.

## Features

- Line following with 3 track sensors (`left`, `mid`, `right`)
- Intersection-based navigation for table routes
- Obstacle detection and wait-until-clear behavior (ultrasonic sensor)
- Food delivery wait state using tray sensor
- Battery percentage display on LCD
- Serial/Bluetooth command control for table selection and manual movement

## Project File

- `restaurant_robot.ino`: Main firmware for the robot

## Hardware Used

- Arduino board (UNO/Nano compatible pin mapping in code)
- Motor driver (2-channel)
- 2 DC motors (left and right drive)
- Ultrasonic sensor (HC-SR04 style)
- 3 line tracking sensors
- Tray sensor
- I2C LCD (20x4, address `0x27`)
- Battery monitor input (analog)
- Optional Bluetooth serial module

## Pin Mapping

Defined in `restaurant_robot.ino`:

- Ultrasonic
  - `trigPin`: `12`
  - `echoPin`: `11`
- Track sensors
  - `left_track`: `A0`
  - `right_track`: `A1`
  - `mid_track`: `A2`
- Other sensors
  - `tray1`: `A3`
  - `battery`: `A4`
- Motors
  - `left_front`: `7`
  - `left_back`: `10`
  - `right_front`: `9`
  - `right_back`: `8`
  - `enA` (PWM): `6`
  - `enB` (PWM): `5`
- LED
  - `led`: `13`

## Serial Commands

Send these characters over Serial/Bluetooth (9600 baud):

- `1`: Deliver to Table 1
- `2`: Deliver to Table 2
- `3`: Deliver to Table 3 (currently placeholder route)
- `4`: Deliver to Table 4 (currently placeholder route)
- `A`: Manual nudge forward
- `B`: Manual nudge backward
- `C`: Turn left
- `D`: Turn right
- `E`: Stop

## How It Works

1. Robot waits in idle mode and continuously updates sensors.
2. It follows the line until it detects an intersection (`left=0`, `mid=0`, `right=0`).
3. At each route stage, it turns and continues to the next intersection.
4. On arrival, it waits for tray state change (food removed), then returns to base.
5. If an obstacle is detected (`distance < 60 cm`), robot stops and waits until path clears.

## Setup and Upload

1. Open `restaurant_robot.ino` in Arduino IDE.
2. Install required libraries:
   - `LiquidCrystal_I2C`
   - `Wire` (usually preinstalled)
3. Select your board and COM port.
4. Upload the sketch.
5. Open Serial Monitor at `9600` baud (or connect Bluetooth app) and send command characters.

## Calibration Notes

- Turn timing is currently fixed (`delay(1200)`) in `turn_left()` and `turn_right()`.
  - Tune this value based on wheel speed, floor friction, and robot geometry.
- Line sensor logic assumes:
  - `0 = black line`
  - `1 = white surface`
- Tray sensor logic currently assumes:
  - `0 = tray occupied`
  - `1 = tray removed`

## Current Limitations

- `table_3()` and `table_4()` currently reuse placeholder routes.
- Navigation uses timing-based turns rather than closed-loop angle control.

## Future Improvements

- Implement dedicated routes for Table 3 and Table 4
- Add PID line following for smoother movement
- Add battery low warning and auto-return behavior
- Add route confirmation on LCD before dispatch
