//
// Created by Tan on 25-Oct-23.
//

#ifndef RFID_WS2812B_H
#define RFID_WS2812B_H

#include "Arduino.h"

class Ws2812b {
public:
    Ws2812b();

    void init();

    void set_color();

    void set_brightness();

    void set_animation();

    void animate();
};
#endif //RFID_WS2812B_H
