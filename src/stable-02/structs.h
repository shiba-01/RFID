//
// Created by Tan on 31-Oct-23.
//

#ifndef RFID_STRUCTS_H
#define RFID_STRUCTS_H

#include "enums.h"

// Struct Definitions

typedef struct {
    const char *name;
    const uint16_t *icon_data;
} menu_icon;

typedef struct {
    char ssid[16];
    int rssi;
    char password[32];
    char hostname[32];
    char ip[32];
} wifi_network_info;

typedef struct {
    String epc = "";
    bool is_matched_check = false;
} rfid_tag;

typedef struct {
    int status_code;
    int content_length;
    String payload;
} http_response;

// Struct for positions of items on a screen
typedef struct {
    int x;
    int y;
    int w;
    int h;
} screen_item_position;

// Struct for screen selector state
typedef struct {
    screen_selector_t type;
    screen_item_position old_position;
    screen_item_position current_position;
    byte screen_item_index;
} screen_selector;

// Struct for task arguments
typedef struct {
    task_t task, previousTask;
    feature_t feature, previousFeature;
    operating_mode_t operatingMode;
    char wifi_ap_ssid[32];
    char wifi_ap_password[32];
    char wifi_sta_ssid[32];
    char wifi_sta_password[32];
    char wifi_hostname[32];
    char *mqtt_topic;
    const char *mqttBrokerIp;
    const char *mqttLwtTopic;
    String mqtt_subscribed_topic;
    String mes_api_host = "";
    int mqttBrokerPort;
    byte blinkLedPin;
    rfid_scanning_mode_t scanning_mode;
} task_args;

// Struct for storing task results and state
typedef struct {
    bool isFsLoaded;
    bool isMuted;
    operating_mode_t savedOperatingModeInFs;
    operating_mode_t currentOperatingMode;
    feature_t currentFeature;
    task_t currentTask;
    byte currentScreenItemIndex;
    byte screenItemCount;
    feature_item_type_t feature_item_type;
    feature_t screenFeatures[10];
    task_t screenTasks[10];
    task_t screenBackgroundTasks[10];
    feature_t featureNavigationHistory[10] = {NO_FEATURE};
    byte featureNavigationHistorySize = 0;
    int wifi_networks_count;
    wifi_network_info wifi_networks[10];
    String mac_address;
    // For scanning options
    String selected_list_items[10] = {""};
    // For MQTT events
    String selected_mes_package = "";
    String selected_mes_package_group = "";
    String mes_operation_name = "";
    String mes_img_url = "";
    String ao_no = "";
    String target_qty = "";
    String delivery_date = "";
    String destination = "";
    String style_text = "";
    String buyer_style_text = "";
    String line_no = "";
    String style_color = "";
    String buyer_po = "";
    String module_name = "";
    uint16_t mes_img_buffer[9800];
    int mes_img_buffer_size = sizeof(mes_img_buffer) / sizeof(mes_img_buffer[0]);
    int mes_target = 0;
    // For RFID
    bool is_the_first_scan = false;
    int current_scanned_rfid_tag_count = 0;
    int current_matched_mes_scanned_rfid_tag_count = 0;
    int registered_rfid_tags_from_server_count = 0;
    rfid_tag scanned_rfid_tags[200];
    rfid_tag registered_rfid_tags_from_server[200];
} task_results;

#endif //RFID_STRUCTS_H