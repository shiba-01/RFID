//
// Created by Tan on 25-Oct-23.
//

#include "buzzer.h"

Buzzer::Buzzer() {
    Serial.println(F("Buzzer initiated"));
}

void Buzzer::init(byte _buzzerPin) {
    buzzerPin = _buzzerPin;
}

void Buzzer::welcome_sound() const {
    Serial.println(F("Start playing welcome sound"));
//    tone(buzzerPin, 880);  // Play A5 note (880 Hz), a high "beep"
//    delay(100);            // Duration 100 ms
//    noTone(buzzerPin);     // Stop the tone
//    digitalWrite(buzzerPin, HIGH);
    Serial.println(F("Stop playing welcome sound"));
}

void Buzzer::successful_sound() const {
    Serial.println(F("Start playing successful sound"));
//    tone(buzzerPin, 880); // Play A5 note (880 Hz), mid-range
//    delay(100);           // Duration 100 ms
//    tone(buzzerPin, 440); // Play A4 note (440 Hz), lower-range
//    delay(100);           // Duration 100 ms
//    tone(buzzerPin, 1760); // Play A6 note (1760 Hz), high-range
//    delay(200);           // Duration 200 ms
//    noTone(buzzerPin);     // Stop the tone
//    digitalWrite(buzzerPin, HIGH);
    Serial.println(F("Stop playing successful sound"));
}

void Buzzer::failure_sound() const {
    Serial.println(F("Start playing failure sound"));
//    tone(buzzerPin, 200); // Play G3 note (200 Hz), a low buzz-like "error"
//    delay(250);           // Duration 250 ms for a longer sound
//    noTone(buzzerPin);    // Stop the tone
//    delay(100);           // Pause between tones
//    tone(buzzerPin, 150); // Play a lower tone (150 Hz) for contrast
//    delay(250);           // Duration 250 ms for a longer sound
//    noTone(buzzerPin);    // Stop the tone
//    delay(10);            // Short delay to ensure the tone has stopped
    Serial.println(F("Stop playing failure sound"));
}

