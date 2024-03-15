//
// Created by Tan on 25-Oct-23.
//

#ifndef RFID_OPERATION_H
#define RFID_OPERATION_H

#include "Arduino.h"
#include "enums.h"

//extern operating_mode currentOperatingMode;

class Operation {
public:
    operating_mode_t currentOperatingMode;

    Operation();

    void set_operation_mode(operating_mode_t _operatingMode);

    operating_mode_t get_operating_mode();
};
#endif //RFID_OPERATION_H
