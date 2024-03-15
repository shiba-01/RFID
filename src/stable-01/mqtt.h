//
// Created by Tan on 30-Oct-23.
//

#ifndef RFID_MQTT_H
#define RFID_MQTT_H

#include <cstdint>
#include "Arduino.h"
#include "structs.h"
#include "AsyncMqttClient.h"

extern AsyncMqttClient mqttClient;

class MQTT {
    static MQTT *instance;
private:
    const char *device_name;

    String mac_address;
    String lwt_topic;
    String lwt_payload;
    String last_subscribed_topic;
public:
    String last_payload;
    String mes_package;
    String mes_package_group;
    String mes_operation_name;
    String mes_img_url;
    String ao_no;
    String target_qty;
    String delivery_date;
    String destination;
    String style_text;
    String buyer_style_text;
    String line_no;
    String style_color;
    String buyer_po;
    String module_name;

    int mes_target;

    bool is_broker_connected;
    bool is_mes_package_selected;
    bool is_mes_package_group_selected;

    mqtt_event_t expected_event;

    MQTT();

    static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);

    static void onMqttConnectStatic(bool sessionPresent);

    void onMqttConnect(bool sessionPresent);

    static void onMqttSubscribeStatic(uint16_t packetId, uint8_t qos);

    void onMqttSubscribe(uint16_t packetId, uint8_t qos);

    static void
    onMqttMessageStatic(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index,
                  size_t total);

    void
    onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index,
                  size_t total);

    bool connect_to_broker(const char *server_ip, int server_port, const char *_lwt_topic, const String &_mac_address);

    bool subscribe_topic(const char *topicName);

    static bool publish_message(char *topicName);

    void handle_incoming_message(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index,
                                 size_t total);

    void wait_for_mqtt_event(mqtt_event_t _event);

    static String extract_value_from_json_string(const String &data, const String &key);
};

#endif //RFID_MQTT_H
