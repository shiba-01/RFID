//
// Created by Tan on 30-Oct-23.
//

#include "mqtt.h"

AsyncMqttClient mqttClient;

MQTT *MQTT::instance = nullptr;

MQTT::MQTT() {
    instance = this;

    is_broker_connected = false;
    is_mes_package_selected = false;

    expected_event = NONE;
    Serial.println(F("MQTT instance initiated"));
}

void MQTT::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.println(F("Disconnected from MQTT. Try reconnecting"));
    mqttClient.connect();
}

void MQTT::onMqttConnectStatic(bool sessionPresent) {
    if (MQTT::instance) {
        MQTT::instance->onMqttConnect(sessionPresent);
    }
}

void MQTT::onMqttConnect(bool sessionPresent) {
    Serial.println(F("Connected to MQTT."));
//    Serial.print(F("Session present: "));
//    Serial.println(sessionPresent);
    is_broker_connected = true;

    mqttClient.subscribe(lwt_topic.c_str(), 0);

    mqttClient.publish(lwt_topic.c_str(), 0, false,
                       (String(R"({"mac": ")") + String(mac_address) + String(R"(", "status": "ON"})")).c_str());
}

void MQTT::onMqttSubscribeStatic(uint16_t packetId, uint8_t qos) {
    if (MQTT::instance) {
        MQTT::instance->onMqttSubscribe(packetId, qos);
    }
}

void MQTT::onMqttSubscribe(uint16_t packetId, uint8_t qos) {
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
    Serial.print(F("Subscribed to topic: "));
    Serial.println(last_subscribed_topic);
}

void MQTT::onMqttMessageStatic(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len,
                               size_t index,
                               size_t total) {
    if (MQTT::instance) {
        MQTT::instance->onMqttMessage(topic, payload, properties, len, index, total);
    }
}

void
MQTT::onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index,
                    size_t total) {
//    String message(payload, len); // This will create a string with the correct length
//    // Now print message instead of payload.
//    Serial.print("Publish received: ");
//    Serial.println(message);
    Serial.println();
    Serial.print("Publish received: ");
    Serial.println(payload);
    Serial.print("  topic: ");
    Serial.println(topic);
    Serial.print("  qos: ");
    Serial.println(properties.qos);
    Serial.print("  dup: ");
    Serial.println(properties.dup);
    Serial.print("  retain: ");
    Serial.println(properties.retain);
    Serial.print("  len: ");
    Serial.println(len);
    Serial.print("  index: ");
    Serial.println(index);
    Serial.print("  total: ");
    Serial.println(total);

    handle_incoming_message(topic, payload, properties, len, index, total);
}

bool
MQTT::connect_to_broker(const char *server_ip, int server_port, const char *_lwt_topic, const String &_mac_address) {
    lwt_topic = _lwt_topic + _mac_address;
    mac_address = _mac_address;
    lwt_payload = String(R"({"mac": ")") + mac_address + String(R"(", "status": "OFF"})");

    mqttClient.setServer(server_ip, server_port);
    mqttClient.setWill(lwt_topic.c_str(), 0, true, lwt_payload.c_str(), 0);
    mqttClient.onConnect(MQTT::onMqttConnectStatic);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(MQTT::onMqttMessageStatic);
    mqttClient.onSubscribe(MQTT::onMqttSubscribeStatic);
    Serial.println(F("Connecting to MQTT broker..."));
    mqttClient.connect();
    return true;
}

bool MQTT::subscribe_topic(const char *topicName) {
    if (mqttClient.connected()) {
        last_subscribed_topic = String(topicName);
        Serial.print(F("Subscribe to : "));
        Serial.println(topicName);
        mqttClient.subscribe(topicName, 0);
        return true;
    }
    Serial.println(F("Not connected to broker"));
    return false;
}

bool MQTT::publish_message(char *topicName) {
    return true;
}

