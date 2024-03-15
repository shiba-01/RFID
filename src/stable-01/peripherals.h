//
// Created by Tan on 25-Oct-23.
//

#ifndef RFID_PERIPHERALS_H
#define RFID_PERIPHERALS_H

#include "Arduino.h"
#include "enums.h"

class Peripherals {
private:
    byte leftUpNavButtonPin, backCancelNavButtonPin, menuSelectNavButtonPin, rightDownNavButtonPin;
    byte lastMenuSelectNavButtonState, lastLeftUpNavButtonState, lastRightDownNavButtonState, lastBackCancelNavButtonState;
public:
    Peripherals();

    void init_navigation_buttons(byte _leftUpNavButtonPin, byte _backCancelNavButtonPin, byte _menuSelectNavButtonPin,
                                 byte _rightDownNavButtonPin);

    button_type_t read_navigation_buttons(byte &currentScreenItemIndex, byte &screenItemCount,
                                          feature_item_type_t &feature_item_type);

    static void blink_led(byte ledPin);

    static void set_digital_input(byte pin);

    static void set_digital_output(byte pin);

    static void retrieve_corresponding_task(task_t &previousTask, task_t &currentTask);

    static void
    retrieve_corresponding_feature(feature_t &previousFeature, feature_t &currentFeature, feature_t &argsFeature,
                                   byte &screenItemIndex, feature_t (&screenFeatures)[10], button_type_t &button_type,
                                   feature_t (&navigation_history)[10], byte &navigation_history_size);
};

#endif //RFID_PERIPHERALS_H
