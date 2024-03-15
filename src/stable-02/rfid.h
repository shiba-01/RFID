//
// Created by Tan on 25-Oct-23.
//

#ifndef RFID_RFID_H
#define RFID_RFID_H

#include "Arduino.h"
#include "structs.h"
#include <unordered_set>

/*
Header Type Command PL(MSB) PL(LSB) Parameter Checksum End
  BB    00     07     00      01        01       09     7E
                     param length
*/

const uint8_t HARDWARE_VERSION_CMD[] = {0xBB, 0x00, 0x03, 0x00,
                                        0x01, 0x00, 0x04, 0x7E};
const uint8_t SOFTWARE_VERSION_CMD[] = {0xBB, 0x00, 0x03, 0x00,
                                        0x01, 0x01, 0x05, 0x7E};
const uint8_t POLLING_ONCE_CMD[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
const uint8_t POLLING_MULTIPLE_CMD[] = {0xBB, 0x00, 0x27, 0x00, 0x03,
                                        0x22, 0x27, 0x10, 0x83, 0x7E};
const uint8_t SET_SELECT_MODE_CMD[] = {0xBB, 0x00, 0x12, 0x00,
                                       0x01, 0x01, 0x14, 0x7E};
const uint8_t SET_SELECT_PARAMETER_CMD[] = {
        0xBB, 0x00, 0x0C, 0x00, 0x13, 0x01, 0x00, 0x00, 0x00,
        0x20, 0x60, 0x00, 0x30, 0x75, 0x1F, 0xEB, 0x70, 0x5C,
        0x59, 0x04, 0xE3, 0xD5, 0x0D, 0x70, 0xAD, 0x7E};
const uint8_t SET_SELECT_OK[] = {0xBB, 0x01, 0x0C, 0x00,
                                 0x01, 0x00, 0x0E, 0x7E};
const uint8_t GET_SELECT_PARAMETER_CMD[] = {0xBB, 0x00, 0x0B, 0x00,
                                            0x00, 0x0B, 0x7E};
const uint8_t READ_STORAGE_CMD[] = {0xBB, 0x00, 0x39, 0x00, 0x09, 0x00,
                                    0x00, 0xFF, 0xFF, 0x03, 0x00, 0x00,
                                    0x00, 0x02, 0x45, 0x7E};
const uint8_t READ_STORAGE_ERROR[] = {0xBB, 0x01, 0xFF, 0x00,
                                      0x01, 0x09, 0x0A, 0x7E};
const uint8_t WRITE_STORAGE_CMD[] = {0xBB, 0x00, 0x49, 0x00, 0x0D, 0x00, 0x00,
                                     0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x02,
                                     0x12, 0x34, 0x56, 0x78, 0x6D, 0x7E};
const uint8_t WRITE_STORAGE_ERROR[] = {0xBB, 0x01, 0xFF, 0x00,
                                       0x01, 0x10, 0x0A, 0x7E};
const uint8_t SET_TX_POWER[] = {0xBB, 0x00, 0xB6, 0x00, 0x02,
                                0x07, 0xD0, 0x8F, 0x7E};
const uint8_t STOP_POLLING_MULTI_CMD[] = {0xBB, 0x00, 0x28, 0x00, 0x00, 0x28, 0x7E,};
const uint8_t SET_HIGH_SENSITIVITY[] = {0xBB, 0x00, 0xF5, 0x00, 0x01, 0x00, 0xF6, 0x7E};

/*------------------- Responses from RFID module -----------------*/
const uint8_t RFID_TAG_NOT_FOUND[] = {0xBB, 0x01, 0xFF, 0x00, 0x01, 0x15, 0x16, 0x7E};

const uint8_t SUCCESSFULLY_STOP_POLLING_MULTI[] = {0xBB, 0x01, 0x28, 0x00, 0x01, 0x00, 0x2A, 0x7E};

const uint8_t SUCCESSFULLY_SET_TX_POWER[] = {0xBB, 0x01, 0xB6, 0x00, 0x01, 0x00, 0xB8, 0x7E};

class Rfid {
public:
    rfid_scanning_mode_t scanning_mode;
    rfid_tag scan_results[200];
    uint8_t buffer[256] = {0};
    int scanned_tag_count = 0;

    Rfid();

    void init(byte rx_pin, byte tx_pin);

    void read_multi_scan_response(volatile bool &_isMenuSelectButtonReleased);

    bool read_response(unsigned long timeout);

    static void send_command(uint8_t *data, size_t size);

    void get_hardware_version();

    void get_software_version();

    void polling_once();

    static void polling_multi();

    void scan_rfid_tag();

    void set_scanning_mode(rfid_scanning_mode_t _scanning_mode);

    void set_tx_power(uint16_t db);

    bool is_duplicate_scan(const String &epc);

    void stop_scanning();

    void read_single_scan_response();

    void clean_buffer();

    static void print_epc(const uint8_t *buffer, uint8_t buffer_size);

    void store_epc(const uint8_t *_buffer, uint8_t buffer_size);

    static void print_raw_read(const uint8_t *buffer, uint8_t buffer_size);
};

#endif //RFID_RFID_H