void MQTT::handle_incoming_message(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index,
                                   size_t total) {
    String raw_last_payload = String(payload);
    for (int i = 0; i < raw_last_payload.length(); i++) {
        if (!isspace(raw_last_payload[i])) {
            last_payload += raw_last_payload[i];
        }
    }

    if (String(topic) == "rfid/mes/" + mac_address) {
        String _type = extract_value_from_json_string(raw_last_payload, "type");
        Serial.println("Extracted MES type: " + _type);
        if ((expected_event == MES_PACKAGE_SELECTED) and (_type == "MES-PACKAGE")) {
            Serial.println(F("MES package message has arrived"));
            is_mes_package_selected = true;

            // Extract MES package, operation name, target and img url
            mes_package = extract_value_from_json_string(raw_last_payload, "mesKey");
            mes_operation_name = extract_value_from_json_string(raw_last_payload, "opName");
            mes_img_url = extract_value_from_json_string(raw_last_payload, "urlImage");
            mes_target = extract_value_from_json_string(raw_last_payload, "mxTarget").toInt();

            ao_no = extract_value_from_json_string(raw_last_payload, "aono");
            target_qty = extract_value_from_json_string(raw_last_payload, "targetQty");
            delivery_date = extract_value_from_json_string(raw_last_payload, "deliveryDate");
            destination = extract_value_from_json_string(raw_last_payload, "destination");
            style_text = extract_value_from_json_string(raw_last_payload, "styleText");
            buyer_style_text = extract_value_from_json_string(raw_last_payload, "buyerStyleText");
            line_no = extract_value_from_json_string(raw_last_payload, "lineNo");
            style_color = extract_value_from_json_string(raw_last_payload, "styleColorWays");
            buyer_po = extract_value_from_json_string(raw_last_payload, "buyerPO");
            module_name = extract_value_from_json_string(raw_last_payload, "moduleName");

            // Print the extracted values
            Serial.print(F("Extracted MES package: "));
            Serial.println(mes_package);
            Serial.print(F("Extracted MES operation name: "));
            Serial.println(mes_operation_name);
            Serial.print(F("Extracted MES img url: "));
            Serial.println(mes_img_url);
            Serial.print(F("Extracted MES target: "));
            Serial.println(mes_target);
            Serial.print(F("Extracted MES module name: "));
            Serial.println(module_name);
        }
        if ((expected_event == MES_PACKAGE_GROUP_SELECTED) and (_type == "MES-PACKAGEGROUP")) {
            Serial.println(F("MES package group message has arrived"));

            is_mes_package_group_selected = true;

            // Extract MES package, operation name, target and img url
            mes_package_group = extract_value_from_json_string(raw_last_payload, "packageGroup");
            mes_img_url = extract_value_from_json_string(raw_last_payload, "imageUrl");

            ao_no = extract_value_from_json_string(raw_last_payload, "aono");
            style_text = extract_value_from_json_string(raw_last_payload, "style");
            buyer_style_text = extract_value_from_json_string(raw_last_payload, "buyer");

            // Print the extracted values
            Serial.print(F("Extracted MES package group: "));
            Serial.println(mes_package_group);
            Serial.print(F("Extracted MES img url: "));
            Serial.println(mes_img_url);
        }

        expected_event = NONE;
    }
}

void MQTT::wait_for_mqtt_event(mqtt_event_t _event){
    expected_event = _event;
//    switch (_event) {
//        case MES_PACKAGE_SELECTED:
//            while (!is_mes_package_selected) {
//                // Wait for message arrives to topic rfid/mes/{mac_address}
//                //Serial.println(F("Waiting for MES package selection"));
//                yield();
//            }
//            Serial.println(F("MES_PACKAGE_SELECTED event has arrived"));
//            break;
//        case MES_PACKAGE_GROUP_SELECTED:
//            while (!is_mes_package_group_selected) {
//                // Wait for message arrives to topic rfid/mes/{mac_address}
//                //Serial.println(F("Waiting for MES package group selection"));
//                yield();
//            }
//            Serial.println(F("MES_PACKAGE_GROUP_SELECTED event has arrived"));
//            break;
//    }
}

String MQTT::extract_value_from_json_string(const String& data, const String& key) {
    int start = data.indexOf(key);
    if (start != -1) {
        start += key.length() + 2;  // Move past the key and the two quote characters and colon
        int end = data.indexOf('\"', start + 1);  // Find the closing quote character
        if (end != -1) {
            String value = data.substring(start + 1, end);
            return value;
        }
    }
    return "";  // Key not found or value isn't extracted
}
