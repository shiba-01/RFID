//
// Created by Tan on 25-Oct-23.
//

#ifndef RFID_REQUEST_H
#define RFID_REQUEST_H

#include "Arduino.h"
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "structs.h"

extern HTTPClient http;

class Request {
public:
    Request();

    static bool ping(const String &host_name, const String &header, const String &header_value);

    static http_response
    get(const String &host_name, const String &path_name, const String &query, const String &header,
        const String &header_value, bool is_buffer_used, uint16_t *buffer, int _size);

    static http_response
    post(const String &host_name, const String &path_name, const String &payload, const String &header,
         const String &header_value);

};

#endif //RFID_REQUEST_H
