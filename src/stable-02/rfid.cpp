//
// Created by Tan on 25-Oct-23.
//

#include "rfid.h"

Rfid::Rfid() {
    scanned_tag_count = 0;
}

void Rfid::init(byte rx_pin, byte tx_pin) {
    Serial2.begin(115200, SERIAL_8N1, rx_pin, tx_pin);
    Serial.println(F("RFID module initiated"));
    get_hardware_version();
    set_tx_power(2600);
//    get_software_version();
}

void Rfid::clean_buffer() {
    // Clean buffer
    for (int i = 0; i < 256; i++) {
        buffer[i] = 0;
    }
}

void Rfid::read_multi_scan_response(volatile bool &_isMenuSelectButtonReleased) {
    clean_buffer();

    int buffer_index = 0;

    bool is_tag_not_found = false;
    bool timeout_occurred = false;

    unsigned long start_time = millis();
    const unsigned long timeout = 500;
    const uint16_t NOTICE_FRAME_LENGTH = 24; // Expected notice frame length
    uint8_t prevScannedTagCount = scanned_tag_count;

    // Read data from RFID module when this method is called
    while (millis() - start_time < timeout && !_isMenuSelectButtonReleased &&
           (scanned_tag_count == prevScannedTagCount)) {
        if (Serial2.available()) {
            byte readByte = Serial2.read();
            //Serial.print(readByte, HEX);
            buffer[buffer_index] = readByte;
            ++buffer_index;

            // Check if the end byte is detected
            if (readByte == 0x7e) {
                //Serial.println();
                //Serial.println(buffer_index);
                // Check if we have a valid notice frame, 0xbb 02 22...........0x7e
                if (buffer[0] == 0xbb && buffer[1] == 0x02 && buffer[2] == 0x22 &&
                    buffer_index == NOTICE_FRAME_LENGTH) {
                    // We have a valid notice frame with the expected length, print its EPC
//                    print_raw_read(buffer, buffer_index);
//                    print_epc(buffer, buffer_index);
                    store_epc(buffer, buffer_index);
                    // Reset buffer index for the next reading
                    buffer_index = 0;
                }
                    // Check if we have a valid response frame, 0xbb 01 FF...........0x7e
                else if (buffer[0] == 0xbb && buffer[1] == 0x01 && buffer[2] == 0xFF && buffer_index >= 5) {
                    // Check if response is tag not found (error code 0x15)
                    if (buffer[5] == 0x15) {
                        is_tag_not_found = true;
                        break;
                    }
                }
            }

            // Check if the buffer is full
            if (buffer_index >= sizeof(buffer)) {
                Serial.println("Buffer overflow");
                // Reset buffer index to prevent further reading
                buffer_index = 0;
            }
        }
    }

    // Check if the scanned tag count has changed
    if (scanned_tag_count != prevScannedTagCount) {
        return;
    }

    // Check if timeout occurred
    if (millis() - start_time >= timeout) {
        timeout_occurred = true;
    }

    // Print message if no tag is found and timeout didn't occur
    if (is_tag_not_found && !timeout_occurred) {
        Serial.println("No tag found");
    }
}

