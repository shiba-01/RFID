//
// Created by Tan on 25-Oct-23.
//

#include "wifi_esp32.h"

AsyncWebServer async_server(80);

Wifi::Wifi() {
    currentApWifiSSID = nullptr;
    currentApWifiPassword = nullptr;
    currentStaWifiSSID = nullptr;
    currentStaWifiPassword = nullptr;
}

void Wifi::init_ap_mode() {
    // Code to initialize the device in AP mode
    WiFi.mode(WIFI_AP);
    if (WiFi.softAP(currentApWifiSSID, currentApWifiPassword)) {
        Serial.println(F("Init AP Wi-Fi successfully"));
        Serial.print(F("Wi-Fi AP IP is: "));
        Serial.println(WiFi.softAPIP());
        is_sta_mode_enabled = false;
        is_ap_mode_enabled = true;
        wait_for_new_wifi_setting();
    } else {
        Serial.println(F("Init AP Wi-Fi failed"));
    }
}

bool Wifi::init_sta_mode() {
    // Code to initialize the device in STA (client) mode
    WiFi.setHostname(currentHostname);
    // Set the Wi-Fi mode based on the AP mode flag
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
    is_sta_mode_enabled = true;

    Serial.println("[WiFi] WiFi is connected!");
    Serial.print("[WiFi] IP address: ");
    Serial.println(WiFi.localIP());
    return true;
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
    Serial.println(F("Terminate AP Wi-Fi"));
    WiFi.softAPdisconnect(true);
    is_ap_mode_enabled = false;
}

void Wifi::terminate_sta_mode() {
    // Code to terminate STA mode
    WiFi.disconnect(true);
    is_sta_mode_enabled = false;
}

String Wifi::get_mac_addr() {
    Serial.print(F("Mac address: "));
    Serial.println(WiFi.macAddress());
    mac_address = WiFi.macAddress();
    return WiFi.macAddress();
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void Wifi::handle_setting_new_wifi_connection(AsyncWebServerRequest *request) {
    if (request->hasParam("ssid") && request->hasParam("password")) {
        is_default_sta_wifi_credential_used = false;

        String ssid = request->getParam("ssid")->value();
        String password = request->getParam("password")->value();

        // Copy the credentials into the class member variables
        ssid.toCharArray(ssidArray, sizeof(ssidArray));
        password.toCharArray(passwordArray, sizeof(passwordArray));

        // Call the function to set the STA Wi-Fi credentials
        set_sta_wifi_credential(ssidArray, passwordArray, currentHostname);

        bool check = init_sta_mode();
        // Initialize STA mode with the new credentials
        if (check) {
            Serial.println(F("Done"));
            request->send(200, "text/plain", "STA mode initialized with SSID = " + ssid + " & Password = " + password);
            async_server.end();
        } else {
            Serial.println(F("Failed"));
            async_server.end();
            terminate_ap_mode();
            // request->send(500, "text/plain", "Failed to initialize STA mode");
        }
    } else {
        // In case the SSID or Password was not provided
        request->send(400, "text/plain", "SSID and Password parameters are required");
    }
}

void Wifi::wait_for_new_wifi_setting() {
    async_server.serveStatic("/js", SPIFFS, "/js/");
    async_server.serveStatic("/css", SPIFFS, "/css/");
    async_server.serveStatic("/img", SPIFFS, "/img/");
    async_server.serveStatic("/", SPIFFS, "/html").setDefaultFile("index.html");

    async_server.onNotFound(notFound);

    // Send a GET request to <IP>/?ssid=<ssid>&password=<password>
    // http://192.168.4.1/set-wifi-connection?ssid=ERPLTD&password=erp@@2020
    // http://192.168.4.1/set-wifi-connection?ssid=kiri&password=101conchodom
    async_server.on("/set-wifi-connection", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handle_setting_new_wifi_connection(request);
    });

    async_server.begin();
}