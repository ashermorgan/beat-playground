#include <Adafruit_CircuitPlayground.h>
#include <AsyncDelay.h>

const int A4_HZ = 440;
const int BUTTON_1_PIN = 4;
const int BUTTON_2_PIN = 5;
const float ACCELERATION_THRESHOLD = 20;
const float TIMING_THRESHOLD = 100;
const int SENSOR_REFRESH_PERIOD = 50;

int selectedSong = 0;   // The index of the selected song
int currentNote;        // The index of the current note
int currentBeat;        // The number of beats left in the current note
int correctTaps;        // The number of correct user taps
int incorrectTaps;      // The number of incorrect user taps
bool tapInProgress;     // Whether a user tap was detected on the last cycle

enum GameStatus { HomeScreen, InProgress, EndScreen };
GameStatus gameStatus = HomeScreen;

AsyncDelay blinkDelay, playbackDelay, sensorDelay;

bool button1flag = false;
bool button2flag = false;

float midi[129];
void generateMIDI() {
    // Generate MIDI pitch data
    // Adapted from https://subsynth.sourceforge.net/midinote2freq.html
    for (int x = 0; x < 127; ++x) {
        midi[x] = (A4_HZ / 32.0) * pow(2.0, ((x - 9.0) / 12.0));
    }
    midi[128] = 30;
    midi[129] = 1;
}

enum Pitch {
    PitchC4  = 60,
    PitchDb4 = 61,
    PitchD4  = 62,
    PitchEb4 = 63,
    PitchE4  = 64,
    PitchF4  = 65,
    PitchGb4 = 66,
    PitchG4  = 67,
    PitchAb4 = 68,
    PitchA4  = 69,
    PitchBb4 = 70,
    PitchB4  = 71,
    PitchC5  = 72,
    PitchDb5 = 73,
    PitchD5  = 74,
    PitchEb5 = 75,
    PitchE5  = 76,
    PitchF5  = 77,
    PitchGb5 = 78,
    PitchG5  = 79,
    PitchAb5 = 80,
    PitchA5  = 81,
    PitchBb5 = 82,
    PitchB5  = 83,
    PitchBeat = 128,
    PitchRest = 129,
};
struct Note {
    Pitch pitch;
    int duration;
};
struct Song {
    int tempo;
    int len;
    int countoff;
    int rests;
    Note *notes;
};

