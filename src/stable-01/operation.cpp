//
// Created by Tan on 25-Oct-23.
//

#include "operation.h"

//operating_mode currentOperatingMode;

Operation::Operation() {
}

void Operation::set_operation_mode(operating_mode_t _operatingMode) {
    currentOperatingMode = _operatingMode;
}

operating_mode_t Operation::get_operating_mode() {
    switch (currentOperatingMode) {
        case TERMINAL:
            Serial.println(F("Current operating mode: Terminal"));
            break;
        case HANDHELD:
            Serial.println(F("Current operating mode: Handheld"));
            break;
    }
    return currentOperatingMode;
}

