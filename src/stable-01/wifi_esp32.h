//
// Created by Tan on 25-Oct-23.
//

#ifndef RFID_WIFI_H
#define RFID_WIFI_H

#include "Arduino.h"
#include <Wifi.h>
#include "structs.h."

class Wifi {
public:
    char *currentApWifiSSID;
    char *currentApWifiPassword;
    char *currentStaWifiSSID;
    char *currentStaWifiPassword;
    char *currentHostname;

    String mac_address;

    int wifi_networks_count;
    wifi_network_info wifi_networks[10];

    Wifi();

    bool init_ap_mode() const;

    bool init_sta_mode() const;

    bool init_mdns();

    void init_web_page();

    void set_ap_wifi_credential(char *ssid, char *password);

    void set_sta_wifi_credential(char *ssid, char *password, char *hostname);

    static void terminate_ap_mode();

    static void terminate_sta_mode();

    int scan_wifi_networks();

    void terminate_web_page();

    String get_mac_addr();
};

#endif //RFID_WIFI_H
