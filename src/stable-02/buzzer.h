//
// Created by Tan on 25-Oct-23.
//

#ifndef RFID_BUZZER_H
#define RFID_BUZZER_H

#include "Arduino.h"

class Buzzer {
private:
    byte buzzerPin = 0;
    bool is_muted = true;
public:
    Buzzer();

    void init(byte _buzzerPin);

    void mute(bool _is_mute);

    void welcome_sound() const;

    void successful_sound() const;

    void failure_sound() const;
};
#endif //RFID_BUZZER_H
