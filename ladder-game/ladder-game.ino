#include "pitches.h"

// NOTE: this method of wiring was probably a mistake. The LEDs are in reverse
// order with LED_END being the first LED you need to activate and LED_START
// being the last.
#define LED_START 3
#define LED_END   12

#define BUZZER    2
#define BUTTON    13

// Melodies are defined as sets of note, duration, delay ending with a 0.
const int failMelody[] = {
  NOTE_C3, 450, 500,
  NOTE_B2, 450, 500,
  0,
};

const int successMelody[] = {
  NOTE_D4, 100, 100,
  NOTE_E4, 100, 100,
  0,
};

const int winMelody[] = {
  NOTE_D4,  100, 150,
  NOTE_D4,  100, 150,
  NOTE_D4,  100, 150,
  NOTE_D4,  300, 450,
  NOTE_AS3, 300, 450,
  NOTE_C4,  300, 450,
  NOTE_D4,  200, 300,
  NOTE_C4,  100, 150,
  NOTE_D4,  600, 900,
  0,
};

void playMelody(const int *melody) {
  for (int thisNote = 0; melody[thisNote]; thisNote += 3) {
    tone(BUZZER, melody[thisNote], melody[thisNote + 1]);
    delay(melody[thisNote + 2]);
  }
}

void setup() {
  //pinMode(LED_BUILTIN, OUTPUT);
  for (int i = LED_START; i <= LED_END; i++) {
    pinMode(i, OUTPUT);
  }
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
}

#define GS_INIT  0
#define GS_INPUT 1
#define GS_CLIMB 2
#define GS_FALL  3
#define GS_WIN   4

int gameState = GS_INIT;
int minActive = LED_END + 1;
int blinkState = LOW;

void ledMarquee() {
  for (int i = LED_END; i >= LED_START; i--) {
    digitalWrite(i, HIGH);
    delay(100);
  }

  for (int i = LED_END; i >= LED_START; i--) {
    digitalWrite(i, LOW);
    delay(100);
  }
}

void ledPingPong(int count) {
  // Reset all LEDs to 0
  for (int i = LED_END; i >= LED_START; i--) {
    digitalWrite(i, LOW);
    delay(100);
  }

  digitalWrite(LED_END, HIGH);
  delay(100);

  for (int loop = 0; loop < count; loop++) {
    for (int i = LED_END - 1; i >= LED_START; i--) {
      digitalWrite(i + 1, LOW);
      digitalWrite(i, HIGH);
      delay(100);
    }

    for (int i = LED_START + 1; i <= LED_END; i++) {
      digitalWrite(i - 1, LOW);
      digitalWrite(i, HIGH);
      delay(100);
    }
  }

  digitalWrite(LED_END, LOW);
}


void gsInit() {
  minActive = LED_END + 1;
  blinkState = LOW;

  // ledMarquee();

  gameState = GS_INPUT;
}

void gsInput() {
  unsigned long curTime = millis();

  int ledNum = LED_END - (minActive - LED_START) - 1;

  int blinkLen = 500 - (3 * ledNum * ledNum);

  // Update the blinkState with whether or not the LED is supposed to be on.
  if ((curTime / blinkLen) % 2 == 0) {
    if (blinkState != LOW) {
      digitalWrite(minActive - 1, LOW);
      blinkState = LOW;
    }
  } else {
    if (blinkState != HIGH) {
      digitalWrite(minActive - 1, HIGH);
      blinkState = HIGH;
    }
  }

  // If we got an input, check the blink state and update the game state accordingly.
  int input = digitalRead(BUTTON);
  if (input == LOW) {
    if (blinkState == HIGH) {
      gameState = GS_CLIMB;
    } else {
      gameState = GS_FALL;
    }

    delay(100);
  }
}

void gsClimb() {
  minActive--;
  if (minActive <= LED_START) {
    gameState = GS_WIN;
    return;
  }

  playMelody(successMelody);

  gameState = GS_INPUT;
}

void gsFall() {
  minActive++;

  // Clamp the value so we can't go before the first LED.
  if (minActive > LED_END) {
    minActive = LED_END + 1;
  }

  playMelody(failMelody);

  // Ensure the previous light is off.
  digitalWrite(minActive - 1, LOW);

  gameState = GS_INPUT;
}

void gsWin() {
  playMelody(winMelody);
  ledPingPong(3);
  gameState = GS_INIT;
}

void loop() {
  switch (gameState) {
    case GS_INPUT:
      gsInput();
      break;
    case GS_CLIMB:
      gsClimb();
      break;
    case GS_FALL:
      gsFall();
      break;
    case GS_WIN:
      gsWin();
      break;

    // By default we fall back to init, in case we broke something.
    default:
    case GS_INIT:
      gsInit();
      break;
  }
}