void Rfid::read_single_scan_response() {
    clean_buffer();

    int buffer_index = 0;

    bool is_tag_not_found = false;
    bool timeout_occurred = false;

    unsigned long start_time = millis();
    const unsigned long timeout = 500;

    const uint16_t NOTICE_FRAME_LENGTH = 24; // Expected notice frame length

    // Read data from RFID module when this method is called
    while (millis() - start_time < timeout) {
        if (Serial2.available()) {
            byte readByte = Serial2.read();
            //Serial.print(readByte, HEX);
            buffer[buffer_index] = readByte;
            ++buffer_index;

            // Check if the end byte is detected
            if (readByte == 0x7e) {
                //Serial.println();
                //Serial.println(buffer_index);
                // Check if we have a valid notice frame, 0xbb 02 22...........0x7e
                if (buffer[0] == 0xbb && buffer[1] == 0x02 && buffer[2] == 0x22 &&
                    buffer_index == NOTICE_FRAME_LENGTH) {
                    // We have a valid notice frame with the expected length, print its EPC
//                    print_raw_read(buffer, buffer_index);
//                    print_epc(buffer, buffer_index);
                    store_epc(buffer, buffer_index);
                    // Reset buffer index for the next reading
                    buffer_index = 0;
                }
                    // Check if we have a valid response frame, 0xbb 01 FF...........0x7e
                else if (buffer[0] == 0xbb && buffer[1] == 0x01 && buffer[2] == 0xFF && buffer_index >= 5) {
                    // Check if response is tag not found (error code 0x15)
                    if (buffer[5] == 0x15) {
                        is_tag_not_found = true;
                        break;
                    }
                }
            }

            // Check if the buffer is full
            if (buffer_index >= sizeof(buffer)) {
                buffer_index = 0;
//                Serial.println("Buffer overflow");
                // Reset buffer index to prevent further reading
            }
        }
    }

    // Check if timeout occurred
    if (millis() - start_time >= timeout) {
        timeout_occurred = true;
    }

    // Print message if no tag is found and timeout didn't occur
    if (is_tag_not_found && !timeout_occurred) {
//        Serial.println("No tag found");
    }
}

void Rfid::print_raw_read(const uint8_t *_buffer, uint8_t buffer_size) {
    Serial.print("Raw Read: ");
    for (uint8_t i = 0; i < buffer_size; ++i) {
        if (_buffer[i] < 0x10) {
            Serial.print("0");
        }
        Serial.print(_buffer[i], HEX);
    }
    Serial.println();
}

void Rfid::print_epc(const uint8_t *buffer, uint8_t buffer_size) {
    const uint8_t EPC_START_INDEX = 8; // Starting index of EPC in the buffer
    const uint8_t EPC_LENGTH = 12; // Expected EPC length

    // Check if the buffer size is sufficient
    if (buffer_size >= EPC_START_INDEX + EPC_LENGTH) {
        Serial.print("EPC: ");
        for (uint8_t i = 0; i < EPC_LENGTH; ++i) {
            if (buffer[EPC_START_INDEX + i] < 0x10) {
                Serial.print("0");
            }
            Serial.print(buffer[EPC_START_INDEX + i], HEX);
        }
        Serial.println();
    } else {
        Serial.println("Invalid EPC data");
    }
}

void Rfid::store_epc(const uint8_t *_buffer, uint8_t buffer_size) {
    const uint8_t EPC_START_INDEX = 8; // Starting index of EPC in the buffer
    const uint8_t EPC_LENGTH = 12; // Expected EPC length

    // Check if the buffer size is sufficient
    if (buffer_size >= EPC_START_INDEX + EPC_LENGTH && scanned_tag_count < 200) {
        String epc = "";
        // Convert the EPC bytes to a string
        for (uint8_t i = 0; i < EPC_LENGTH; i++) {
            char hex[3];
            sprintf(hex, "%02X", _buffer[EPC_START_INDEX + i]);
            epc += hex;
        }

        if (!is_duplicate_scan(epc)) {
//            Serial.print(F("New added EPC: "));
//            Serial.println(epc);
            // Clear the existing EPC string
            scan_results[scanned_tag_count].epc = epc;
            scan_results[scanned_tag_count].is_matched_check = false;
            // Increment the scanned tag count
            ++scanned_tag_count;
        }
    }
}

bool Rfid::read_response(unsigned long timeout) {
    unsigned long start_time = millis();

    uint8_t buffer_index = 0;

    clean_buffer();

    // Read the response until no more data is received for the duration of the timeout
    while (Serial2.available() || (millis() - start_time < timeout)) {
        if (Serial2.available()) {
            byte readByte = Serial2.read();
            buffer[buffer_index] = readByte;
            ++buffer_index;
            // Check if the end byte is detected
            if (readByte == 0x7e) {
                break;
            }
        }
    }

    if (buffer[0] == 0xbb && buffer[buffer_index - 1] == 0x7e) {
        return true;
    } else {
        return false;
    }
}