const Song songs[] = {
    {
        // Twinkle Twinkle Little Star
        500, 18, 4, 0, new Note[18] {
            { PitchBeat, 1 }, { PitchBeat, 1 }, { PitchBeat, 1 }, { PitchBeat, 1 },

            { PitchC4, 1 }, { PitchC4, 1 }, { PitchG4, 1 }, { PitchG4, 1 },
            { PitchA4, 1 }, { PitchA4, 1 }, { PitchG4, 2 },
            { PitchF4, 1 }, { PitchF4, 1 }, { PitchE4, 1 }, { PitchE4, 1 },
            { PitchD4, 1 }, { PitchD4, 1 }, { PitchC4, 2 },
        }
    }, {
        // Centuries
        170, 31, 3, 1, new Note[31] {
            { PitchBeat, 4 }, { PitchBeat, 4 }, { PitchBeat, 4 },

            { PitchE4, 2 }, { PitchGb4, 2 },
            { PitchG4, 2 }, { PitchGb4, 2 }, { PitchE4, 2 }, { PitchE4, 4 },
            { PitchE4, 2 }, { PitchE4, 2 },  { PitchE4, 2 },
            { PitchG4, 2 }, { PitchGb4, 2 }, { PitchE4, 2 }, { PitchE4, 4 },
            { PitchE4, 2 }, { PitchE4, 2 },  { PitchD4, 2 },
            { PitchE4, 2 }, { PitchB4, 1 },  { PitchB4, 9 }, { PitchRest, 2 }, { PitchD4, 2 },
            { PitchE4, 2 }, { PitchB4, 1 },  { PitchB4, 3 }, { PitchD4, 2 },
            { PitchE4, 2 }, { PitchB4, 1 },  { PitchB4, 3 },
        }
    }, {
        // Blinding Lights
        175, 23, 4, 0, new Note[23] {
            { PitchBeat, 2 }, { PitchBeat, 2 }, { PitchBeat, 2 }, { PitchBeat, 2 },

            { PitchF4, 13 }, { PitchF4, 1 },  { PitchG4, 2 },
            { PitchF4, 2 },  { PitchEb4, 1 }, { PitchEb4, 2 }, { PitchC4, 1 },
            { PitchEb4, 7 }, { PitchEb4, 3 },
            { PitchBb4, 1 }, { PitchG4, 2 },  { PitchF4, 2 },  { PitchEb4, 3 },
            { PitchBb4, 1 }, { PitchG4, 2 },  { PitchF4, 2 },  { PitchEb4, 1 }, { PitchF4, 4 },
            { PitchF4, 2 },
        }
    }, {
        // Counting Stars
        125, 46, 4, 2, new Note[46] {
            { PitchBeat, 4 }, { PitchBeat, 4 }, { PitchBeat, 4 }, { PitchBeat, 4 },

            { PitchGb4, 4 },  { PitchAb4, 4 }, { PitchB4, 4 },  { PitchAb4, 4 },
            { PitchGb4, 2 },  { PitchAb4, 2 }, { PitchGb4, 1 }, { PitchE4, 2 },  { PitchAb4, 9 },
            { PitchRest, 2 }, { PitchGb4, 2 }, { PitchGb4, 2 }, { PitchAb4, 1 }, { PitchA4, 3 },
            { PitchAb4, 2 },  { PitchGb4, 2 }, { PitchE4, 2 },
            { PitchAb4, 4 },  { PitchDb4, 4 }, { PitchE4, 8 },

            { PitchGb4, 4 },  { PitchAb4, 4 }, { PitchB4, 4 },  { PitchAb4, 4 },
            { PitchGb4, 2 },  { PitchAb4, 2 }, { PitchGb4, 1 }, { PitchE4, 2 },  { PitchAb4, 9 },
            { PitchRest, 2 }, { PitchGb4, 2 }, { PitchGb4, 2 }, { PitchAb4, 2 },
            { PitchA4, 2 },   { PitchAb4, 2 }, { PitchGb4, 1 }, { PitchE4, 3 },
            { PitchAb4, 2 },  { PitchGb4, 2 }, { PitchGb4, 2 }, { PitchDb4, 2 }, { PitchE4, 8 },
        }
    }
};
const int totalSongs = sizeof(songs) / sizeof(Song);

void button1Press() {
    button1flag = true;
}

void button2Press() {
    button2flag = true;
}

void setPixels(int color) {
    for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, color);
    }
}

void updateHomeScreen() {
    CircuitPlayground.clearPixels();
    if (millis() / 500 % 2) {
        CircuitPlayground.setPixelColor(selectedSong, 0xffffff);
    }
}

void startGame() {
    // Update game status
    gameStatus = InProgress;

    // Initialize variables
    currentNote = -1;
    currentBeat = 1;
    correctTaps = 0;
    incorrectTaps = 0;

    // Start playback delay
    playbackDelay.start(songs[selectedSong].tempo, AsyncDelay::MILLIS);
}

void displayEndScreen() {
    // Update game status
    gameStatus = EndScreen;

    // Calculate score (0-9)
    float score = 9.0 * correctTaps / (songs[selectedSong].len -
        songs[selectedSong].countoff - songs[selectedSong].rests + incorrectTaps);

    // Display score
    CircuitPlayground.clearPixels();
    for (int i = 0; i <= score && i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 0x0000ff);
    }
}

void nextBeat() {
    if (--currentBeat == 0) {
        // Next note
        if (++currentNote >= songs[selectedSong].len) {
            // End of song
            displayEndScreen();
        } else {
            // Reset beat counter
            currentBeat = songs[selectedSong].notes[currentNote].duration;

            // Play next note
            // CircuitPlayground.playTone() isn't fully non-blocking, so use
            //     Arduino tone() instead
            tone(CPLAY_BUZZER, midi[PitchRest], 10);
            delay(10);
            tone(CPLAY_BUZZER, midi[songs[selectedSong].notes[currentNote].pitch],
                songs[selectedSong].notes[currentNote].duration *
                songs[selectedSong].tempo);
        }
    }
}

