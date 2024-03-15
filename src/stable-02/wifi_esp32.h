//
// Created by Tan on 25-Oct-23.
//

#ifndef RFID_WIFI_H
#define RFID_WIFI_H

#include "Arduino.h"
#include <Wifi.h>
#include "structs.h."
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <esp_task_wdt.h>

class Wifi {
private:
    char ssidArray[32];
    char passwordArray[64];
public:
    char *currentApWifiSSID;
    char *currentApWifiPassword;
    char *currentStaWifiSSID;
    char *currentStaWifiPassword;
    char *currentHostname;

    String mac_address;

    int wifi_networks_count;
    wifi_network_info wifi_networks[10];

    bool is_sta_mode_enabled = true;
    bool is_ap_mode_enabled = false;
    bool is_default_sta_wifi_credential_used = true;

    Wifi();

    void init_ap_mode();

    bool init_sta_mode();

    void set_ap_wifi_credential(char *ssid, char *password);

    void set_sta_wifi_credential(char *ssid, char *password, char *hostname);

    void terminate_ap_mode();

    void terminate_sta_mode();

    String get_mac_addr();

    void wait_for_new_wifi_setting();

    void handle_setting_new_wifi_connection(AsyncWebServerRequest *request);
};

#endif //RFID_WIFI_H
