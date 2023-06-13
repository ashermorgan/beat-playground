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

enum Note {
    NoteC4  = 60,
    NoteDb4 = 61,
    NoteD4  = 62,
    NoteEb4 = 63,
    NoteE4  = 64,
    NoteF4  = 65,
    NoteGb4 = 66,
    NoteG4  = 67,
    NoteAb4 = 68,
    NoteA4  = 69,
    NoteBb4 = 70,
    NoteB4  = 71,
    NoteC5  = 72,
    NoteDb5 = 73,
    NoteD5  = 74,
    NoteEb5 = 75,
    NoteE5  = 76,
    NoteF5  = 77,
    NoteGb5 = 78,
    NoteG5  = 79,
    NoteAb5 = 80,
    NoteA5  = 81,
    NoteBb5 = 82,
    NoteB5  = 83,
    NoteBeat = 128,
    NoteRest = 129,
};

struct Song {
    int tempo;
    int len;
    int countoff;
    int rests;
    int notes[][2];
};
Song song1 = {
    // Twinkle Twinkle Little Star
    500, 18, 4, 0, {
        { NoteBeat, 1 }, { NoteBeat, 1 }, { NoteBeat, 1 }, { NoteBeat, 1 },

        { NoteC4, 1 }, { NoteC4, 1 }, { NoteG4, 1 }, { NoteG4, 1 },
        { NoteA4, 1 }, { NoteA4, 1 }, { NoteG4, 2 },
        { NoteF4, 1 }, { NoteF4, 1 }, { NoteE4, 1 }, { NoteE4, 1 },
        { NoteD4, 1 }, { NoteD4, 1 }, { NoteC4, 2 },
    }
};
Song song2 = {
    // Centuries
    170, 31, 3, 1, {
        { NoteBeat, 4 }, { NoteBeat, 4 }, { NoteBeat, 4 },

        { NoteE4, 2 }, { NoteGb4, 2 },
        { NoteG4, 2 }, { NoteGb4, 2 }, { NoteE4, 2 }, { NoteE4, 4 },
        { NoteE4, 2 }, { NoteE4, 2 },  { NoteE4, 2 },
        { NoteG4, 2 }, { NoteGb4, 2 }, { NoteE4, 2 }, { NoteE4, 4 },
        { NoteE4, 2 }, { NoteE4, 2 },  { NoteD4, 2 },
        { NoteE4, 2 }, { NoteB4, 1 },  { NoteB4, 9 }, { NoteRest, 2 }, { NoteD4, 2 },
        { NoteE4, 2 }, { NoteB4, 1 },  { NoteB4, 3 }, { NoteD4, 2 },
        { NoteE4, 2 }, { NoteB4, 1 },  { NoteB4, 3 },

    }
};
Song song3 = {
    // Blinding Lights
    175, 23, 4, 0, {
        { NoteBeat, 2 }, { NoteBeat, 2 }, { NoteBeat, 2 }, { NoteBeat, 2 },

        { NoteF4, 13 }, { NoteF4, 1 },  { NoteG4, 2 },
        { NoteF4, 2 },  { NoteEb4, 1 }, { NoteEb4, 2 }, { NoteC4, 1 },
        { NoteEb4, 7 }, { NoteEb4, 3 },
        { NoteBb4, 1 }, { NoteG4, 2 },  { NoteF4, 2 },  { NoteEb4, 3 },
        { NoteBb4, 1 }, { NoteG4, 2 },  { NoteF4, 2 },  { NoteEb4, 1 }, { NoteF4, 4 },
        { NoteF4, 2 },
    }
};
Song song4 = {
    // Counting Stars
    125, 46, 4, 2, {
        { NoteBeat, 4 }, { NoteBeat, 4 }, { NoteBeat, 4 }, { NoteBeat, 4 },

        { NoteGb4, 4 },  { NoteAb4, 4 }, { NoteB4, 4 },  { NoteAb4, 4 },
        { NoteGb4, 2 },  { NoteAb4, 2 }, { NoteGb4, 1 }, { NoteE4, 2 },  { NoteAb4, 9 },
        { NoteRest, 2 }, { NoteGb4, 2 }, { NoteGb4, 2 }, { NoteAb4, 1 }, { NoteA4, 3 },
        { NoteAb4, 2 },  { NoteGb4, 2 }, { NoteE4, 2 },
        { NoteAb4, 4 },  { NoteDb4, 4 }, { NoteE4, 8 },

        { NoteGb4, 4 },  { NoteAb4, 4 }, { NoteB4, 4 },  { NoteAb4, 4 },
        { NoteGb4, 2 },  { NoteAb4, 2 }, { NoteGb4, 1 }, { NoteE4, 2 },  { NoteAb4, 9 },
        { NoteRest, 2 }, { NoteGb4, 2 }, { NoteGb4, 2 }, { NoteAb4, 2 },
        { NoteA4, 2 },   { NoteAb4, 2 }, { NoteGb4, 1 }, { NoteE4, 3 },
        { NoteAb4, 2 },  { NoteGb4, 2 }, { NoteGb4, 2 }, { NoteDb4, 2 }, { NoteE4, 8 },
    }
};
const int totalSongs = 4;

