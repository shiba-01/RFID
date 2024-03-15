//
// Created by Tan on 25-Oct-23.
//

#include "request.h"

HTTPClient http;

Request::Request() {

}

bool Request::ping(const String &host_name, const String &header, const String &header_value) {
    Serial.println(F("Checking API server connection: "));

    http.begin(host_name);
    http.addHeader(header, header_value);

    int httpCode = http.GET();

    if (httpCode > 0) {
        Serial.println(F("API server is reachable."));
        http.end();
        return true;
    } else {
        Serial.print(F("Failed to reach API server. Error code: "));
        Serial.println(httpCode);
        http.end();
        return false;
    }
}

http_response Request::get(const String &host_name, const String &path_name, const String &query, const String &header,
                           const String &header_value, bool is_buffer_used, uint16_t *buffer, int _size) {
    http_response _response;
    http.begin(host_name + path_name + query);
    if (header != "") {
        http.addHeader(header, header_value);
    }
    _response.status_code = http.GET();
    if (_response.status_code > 0) {
        if (_response.status_code == HTTP_CODE_OK) {
            Serial.print(F("GET request is successful. Response: "));
            if (!is_buffer_used) {
                _response.payload = http.getString();
                Serial.println(_response.payload);
            } else {
                int len = http.getSize();
                Serial.print(F("Downloading image, size = "));
                Serial.println(len);

                int count = 0;
                int index = 0;
                // Buffer to read
                uint8_t buff[128] = {0};
                // Array to store the continuous group of 2 bytes; then we combine them to get the original byte
                uint8_t buff2[2] = {0};
                // uint16_t variable to store the original byte
                uint16_t buff16 = 0;
                // Clear image array
                memset(buffer, 0, _size);
                WiFiClient *stream = http.getStreamPtr();
                while (http.connected() && (len > 0 || len == -1)) {
                    // get available data size
                    size_t size = stream->available();
                    // Count the number of bytes read
                    count += size;
                    // Print size to Serial with line break
                    //Serial.printf("Size available: %d\n", size);
                    //Serial.println();
                    if (size) {
                        // read up to 128 bytes
                        int c = stream->readBytes(buff, std::min((size_t) len, sizeof(buff)));
                        int i = 0;
                        while (i < c) {
                            // Print buff[i] to Serial as HEX
                            //Serial.printf("%02X ", buff[i]);
                            //Serial.printf("%02X ", buff[i + 1]);
                            // Print buff[i] to Serial as DEC
                            //Serial.printf("%d ", buff[i]);
                            // Add buff[i] and buff[i+1] to buff2
                            buff2[0] = buff[i];
                            buff2[1] = buff[i + 1];
                            // Concatenate buff2[0] and buff2[1] to 16-bit variable buff16
                            buff16 = (buff2[0] << 8) | buff2[1];
                            // Print buff16 to Serial as HEX
                            //Serial.printf("%04X ", buff16);
                            // Print buff16 to Serial as DEC
                            //Serial.printf("%d ", buff16);
                            // Reset buff2
                            memset(buff2, 0, sizeof(buff2));
                            // Write buff16 to imageArray
                            //imageArray[index] = buff[i];
                            buffer[index] = buff16;
                            // Increase index
                            index++;
                            // Increase i by 2
                            i += 2;
                        }
                        if (len > 0) {
                            len -= c;
                        }
                    }
                    //Serial.println();
                    delay(1);
                }
            }
        } else {
            Serial.print(F("HTTP status is not OK: "));
            Serial.println(_response.status_code);
            _response.payload = "failed";
        }
    } else {
        Serial.println(F("HTTP status code # 0"));
        _response.payload = "failed";
    }

    http.end();
    return _response;
}

http_response
Request::post(const String &host_name, const String &path_name, const String &payload, const String &header,
              const String &header_value) {
    http_response _response;
    http.begin(host_name + path_name);
    http.addHeader(header, header_value);
    http.addHeader("Content-Type", "application/xml");
    Serial.println(F("Sending payload: "));
    Serial.println(payload);
    _response.status_code = http.POST(payload);

    if (_response.status_code > 0) {
        if (_response.status_code == HTTP_CODE_OK) {
            _response.payload = http.getString();
            Serial.print(F("POST request is successful. Response: "));
            Serial.println(_response.payload);
        } else {
            _response.payload = "failed";
        }
    } else {
        _response.payload = "failed";
    }

    http.end();
    return _response;
}