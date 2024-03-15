//
// Created by Tan on 25-Oct-23.
//

#include "wifi_esp32.h"

Wifi::Wifi() {
    currentApWifiSSID = nullptr;
    currentApWifiPassword = nullptr;
    currentStaWifiSSID = nullptr;
    currentStaWifiPassword = nullptr;
}

bool Wifi::init_ap_mode() const {
    // Code to initialize the device in AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP(currentApWifiSSID, currentApWifiPassword);
    return WiFi.softAPIP() != INADDR_NONE;
}

bool Wifi::init_sta_mode() const {
    // Code to initialize the device in STA (client) mode
    WiFi.setHostname(currentHostname);
    WiFi.mode(WIFI_STA);
    WiFi.begin(currentStaWifiSSID, currentStaWifiPassword);
    // We'll wait up to 10 seconds for a connection
    unsigned long startTime = millis();
    const unsigned long timeout = 10000; // 10 seconds timeout

    // Keep checking the status until we're connected or until the timeout
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime >= timeout) {
            Serial.println("[WiFi] Failed to connect within the timeout period.");
            return false;
        }

        delay(500); // Wait half a second before checking again

        // You can handle different cases here if you want to provide detailed feedback
        if (WiFi.status() == WL_NO_SSID_AVAIL) {
            Serial.println("[WiFi] SSID not found");
            return false;
        } else if (WiFi.status() == WL_CONNECT_FAILED) {
            Serial.println("[WiFi] Connection failed");
            return false;
        } else if (WiFi.status() == WL_CONNECTION_LOST) {
            Serial.println("[WiFi] Connection lost");
            return false;
        }
    }

    // If we get here, we are connected
    Serial.println("[WiFi] WiFi is connected!");
    Serial.print("[WiFi] IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

bool Wifi::init_mdns() {
    // Code to initialize mDNS service
    // if (MDNS.begin("esp8266")) {
    //     Serial.println("MDNS responder started");
    //     return true;
    // }
    // return false;
    return true; // Placeholder return value
}

void Wifi::init_web_page() {
    // Code to initialize web server or web page
    // You may need to start a web server and define route handlers
    // webServer.on("/", HTTP_GET, [this]() { handleRoot(); });
    // webServer.begin();
}

void Wifi::set_ap_wifi_credential(char *ssid, char *password) {
    currentApWifiSSID = ssid;
    currentApWifiPassword = password;
    char buffer[100];
    sprintf(buffer, "Set new AP Wifi credential to ssid: %s, password: %s", ssid, password);
    Serial.println(buffer);
}

void Wifi::set_sta_wifi_credential(char *ssid, char *password, char *hostname) {
    currentStaWifiSSID = ssid;
    currentStaWifiPassword = password;
    currentHostname = hostname;
    char buffer[100];
    sprintf(buffer, "Set new STA Wifi credential to ssid: %s, password: %s, hostname: %s", ssid, password, hostname);
    Serial.println(buffer);
}

void Wifi::terminate_ap_mode() {
    // Code to terminate AP mode
    WiFi.softAPdisconnect(true);
}

void Wifi::terminate_sta_mode() {
    // Code to terminate STA mode
    WiFi.disconnect(true);
}

void Wifi::terminate_web_page() {
    // Code to terminate the web server or web page
    // webServer.stop();
}

int Wifi::scan_wifi_networks() {
    // Set WiFi to station mode and disconnect from an AP if it was previously connected.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // WiFi.scanNetworks will return the number of networks found.
    wifi_networks_count = WiFi.scanNetworks();
    Serial.println("Scan Wi-Fi networks done");
    if (wifi_networks_count == 0) {
        Serial.println("No Wi-Fi networks found");
    } if (wifi_networks_count == WIFI_SCAN_FAILED) {
        Serial.println("Wi-Fi scan failed");
        // Handle the scan failure (e.g., by returning an error code)
        return -1;
    } else if (wifi_networks_count == WIFI_SCAN_RUNNING) {
        Serial.println("Wi-Fi scan is still running");
        // Handle the ongoing scan appropriately
        return -1;
    } else if (wifi_networks_count == WL_SCAN_COMPLETED) {
        Serial.println("Wi-Fi scan completed");
        // Normal operation would continue from here
    } else {
        Serial.print(F("Found "));
        Serial.print(wifi_networks_count);
        Serial.println(F(" available Wi-Fi networks"));

        Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
        for (int i = 0; i < wifi_networks_count && i < 10; ++i) {
            // Store the SSID in the wifi_networks array
            strncpy(wifi_networks[i].ssid, WiFi.SSID(i).c_str(), sizeof(wifi_networks[i].ssid) - 1);
            wifi_networks[i].ssid[sizeof(wifi_networks[i].ssid) - 1] = '\0'; // Ensure null-termination
            // Store RSSI in the wifi_networks array
            wifi_networks[i].rssi = WiFi.RSSI(i);

            // Print SSID and RSSI for each network found
            Serial.printf("%2d", i + 1);
            Serial.print(" | ");
            Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
            Serial.print(" | ");
            Serial.printf("%4d", WiFi.RSSI(i));
            Serial.print(" | ");
            Serial.printf("%2d", WiFi.channel(i));
            Serial.print(" | ");
            switch (WiFi.encryptionType(i)) {
                case WIFI_AUTH_OPEN:
                    Serial.print("open");
                    break;
                case WIFI_AUTH_WEP:
                    Serial.print("WEP");
                    break;
                case WIFI_AUTH_WPA_PSK:
                    Serial.print("WPA");
                    break;
                case WIFI_AUTH_WPA2_PSK:
                    Serial.print("WPA2");
                    break;
                case WIFI_AUTH_WPA_WPA2_PSK:
                    Serial.print("WPA+WPA2");
                    break;
                case WIFI_AUTH_WPA2_ENTERPRISE:
                    Serial.print("WPA2-EAP");
                    break;
                case WIFI_AUTH_WPA3_PSK:
                    Serial.print("WPA3");
                    break;
                case WIFI_AUTH_WPA2_WPA3_PSK:
                    Serial.print("WPA2+WPA3");
                    break;
                case WIFI_AUTH_WAPI_PSK:
                    Serial.print("WAPI");
                    break;
                default:
                    Serial.print("unknown");
            }
            Serial.println();
            delay(10);
        }
    }
    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();

    return wifi_networks_count;
}

String Wifi::get_mac_addr() {
    Serial.print(F("Mac address: "));
    Serial.println(WiFi.macAddress());
    mac_address = WiFi.macAddress();
    return WiFi.macAddress();
}