void button1Press() {
    button1flag = true;
}

void button2Press() {
    button2flag = true;
}

// Song data helpers
int currentTempo() {
    switch(selectedSong) {
        case 0:
            return song1.tempo;
        case 1:
            return song2.tempo;
        case 2:
            return song3.tempo;
        case 3:
            return song4.tempo;
    }
}
int currentLength() {
    switch(selectedSong) {
        case 0:
            return song1.len;
        case 1:
            return song2.len;
        case 2:
            return song3.len;
        case 3:
            return song4.len;
    }
}
int currentCountoff() {
    switch(selectedSong) {
        case 0:
            return song1.countoff;
        case 1:
            return song2.countoff;
        case 2:
            return song3.countoff;
        case 3:
            return song4.countoff;
    }
}
int currentRests() {
    switch(selectedSong) {
        case 0:
            return song1.rests;
        case 1:
            return song2.rests;
        case 2:
            return song3.rests;
        case 3:
            return song4.rests;
    }
}
int currentPitch() {
    switch(selectedSong) {
        case 0:
            return midi[song1.notes[currentNote][0]];
        case 1:
            return midi[song2.notes[currentNote][0]];
        case 2:
            return midi[song3.notes[currentNote][0]];
        case 3:
            return midi[song4.notes[currentNote][0]];
    }
}
int currentDuration() {
    switch(selectedSong) {
        case 0:
            return song1.notes[currentNote][1];
        case 1:
            return song2.notes[currentNote][1];
        case 2:
            return song3.notes[currentNote][1];
        case 3:
            return song4.notes[currentNote][1];
    }
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
    playbackDelay.start(currentTempo(), AsyncDelay::MILLIS);
}

void displayEndScreen() {
    // Update game status
    gameStatus = EndScreen;

    // Calculate score (0-9)
    float score = 9.0 * correctTaps / (currentLength() - currentCountoff() -
        currentRests() + incorrectTaps);

    // Display score
    CircuitPlayground.clearPixels();
    for (int i = 0; i <= score && i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 0x0000ff);
    }
}

void nextBeat() {
    if (--currentBeat == 0) {
        // Next note
        if (++currentNote >= currentLength()) {
            // End of song
            displayEndScreen();
        } else {
            // Reset beat counter
            currentBeat = currentDuration();

            // Play next note
            // CircuitPlayground.playTone() isn't fully non-blocking, so use
            //     Arduino tone() instead
            tone(CPLAY_BUZZER, midi[NoteRest], 10);
            delay(10);
            tone(CPLAY_BUZZER, currentPitch(), currentDuration() * currentTempo());
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
    int lastNote = expiry - (currentDuration() - currentBeat + 1) * currentTempo();
    int nextNote = expiry + (currentBeat - 1) * currentTempo();

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

                if (currentNote >= currentCountoff() - 1 && detectTap()) {
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
                tone(CPLAY_BUZZER, midi[NoteRest], 1);
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