void Rfid::send_command(uint8_t *data, size_t size) {
    Serial.println(F("Sending command"));
    Serial2.write(data, size);
    //Serial2.flush(); // Wait for the data to be completely sent
}

void Rfid::get_hardware_version() {
    Serial.println(F("Getting RFID hardware version: "));
    send_command((uint8_t *) HARDWARE_VERSION_CMD, sizeof(HARDWARE_VERSION_CMD));
    if (read_response(3000)) {
        String info;
        for (uint8_t i = 0; i < 50; i++) {
            info += (char) buffer[6 + i];
            if (buffer[8 + i] == 0x7e) {
                break;
            }
        }
        Serial.println(info);
    } else {
        Serial1.println(F("Error in getting hardware version"));
    }
}

void Rfid::get_software_version() {
    Serial.println(F("Getting RFID software version"));
    send_command((uint8_t *) SOFTWARE_VERSION_CMD, sizeof(SOFTWARE_VERSION_CMD));
    read_response(3000);
}

void Rfid::polling_once() {
    send_command((uint8_t *) POLLING_ONCE_CMD, sizeof(POLLING_ONCE_CMD));
    read_single_scan_response();
    //Serial.println(read_response(false, nullptr, 8, 20, EPC_READING));
}

void Rfid::polling_multi() {
//    send_command((uint8_t *) STOP_POLLING_MULTI_CMD, sizeof(STOP_POLLING_MULTI_CMD),
//                 true, (uint8_t *) SUCCESSFULLY_STOP_POLLING_MULTI);
    send_command((uint8_t *) POLLING_MULTIPLE_CMD, sizeof(POLLING_MULTIPLE_CMD));
    //read_multi_scan_response();
}

void Rfid::scan_rfid_tag() {
    switch (scanning_mode) {
        case SINGLE_SCAN:
            Serial.println(F("Start scanning RFID tags once"));
            polling_once();
            Serial.print(F("Total scanned RFID tags currently: "));
            Serial.println(scanned_tag_count);
            stop_scanning();
            //for (int i = 0; i < 200; ++i) {
            //Serial.print(scan_results[i].epc);
            //Serial.print(", ");
            //}
            //Serial.println();
            break;
        case MULTI_SCAN:
            Serial.println(F("Start scanning multi RFID tags"));
            polling_multi();
            //stop_scanning();
            break;
    }
}

void Rfid::set_scanning_mode(rfid_scanning_mode_t _scanning_mode) {
    scanning_mode = _scanning_mode;
}

void Rfid::set_tx_power(uint16_t db) {
    Serial.println(F("Start setting tx power"));
    send_command((uint8_t *) SET_TX_POWER, sizeof(SET_TX_POWER));
    if (read_response(500)) {
        Serial.println(F("Set TX power successfully"));
    } else {
        Serial.println(F("Set TX power failed"));
    }
//    Serial.println(F("Start setting high sensitivity"));
//    send_command((uint8_t *) SET_HIGH_SENSITIVITY, sizeof(SET_HIGH_SENSITIVITY));
//    if (read_response(500)) {
//        Serial.println(F("Set high sensitivity successfully"));
//    } else {
//        Serial.println(F("Set high sensitivity failed"));
//    }
}

bool Rfid::is_duplicate_scan(const String &epc) {
    for (int i = 0; i < scanned_tag_count; ++i) {
        if (epc.equalsIgnoreCase(scan_results[i].epc)) {
            return true;
        }
    }
    return false;
}

void Rfid::stop_scanning() {
    Serial.println(F("Stop scanning multi RFID tags"));
    send_command((uint8_t *) STOP_POLLING_MULTI_CMD, sizeof(STOP_POLLING_MULTI_CMD));
    if (read_response(5000)) {
        Serial.println(F("Stopped scanning multi RFID tags"));
    } else {
        Serial.println(F("Failed to stop scanning multi RFID tags"));
    }
    //Serial.println(read_response(true, (uint8_t *) SUCCESSFULLY_STOP_POLLING_MULTI, 8, 3000, NORMAL_READING));
}