bool detectTap() {
    // Check acceleration
    float x = CircuitPlayground.motionX();
    float y = CircuitPlayground.motionY();
    float z = CircuitPlayground.motionZ();
    bool isTap = sqrt(x*x + y*y + z*z) > ACCELERATION_THRESHOLD;

    // Enforce tapInProgress
    bool result = tapInProgress ? false : isTap;
    tapInProgress = isTap;
    return result;
}

bool checkTap() {
    // Calculate time of next/previous notes
    int expiry = playbackDelay.getExpiry();
    int lastNote = expiry - (songs[selectedSong].notes[currentNote].duration -
        currentBeat + 1) * songs[selectedSong].tempo;
    int nextNote = expiry + (currentBeat - 1) * songs[selectedSong].tempo;

    // Determine if tap is in time
    int now = millis();
    return nextNote - now < TIMING_THRESHOLD || now - lastNote < TIMING_THRESHOLD;
}

void setup() {
    // Generate MIDI data
    generateMIDI();

    // Initialize circuit playground
    CircuitPlayground.begin();

    // Setup pins for buttons and speaker
    pinMode(BUTTON_1_PIN, INPUT_PULLDOWN);
    pinMode(BUTTON_2_PIN, INPUT_PULLDOWN);
    pinMode(CPLAY_BUZZER, OUTPUT);

    // Add button interrupts
    attachInterrupt(digitalPinToInterrupt(BUTTON_1_PIN), button1Press, RISING);
    attachInterrupt(digitalPinToInterrupt(BUTTON_2_PIN), button2Press, RISING);

    // Start delays used by homescreen
    blinkDelay.start(500, AsyncDelay::MILLIS);
    sensorDelay.start(SENSOR_REFRESH_PERIOD, AsyncDelay::MILLIS);
}

void loop() {
    switch (gameStatus) {
        case HomeScreen:
            if (blinkDelay.isExpired()) {
                blinkDelay.repeat();

                // Update song selection display
                updateHomeScreen();
            }

            if (sensorDelay.isExpired()) {
                sensorDelay.repeat();

                if (detectTap()) {
                    // Start song
                    startGame();
                }
            }
            break;

        case InProgress:
            if (playbackDelay.isExpired()) {
                playbackDelay.repeat();

                // Play next beat in song
                nextBeat();

                if (gameStatus == EndScreen) {
                    // End of song (don't check for taps)
                    return;
                }
            }

            if (sensorDelay.isExpired()) {
                sensorDelay.repeat();

                if (currentNote >= songs[selectedSong].countoff - 1 && detectTap()) {
                    if (checkTap()) {
                        // Correct tap
                        setPixels(0x00ff00);
                        correctTaps++;
                    } else {
                        // Incorrect tap
                        setPixels(0xff0000);
                        incorrectTaps++;
                    }
                } else {
                    // No tap
                    setPixels(0x000000);
                }
            }
            break;

        case EndScreen:
            if (sensorDelay.isExpired()) {
                sensorDelay.repeat();

                if (detectTap()) {
                    // Return to home screen
                    gameStatus = HomeScreen;
                }
            }
            break;
    }

    if (button1flag || button2flag) {
        switch (gameStatus) {
            case HomeScreen:
                // Cycle through songs
                if (button1flag) {
                    selectedSong = (selectedSong + 1) % totalSongs;
                    updateHomeScreen();
                } else {
                    selectedSong = (selectedSong - 1 + totalSongs) % totalSongs;
                    updateHomeScreen();
                }
                break;

            case InProgress:
                // Exit game
                gameStatus = HomeScreen;

                // Stop audio
                tone(CPLAY_BUZZER, midi[PitchRest], 1);
                break;

            case EndScreen:
                // Return to home screen
                gameStatus = HomeScreen;
                break;
        }

        // Debounce
        delay(5);
        button1flag = false;
        button2flag = false;
    }
}
