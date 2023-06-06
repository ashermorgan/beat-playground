# Beat Playground
A rhythm game for the Adafruit Circuit Playground Express.

## Setup
1. Install the `Arduino SAMD` board profile and the `Adafruit Circuit Playground` and `AsyncDelay` libraries using the Arduino IDE
2. Compile and upload `./beat-playground` to your Circuit Playground Express

## Game Instructions
1. Use the left and right buttons to select a song (see list of songs below). The current selected song number will be displayed on the NeoPixels
2. "Tap" the air with the board (as if you're hitting a drum) to start the song
3. Tap along with the melody. Green lights indicate a correct tap and red lights indicate an incorrect (out of time) tap
4. When the song ends, your score (1-10) will be displayed on the NeoPixels
5. Tap or press either button to return to the song selection menu

## Songs Included
1. Twinkle Twinkle Little Star
2. Centuries (Fall Out Boy)
3. Blinding Lights (The Weeknd)
4. Counting Stars (OneRepublic)

## Important Constants
Declared at the top of `beat-playground/beat-playground.ino`

Name | Default | Description
--- | --- | ---
`ACCELERATION_THRESHOLD` | `20` | The minimum net acceleration for a tap to be registered (in m/s^2). Update to adjust the sensitivity of the tap detector.
`TIMING_THRESHOLD` | `100` | The maximum offset of a correct tap (in ms). Update to adjust the difficulty.
`SENSOR_REFRESH_PERIOD` | `50` | How often the program tries to detect a tap (in ms). This should be less than `TIMING_THRESHOLD`.
