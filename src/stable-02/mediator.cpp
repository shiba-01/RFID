//
// Created by Tan on 25-Oct-23.
//

#include "mediator.h"

Wifi wifi;
Request request;
Display display;
Peripherals peripherals;
Buzzer buzzer;
Rfid rfid;
MQTT mqtt;

Mediator::Mediator() {
    isTaskExecutable = false;
    isTaskCompleted = true;

    taskResults.isMuted = true;
    taskResults.currentFeature = NO_FEATURE;
    taskResults.currentTask = NO_TASK;
    taskArgs.task = IDLE;
    taskResults.currentScreenItemIndex = 0;
    taskResults.featureNavigationHistory[++taskResults.featureNavigationHistorySize] = HOME_HANDHELD_2;

    Serial.println("Mediator initiated");
    //dataRow.timestamp = NULL;
}

void Mediator::init_services() {
    // Set buzzer pin
    peripherals.set_digital_output(buzzerPinDefinition);
    buzzer.init(buzzerPinDefinition);
    buzzer.mute(taskResults.isMuted);
    // Render layout based on operating mode
    if (taskArgs.feature == HOME_TERMINAL) {
        display.init(LANDSCAPE, taskResults);
    } else {
        display.init(PORTRAIT, taskResults);
    }
    // Play welcome sound using buzzer
    buzzer.welcome_sound();
    display.render_feature(LOADING, taskResults);
    peripherals.init_navigation_buttons(leftUpNavButtonPinDefinition, backCancelNavButtonPinDefinition,
                                        gunButtonPinDefinition, rightDownNavButtonPinDefinition);
    // Check RFID module
    rfid.init(rfid_rx_pin, rfid_tx_pin);
    // Stop RFID module if it still is scanning
    rfid.stop_scanning();
    // Get mac address
    wifi.get_mac_addr();
    taskResults.mac_address = wifi.mac_address;
}

void Mediator::execute_task(task_t task) {
    switch (task) {
        case IDLE:
            Serial.println(F("Idling"));
            //Do nothing in idle mode until nav button is pressed
            //isTaskCompleted = true;
            break;
        case BLINK_LED:
            Serial.println(F("Execute task BLINK_LED"));
            peripherals.blink_led(taskArgs.blinkLedPin);
            isTaskCompleted = true;
            break;
        case BLINK_SCREEN:
            Serial.println(F("Execute task BLINK_SCREEN"));
            //display.blink_screen(isTaskCompleted);
            break;
        case READ_SERIAL_COMMUNICATION_MESSAGE:
            //Serial.println(F("Execute task READ_COMMUNICATION_MESSAGE"));

            break;
        case SEND_SERIAL_COMMUNICATION_MESSAGE:
            Serial.println(F("Execute task SEND_COMMUNICATION_MESSAGE"));

            break;
        case INIT_MESSAGE_QUEUE:
            Serial.println(F("Execute task INIT_MESSAGE_QUEUE"));
            break;
        case CLEAR_MESSAGE_QUEUE:
            Serial.println(F("Execute task CLEAR_MESSAGE_QUEUE"));
            break;
        case GET_MQTT_CONFIG_FROM_SERVER: {
            Serial.println(F("Execute task GET_MQTT_CONFIG_FROM_SERVER"));
            http_response mqtt_config_response = request.get(tpm_server_url, get_mqtt_config,
                                                             get_mqtt_config_query + taskResults.mac_address, "keyCode",
                                                             "PkerpVN2024*", false, 0, 0);

            taskArgs.mqttBrokerIp = extract_value_from_json_string(mqtt_config_response.payload.c_str(),
                                                                   "\"tcpServer\"");
            taskArgs.mqttBrokerPort = atoi(
                    extract_value_from_json_string(mqtt_config_response.payload.c_str(), "\"port\""));
            taskArgs.mqttLwtTopic = mqtt_lwt_topic;
            taskArgs.mes_api_host = extract_value_from_json_string(mqtt_config_response.payload.c_str(),
                                                                   "\"mesAPIHost\"");
            // Print the extracted values
            if ((taskArgs.mqttBrokerIp != nullptr) and (taskArgs.mqttBrokerPort != 0)) {
                Serial.print("TCP Server: ");
                Serial.println(taskArgs.mqttBrokerIp);
                Serial.print("Port: ");
                Serial.println(taskArgs.mqttBrokerPort);
                Serial.print("MES API Host: ");
                Serial.println(taskArgs.mes_api_host);

                execute_task(CONNECT_MQTT_BROKER);
                taskArgs.mqtt_subscribed_topic = String(mqtt_mes_selection_topic) + taskResults.mac_address;
                execute_task(SUBSCRIBE_MQTT_TOPIC);
            } else {
                Serial.println("TCP Server not found. Check HTTP response");
            }
            break;
        }
        case PUBLISH_MQTT_MESSAGE:
            Serial.println(F("Execute task PUBLISH_MQTT_MESSAGE"));
            break;
        case SUBSCRIBE_MQTT_TOPIC: {
            Serial.println(F("Execute task SUBSCRIBE_TOPIC"));
            if (!mqtt.is_broker_connected) {
                Serial.println(F("Please wait to connect to MQTT first before subscribing"));
                unsigned long elapsed_time = millis();
                // Wait until MQTT is connected
                while (millis() - elapsed_time <= 2000) {
                    elapsed_time = millis();
                    if (mqtt.is_broker_connected) break;
                }
            }
            mqtt.subscribe_topic(taskArgs.mqtt_subscribed_topic.c_str());
            break;
        }
        case CONNECT_MQTT_BROKER:
            Serial.println(F("Execute task CONNECT_MQTT_BROKER"));
            mqtt.connect_to_broker(taskArgs.mqttBrokerIp, taskArgs.mqttBrokerPort, taskArgs.mqttLwtTopic,
                                   wifi.mac_address);
            break;
        case HANDLE_MQTT_MESSAGE:
            //Serial.println(F("Execute task HANDLE_MQTT_MESSAGE"));
            switch (taskArgs.feature) {
                case QR_CODE_SCANNING: {
                    if (display.qr_code_type == "MES-PACKAGE") {
                        taskResults.selected_mes_package = "";

                        // Wait until the message with according event arrives
                        mqtt.wait_for_mqtt_event(MES_PACKAGE_SELECTED);
                        if (mqtt.is_mes_package_selected) {
                            taskResults.selected_mes_package = mqtt.mes_package;
                            taskResults.mes_operation_name = mqtt.mes_operation_name;
                            taskResults.mes_target = mqtt.mes_target;
                            taskResults.mes_img_url = mqtt.mes_img_url;
                            taskResults.ao_no = mqtt.ao_no;
                            taskResults.target_qty = mqtt.target_qty;
                            taskResults.delivery_date = mqtt.delivery_date;
                            taskResults.destination = mqtt.destination;
                            taskResults.style_text = mqtt.style_text;
                            taskResults.buyer_style_text = mqtt.buyer_style_text;
                            taskResults.line_no = mqtt.line_no;
                            taskResults.style_color = mqtt.style_color;
                            taskResults.buyer_po = mqtt.buyer_po;
                            taskResults.module_name = mqtt.module_name;

                            // Download MES img from url and display it
                            request.get("http://203.113.151.196:8888", get_resized_mes_img,
                                        get_resized_mes_img_query + taskResults.mes_img_url, "", "", true,
                                        taskResults.mes_img_buffer, taskResults.mes_img_buffer_size);
                            // Download registered RFID tags which are associated with this MES package before
                            http_response list_response = request.get(taskArgs.mes_api_host,
                                                                      get_registered_rfid_tag_list,
                                                                      get_registered_rfid_tag_mes_key_query +
                                                                      taskResults.selected_mes_package + "&" +
                                                                      get_registered_rfid_tag_mes_type_query +
                                                                      "MES-PACKAGE", "keyCode",
                                                                      "PkerpVN2024*", false, nullptr, 0);
                            // Play successful sound
                            if (list_response.status_code == HTTP_CODE_OK) {
                                buzzer.successful_sound();

                                // Extract all registered tags into the storing list
                                extract_registered_rfid_tags(list_response.payload);
                            } else {
                                buzzer.failure_sound();
                            }
                            mqtt.is_mes_package_selected = false;
                            // Back to home
                            taskResults.currentFeature = NO_FEATURE;
                            taskArgs.feature = HOME_HANDHELD_2;
                            display.qr_code_type = "";
                            mqtt.reset_saved_data();
                        }
                    } else if (display.qr_code_type == "MES-PACKAGE-GROUP") {
                        taskResults.selected_mes_package_group = "";

                        // Wait until the message with according event arrives
                        mqtt.wait_for_mqtt_event(MES_PACKAGE_GROUP_SELECTED);
                        if (mqtt.is_mes_package_group_selected) {
                            taskResults.selected_mes_package_group = mqtt.mes_package_group;
                            taskResults.mes_operation_name = mqtt.mes_operation_name;
                            taskResults.mes_target = mqtt.mes_target;
                            taskResults.mes_img_url = mqtt.mes_img_url;
                            taskResults.ao_no = mqtt.ao_no;
                            taskResults.target_qty = mqtt.target_qty;
                            taskResults.delivery_date = mqtt.delivery_date;
                            taskResults.destination = mqtt.destination;
                            taskResults.style_text = mqtt.style_text;
                            taskResults.buyer_style_text = mqtt.buyer_style_text;
                            taskResults.line_no = mqtt.line_no;
                            taskResults.style_color = mqtt.style_color;
                            taskResults.buyer_po = mqtt.buyer_po;
                            taskResults.module_name = mqtt.module_name;

                            // Download MES img from url and display it
                            request.get(resized_image_server_url, get_resized_mes_img,
                                        get_resized_mes_img_query + taskResults.mes_img_url, "", "", true,
                                        taskResults.mes_img_buffer, taskResults.mes_img_buffer_size);
                            // Download registered RFID tags which are associated with this MES package before
                            http_response list_response = request.get(taskArgs.mes_api_host,
                                                                      get_registered_rfid_tag_list,
                                                                      get_registered_rfid_tag_mes_key_query +
                                                                      taskResults.selected_mes_package_group + "&" +
                                                                      get_registered_rfid_tag_mes_type_query +
                                                                      "MES-PACKAGEGROUP", "keyCode",
                                                                      "PkerpVN2024*", false, nullptr, 0);
                            // Play successful sound
                            if (list_response.status_code == HTTP_CODE_OK) {
                                buzzer.successful_sound();

                                // Extract all registered tags into the storing list
                                extract_registered_rfid_tags(list_response.payload);
                            } else {
                                buzzer.failure_sound();
                            }
                            mqtt.is_mes_package_group_selected = false;
                            // Back to home
                            taskResults.currentFeature = NO_FEATURE;
                            taskArgs.feature = HOME_HANDHELD_2;
                            display.qr_code_type = "";
                            mqtt.reset_saved_data();
                        }
                    }
                    break;
                }
            }
            break;
        case LOAD_CONFIG:
            Serial.println(F("Execute task LOAD_CONFIG"));
            break;
        case SAVE_CONFIG:
            Serial.println(F("Execute task SAVE_CONFIG"));
            break;
        case LOAD_FS:
            Serial.println(F("Execute task LOAD_FS"));
            break;
        case SAVE_FS:
            Serial.println(F("Execute task SAVE_FS"));
            break;
        case CHECK_WIFI_CONNECTION: {
            static unsigned long last_millis = millis();
            static unsigned long last_reconnect_millis = millis();
            const unsigned long blink_interval = 500;
            const unsigned long reconnect_interval = 5000;
            static bool is_reconnected = false;

            //Serial.println(F("Execute task CHECK_WIFI_CONNECTION"));
            if (WiFi.status() == WL_CONNECTED) {
                // Wi-Fi is connected
                //Serial.println(F("Wi-Fi is connected"));
                display.iconWidth = 16;
                display.iconHeight = 16;
                display.put_icon(294, 10, display.menu_icon_names[23]);

                // Because we have had a reconnection, we need re-subscribe to MQTT broker
                if (is_reconnected) {
                    // Terminate previous AP mode
                    if ((wifi.is_ap_mode_enabled) && (wifi.is_sta_mode_enabled)) {
                        Serial.println(F("Got sta wifi again. Stop AP"));
                        wifi.terminate_ap_mode();
                    }
                    is_reconnected = false;
                    if (taskArgs.mes_api_host != "") {
                        mqtt.is_broker_connected = false;
                        execute_task(CONNECT_MQTT_BROKER);
                        execute_task(SUBSCRIBE_MQTT_TOPIC);
                        buzzer.successful_sound();
                    } else {
                        execute_task(INIT_STA_WIFI);
                    }
                }
            } else {
                if (!is_reconnected) {
                    Serial.println(F("Will reconnect MQTT when sta mode is done again"));
                    buzzer.failure_sound();
                    is_reconnected = true;
                }
                //Serial.println(F("Wi-Fi is not connected"));
                display.iconWidth = 16;
                display.iconHeight = 16;
                // Blink the icon
                unsigned long current_millis = millis();
                if (current_millis - last_millis >= blink_interval) {
                    last_millis = current_millis;
                    display.put_icon(294, 10, display.menu_icon_names[24]);
                }
                if (current_millis - last_millis >= blink_interval / 2) {
                    display.put_icon(294, 10, display.menu_icon_names[46]); // Clear the icon
                }

                // Try to reconnect
                if (current_millis - last_reconnect_millis >= reconnect_interval) {
                    last_reconnect_millis = current_millis;
                    if (wifi.is_sta_mode_enabled) {
                        Serial.println(F("Try to init sta mode now"));
                        wifi.init_sta_mode();
                    }
                }
            }
            break;
        }
        case INIT_AP_WIFI:
            Serial.println(F("Execute task INIT_AP_WIFI"));

            strncpy(taskArgs.wifi_ap_ssid, "RFID-001", sizeof(taskArgs.wifi_ap_ssid));
            //strncpy(taskArgs.wifi_ap_password, "rfid001x", sizeof(taskArgs.wifi_ap_password));
            // Ensure null-termination if the string length equals the buffer size
            taskArgs.wifi_ap_ssid[sizeof(taskArgs.wifi_ap_ssid) - 1] = '\0';
            taskArgs.wifi_ap_password[sizeof(taskArgs.wifi_ap_password) - 1] = '\0';
            wifi.set_ap_wifi_credential(taskArgs.wifi_ap_ssid, taskArgs.wifi_ap_password);
            // Start to connect to Wi-Fi as AP credential
            wifi.init_ap_mode();
            break;
        case INIT_STA_WIFI:
            Serial.println(F("Execute task INIT_STA_WIFI"));

            if (wifi.is_default_sta_wifi_credential_used) {
                Serial.println(F("Default STA Wi-Fi credential is used"));
                strncpy(taskArgs.wifi_sta_ssid, default_wifi_ssid_1, sizeof(taskArgs.wifi_sta_ssid));
                strncpy(taskArgs.wifi_sta_password, default_wifi_password_1, sizeof(taskArgs.wifi_sta_password));
                strncpy(taskArgs.wifi_hostname, device_hostname, sizeof(taskArgs.wifi_hostname));
                // Ensure null-termination if the string length equals the buffer size
                taskArgs.wifi_sta_ssid[sizeof(taskArgs.wifi_sta_ssid) - 1] = '\0';
                taskArgs.wifi_sta_password[sizeof(taskArgs.wifi_sta_password) - 1] = '\0';
                taskArgs.wifi_hostname[sizeof(taskArgs.wifi_hostname) - 1] = '\0';
                wifi.set_sta_wifi_credential(taskArgs.wifi_sta_ssid, taskArgs.wifi_sta_password,
                                             taskArgs.wifi_hostname);
            }

            // Start to connect to Wi-Fi as set STA credential
            if (wifi.init_sta_mode()) {
                Serial.println(F("Init sta wifi successfully"));
                execute_task(GET_MQTT_CONFIG_FROM_SERVER);
                buzzer.successful_sound();
            } else {
                buzzer.failure_sound();
//                Serial.println(F("Init sta Wi-Fi failed. Reset in 3s"));
//                delay(3000);
//                // Reset device
//                ESP.restart();
                Serial.println(F("Init sta wifi failed"));
            }
            break;
        case TERMINATE_AP_WIFI:
            Serial.println(F("Execute task TERMINATE_AP_WIFI"));
            wifi.terminate_ap_mode();
            break;
        case TERMINATE_STA_WIFI:
            Serial.println(F("Execute task TERMINATE_STA_WIFI"));
            wifi.terminate_sta_mode();
            break;
        case GET_OPERATING_MODE:
            Serial.println(F("Execute task GET_OPERATING_MODE"));
//            taskResults.currentOperatingMode = operation.get_operating_mode();
            break;
        case SET_OPERATING_MODE:
            Serial.println(F("Execute task SET_OPERATING_MODE"));
//            operation.set_operation_mode(taskArgs.operatingMode);
            taskResults.currentOperatingMode = taskArgs.operatingMode;
            break;
        case RENDER_FEATURE:
            static bool is_render_forced = false;

            if ((taskArgs.feature != taskResults.currentFeature) or (is_render_forced)) {
                Serial.print(F("Execute task RENDER_FEATURE :"));
                Serial.println(feature_as_string(taskArgs.feature));
                display.render_feature(taskArgs.feature, taskResults);
                // Check if this feature requires background tasks before rendering information, if yes, run tasks,
                // then re-render
                if (display.is_background_task_required) {
                    if (display.is_loading_animation_displayed)
                        display.render_feature(LOADING, taskResults);
                    byte feature_background_task_index = 0;
                    while ((feature_background_task_index <= 9) and
                           (display.current_screen_background_tasks[feature_background_task_index] != NO_TASK)) {
                        Serial.println(F("Executing background task"));
                        execute_task(display.current_screen_background_tasks[feature_background_task_index]);
                        ++feature_background_task_index;
                    }

                    Serial.print(F("This feature has : "));
                    Serial.print(feature_background_task_index);
                    Serial.println(F(" background tasks"));

                    display.is_background_task_completed = true;
                    display.is_background_task_required = false;
                    display.render_feature(taskArgs.feature, taskResults);
                    display.is_background_task_completed = false;
                    display.is_loading_animation_displayed = false;
                    is_render_forced = false;
                }

                if (display.is_back_to_home) {
                    display.is_back_to_home = false;
                    taskResults.currentFeature = NO_FEATURE;
                    taskArgs.feature = HOME_HANDHELD_2;
                    execute_task(RENDER_FEATURE);
                } else {
                    // Update screen item index for screen selector
                    taskResults.currentScreenItemIndex = 0;
                    // Update screen item count for screen selector
                    taskResults.screenItemCount = display.screen_item_count;
                    // Update the type of items on the screen
                    taskResults.feature_item_type = display.current_feature_item_type;
                    // Update the list of features/tasks of items on the screen
                    switch (taskResults.feature_item_type) {
                        case MENU_ICON:
                            for (int i = 0; i < 10; ++i) {
                                taskResults.screenFeatures[i] = display.current_screen_features[i];
                            }
                            break;
                        case LIST_ITEM:
                            taskResults.screenFeatures[0] = display.current_screen_features[0];
                            break;
                        case TASK_ITEM:
                            for (int i = 0; i < 10; ++i) {
                                taskResults.screenTasks[i] = display.current_screen_tasks[i];
                            }
                            break;
                    }
                    // Print screen item count of this feature (screen)
                    Serial.print(F("Feature has : "));
                    Serial.print(taskResults.screenItemCount);
                    Serial.println(F(" items on the screen"));
                }
            } else {
                // The Feature is not changed.
                // Keep current rendering
                //Serial.println(F("Feature is not changed. Keep current rendering"));
            }
            break;
        case INIT_NAVIGATION_BUTTON:
            Serial.println(F("Execute task INIT_NAVIGATION_BUTTON"));
            peripherals.init_navigation_buttons(leftUpNavButtonPinDefinition,
                                                backCancelNavButtonPinDefinition,
                                                gunButtonPinDefinition,
                                                rightDownNavButtonPinDefinition);
            break;
        case READ_NAVIGATION_BUTTON: {
            // Serial.println(F("Execute task READ_NAVIGATION_BUTTON"));
            if (Peripherals::isMenuSelectButtonReleased) {
                Peripherals::isMenuSelectButtonReleased = false;
                if (!isTaskCompleted) {
                    isTaskExecutable = false;
                    isTaskCompleted = true;
                    // Stop the RFID module if it is still working
                    rfid.stop_scanning();
                }
                break;
            }
            // Get navigation direction
            // To store current screen item index with LIST_ITEM type
            byte previous_screen_item_index = taskResults.currentScreenItemIndex;
            button_type_t is_nav_button_pressed = peripherals.read_navigation_buttons(
                    taskResults.currentScreenItemIndex,
                    taskResults.screenItemCount,
                    taskResults.feature_item_type);
            // Clear current screen selector and update to new position from button state
            switch (is_nav_button_pressed) {
                case LEFT_UP:
                    // We just traverse through screen items for both cases
                    if (taskResults.currentFeature == HOME_HANDHELD_2) {
                        if (taskResults.currentScreenItemIndex == 4) {
                            display.screen_selector_border_color = 0x3b2d;
                        }
                        if (taskResults.currentScreenItemIndex == 2) {
                            if (taskResults.screenItemCount == 4) {
                                display.screen_selector_border_color = 0x3b2d;
                            }
                        }
                    }
                    display.clear_screen_selector(taskResults);
                    display.update_screen_selector(taskResults.currentScreenItemIndex);
                    break;
                case RIGHT_DOWN:
                    // We just traverse through screen items for both cases
                    if (taskResults.currentFeature == HOME_HANDHELD_2 && taskResults.currentScreenItemIndex == 0) {
                        display.screen_selector_border_color = 0x3b2d;
                    }
                    display.clear_screen_selector(taskResults);
                    display.update_screen_selector(taskResults.currentScreenItemIndex);
                    break;
                case SELECT:
                    switch (taskResults.feature_item_type) {
                        case MENU_ICON:
                            // Reset navigation history if we render home
                            if (taskResults.currentFeature == HOME_HANDHELD_2) clear_navigation_history();

                            Serial.println(F("Retrieving corresponding feature now"));
                            Serial.print(F("Current screen item index: "));
                            Serial.println(taskResults.currentScreenItemIndex);
                            peripherals.retrieve_corresponding_feature(taskArgs.previousFeature,
                                                                       taskResults.currentFeature, taskArgs.feature,
                                                                       taskResults.currentScreenItemIndex,
                                                                       taskResults.screenFeatures,
                                                                       is_nav_button_pressed,
                                                                       taskResults.featureNavigationHistory,
                                                                       taskResults.featureNavigationHistorySize);

                            // Set screen selector border color accordingly to the next feature
                            display.set_screen_selector_border_color(taskArgs.feature);

                            // Clear setting icon if not in homepage
                            if (taskArgs.feature != HOME_HANDHELD_2) {
                                tft.fillRect(252, 10, 16, 16, display.headerColor);
                            }
                            break;
                        case LIST_ITEM:
                            // When item is selected, start to switch to next screen and execute background task
                            //taskArgs.feature = taskResults.screenFeatures[0];
                            // Append selected item in the list of this screen into the selected list items array
                            if (taskResults.selected_list_items[0] == "") {
                                taskResults.selected_list_items[0]
                                        = display.current_screen_list_items[taskResults.currentScreenItemIndex];
                            } else {
                                byte i = 9;
                                while (i > 0) {
                                    if (taskResults.selected_list_items[i - 1] != "") {
                                        taskResults.selected_list_items[i]
                                                = display.current_screen_list_items[taskResults.currentScreenItemIndex];
                                        break;
                                    }
                                    --i;
                                }
                            }

                            Serial.print(F("Submitted selected item in the list: "));
                            Serial.println(display.current_screen_list_items[taskResults.currentScreenItemIndex]);
                            // Set the current item index to 0 because we only have 1 screen to be rendered next
                            taskResults.currentScreenItemIndex = 0;
                            Serial.println(F("Retrieving corresponding task now"));
                            peripherals.retrieve_corresponding_feature(taskArgs.previousFeature,
                                                                       taskResults.currentFeature, taskArgs.feature,
                                                                       taskResults.currentScreenItemIndex,
                                                                       taskResults.screenFeatures,
                                                                       is_nav_button_pressed,
                                                                       taskResults.featureNavigationHistory,
                                                                       taskResults.featureNavigationHistorySize);

                            Serial.println(F("Selected items from previous lists so far: "));
                            for (byte i = 0; i < 10; ++i) {
                                if (taskResults.selected_list_items[i] != "")
                                    Serial.println(taskResults.selected_list_items[i]);
                            }
                            //peripherals.retrieve_corresponding_task(taskArgs.previousTask, taskResults.currentTask);
                            break;
                        case TASK_ITEM:
                            Serial.println("Task item");
                            // We just execute the task which is associated with the clicked item.
                            // Render to next feature will be done in the task
                            //execute_task(taskResults.screenTasks[taskResults.currentScreenItemIndex]);
                            is_render_forced = true;
                            break;
                    }
                    break;
                case BACK_CANCEL:
                    // If we are in home, no back anymore
                    if (taskResults.currentFeature != HOME_HANDHELD_2) {
                        Peripherals::retrieve_corresponding_feature(taskArgs.previousFeature,
                                                                    taskResults.currentFeature, taskArgs.feature,
                                                                    taskResults.currentScreenItemIndex,
                                                                    taskResults.screenFeatures, is_nav_button_pressed,
                                                                    taskResults.featureNavigationHistory,
                                                                    taskResults.featureNavigationHistorySize);

                        // Set screen selector border color accordingly to the next feature
                        display.set_screen_selector_border_color(taskArgs.feature);
                        if ((taskResults.currentFeature == RFID_SCAN_RESULT) or
                            (taskResults.currentFeature == RFID_REGISTER_TAG) or
                            (taskResults.currentFeature == SETTING)) {
                            display.is_viewport_cleared = true;
                        }
                    } else {
                        clear_navigation_history();
                    }
                case NOT_PRESSED: {
                    isTaskExecutable = false;
                    isTaskCompleted = true;
                    break;
                }
            }
            break;
        }
        case GET_CURRENT_FEATURE:
            Serial.println(F("Execute task GET_CURRENT_FEATURE"));
            taskArgs.feature = taskResults.currentFeature;
            break;
        case SET_CURRENT_FEATURE:
            if (taskArgs.feature != taskResults.currentFeature) {
                taskArgs.previousFeature = taskResults.currentFeature;
                taskResults.currentFeature = taskArgs.feature;
            }
            break;
        case SET_TASK:
            Serial.println(F("Execute task SET_TASK"));
            taskResults.currentTask = taskArgs.task;
            break;
        case SET_TASK_STATUS:
            Serial.println(F("Execute task SET_TASK_STATUS"));
            break;
        case GET_TASK_STATUS:
            Serial.println(F("Execute task GET_TASK_STATUS"));
            break;
        case GET_RFID_SCAN_DATA:
            Serial.println(F("Execute task GET_RFID_SCAN_DATA"));
            break;
        case IMPORT_DATA_FROM_SD_CARD:
            Serial.println(F("Execute task IMPORT_DATA_FROM_SD_CARD"));
            break;
        case IMPORT_DATA_FROM_SERVER:
            Serial.println(F("Execute task IMPORT_DATA_FROM_SERVER"));
            break;
        case EXPORT_DATA_TO_SD_CARD:
            Serial.println(F("Execute task EXPORT_DATA_TO_SD_CARD"));
            break;
        case EXPORT_DATA_TO_SERVER:
            Serial.println(F("Execute task EXPORT_DATA_TO_SERVER"));
            break;
        case SYNC_DATA_TO_SERVER:
            Serial.println(F("Execute task SYNC_DATA_TO_SERVER"));
            break;
        case SYNC_DATA_TO_DEVICE:
            Serial.println(F("Execute task SYNC_DATA_TO_DEVICE"));
            break;
        case READ_RFID_TAG:
            Serial.println(F("Execute task READ_RFID_TAG"));
            // Fresh task running
            if (!isTaskExecutable && isTaskCompleted && !Peripherals::isMenuSelectButtonReleased) {
                Serial.println(F("Fresh task READ_RFID_TAG running"));
                isTaskExecutable = true;
                isTaskCompleted = false;
                // Keep this task run continuously
                taskArgs.task = READ_RFID_TAG;
                set_current_task();

                if (taskResults.is_the_first_scan) {
                    rfid.scanned_tag_count = 0;
                    taskResults.is_the_first_scan = false;
                }
                //rfid.set_scanning_mode(SINGLE_SCAN);
                rfid.set_scanning_mode(MULTI_SCAN);
                rfid.scan_rfid_tag();
            } else if (isTaskExecutable && !isTaskCompleted) {
                // Serial.println(F("Scanning"));
                // We re scanning, read scan result
                rfid.read_multi_scan_response(Peripherals::isMenuSelectButtonReleased);
                // We re scanning, update the counted tags
                if (taskResults.currentFeature == RFID_REGISTER_TAG) {
                    if (taskResults.current_scanned_rfid_tag_count != rfid.scanned_tag_count) {
                        taskResults.current_scanned_rfid_tag_count = rfid.scanned_tag_count;
                        //buzzer.successful_sound();
                        display.update_rfid_registration_scan_result(taskResults);
                    } else {
                        //buzzer.failure_sound();
                    }
                } else if (taskResults.currentFeature == RFID_SCAN_RESULT) {
                    // We first check if the latest scanned tag is in registered tags before (Check MES - matched) -
                    // Default false
                    bool check = false;

                    for (int i = 0; i < 200; ++i) {
                        for (int j = 0; j < 200; ++j) {
                            if (is_epc_matched(rfid.scan_results[i].epc,
                                               taskResults.registered_rfid_tags_from_server[j].epc) and
                                (rfid.scan_results[i].epc != "") and
                                (taskResults.registered_rfid_tags_from_server[j].epc != "") and
                                (!rfid.scan_results[i].is_matched_check)) {
                                rfid.scan_results[i].is_matched_check = true;
                                Serial.print(F("RFID tag is registered before: "));
                                Serial.println(rfid.scan_results[i].epc);
                                check = true;
                                ++taskResults.current_matched_mes_scanned_rfid_tag_count;
                                buzzer.successful_sound();
                                break;
                            }
                        }
                    }
                    // Totally scanned to be displayed
                    if (taskResults.current_scanned_rfid_tag_count != rfid.scanned_tag_count) {
                        taskResults.current_scanned_rfid_tag_count = rfid.scanned_tag_count;
                        display.update_rfid_match_check_scan_result(taskResults);
                    }
                    // If not matched, sound
                    if (!check) {
                        buzzer.failure_sound();
                    }
                }
            }
            break;
        case REGISTER_RFID_TAG: {
            Serial.println(F("Execute task REGISTER_RFID_TAG"));
            String registration_payload = R"({"mesKey": ")" + taskResults.selected_mes_package + R"(","tagIds": [)";
            if (rfid.scanned_tag_count == 0) {
                Serial.println(F("No scanned RFID tag. Back to home now"));
            } else {
                for (int i = 0; i < 199; ++i) {
                    if (rfid.scan_results[i].epc != "") {
                        if ((rfid.scan_results[i + 1].epc != "") and (i != 199)) {
                            registration_payload += "\"" + rfid.scan_results[i].epc + "\",";
                        } else {
                            registration_payload += "\"" + rfid.scan_results[i].epc + "\"]}";
                        }
                    }
                }
                http_response registration_response = request.post(taskArgs.mes_api_host, register_new_rfid_tag,
                                                                   registration_payload, "keyCode",
                                                                   "PkerpVN2024*");
                if (registration_response.status_code == HTTP_CODE_OK) {
                    buzzer.successful_sound();

                    // Update registered tag list
                    // Download registered RFID tags which are associated with this MES package before
                    http_response list_response = request.get(taskArgs.mes_api_host,
                                                              get_registered_rfid_tag_list,
                                                              get_registered_rfid_tag_mes_key_query +
                                                              taskResults.selected_mes_package + "&" +
                                                              get_registered_rfid_tag_mes_type_query +
                                                              "MES-PACKAGE", "keyCode",
                                                              "PkerpVN2024*", false, nullptr, 0);

                    // Extract all registered tags into the storing list
                    extract_registered_rfid_tags(list_response.payload);
                } else {
                    buzzer.failure_sound();
                }
                rfid.scanned_tag_count = 0;
                taskResults.current_scanned_rfid_tag_count = 0;
            }
            break;
        }
        case WRITE_RFID_TAG:
            Serial.println(F("Execute task WRITE_RFID_TAG"));
            break;
        case SET_RFID_SCANNING_MODE:
            Serial.println(F("Execute task SET_RFID_SCANNING_MODE"));
            rfid.set_scanning_mode(taskArgs.scanning_mode);
            break;
        case RESET_SCANNED_RFID_TAG_COUNT: {
            Serial.println(F("RESET_SCANNED_RFID_TAG_COUNT"));
            // Reset scanned RFID tag count, matched-MES scanned RFID tag count
            rfid.scanned_tag_count = 0;
            taskResults.current_scanned_rfid_tag_count = 0;
            taskResults.current_matched_mes_scanned_rfid_tag_count = 0;
            for (int i = 0; i < 200; ++i) {
                taskResults.scanned_rfid_tags[i].epc = "";
                taskResults.scanned_rfid_tags[i].is_matched_check = false;
                rfid.scan_results[i].epc = "";
                rfid.scan_results[i].is_matched_check = false;
            }
            delay(500);
            break;
        }
        case INSERT_DATA_ROW:
            Serial.println(F("Execute task INSERT_DATA_ROW"));
            break;
        case UPDATE_DATA_ROW:
            Serial.println(F("Execute task UPDATE_DATA_ROW"));
            break;
        case DELETE_DATA_ROW:
            Serial.println(F("Execute task DELETE_DATA_ROW"));
            break;
        case SAVE_DATA_COLLECTION:
            Serial.println(F("Execute task SAVE_DATA_COLLECTION"));
            break;
        case DELETE_DATA_COLLECTION:
            Serial.println(F("Execute task DELETE_DATA_COLLECTION"));
            break;
        case LOAD_DATA_COLLECTION:
            Serial.println(F("Execute task LOAD_DATA_COLLECTION"));
            break;
        case READ_PERIPHERAL_INPUT:
            Serial.println(F("Execute task READ_PERIPHERAL_INPUT"));
            break;
        case SEND_PERIPHERAL_OUTPUT:
            Serial.println(F("Execute task SEND_PERIPHERAL_OUTPUT"));
            break;
        case START_CONVEYOR:
            Serial.println(F("Execute task START_CONVEYOR"));
            break;
        case STOP_CONVEYOR:
            Serial.println(F("Execute task STOP_CONVEYOR"));
            break;
        case SUBMIT_CHOSEN_ITEM:
            Serial.println(F("Execute task SUBMIT_CHOSEN_ITEM"));
            break;
        case TOGGLE_SOUND:
            Serial.println(F("Execute task TOGGLE_SOUND"));
            if (taskResults.isMuted) {
                taskResults.isMuted = false;
            } else {
                taskResults.isMuted = true;
            }
            buzzer.mute(taskResults.isMuted);
            break;
    }
}

void Mediator::set_current_task() {
    if (taskArgs.task != NO_TASK) {
        taskResults.currentTask = taskArgs.task;
        Serial.print(F("Set current task to: "));
        Serial.println(task_as_string(taskArgs.task));
    }
}

void Mediator::set_current_feature() {
    if (taskResults.currentFeature != taskArgs.feature) {
        taskResults.currentFeature = taskArgs.feature;
        Serial.print(F("Set current feature successfully to "));
        Serial.println(F(feature_as_string(taskResults.currentFeature)));
    }
}

void Mediator::set_current_task_status(bool taskStatus) {
    if (taskStatus) {
        Serial.println(F("Set current task status to completed"));
    } else {
        Serial.println(F("Set current task status to incomplete"));
    }
    isTaskCompleted = taskStatus;
    isTaskExecutable = !taskStatus;
}

bool Mediator::get_current_task_status() const {
    if (isTaskCompleted) {
        Serial.println(F("Current task status: completed"));
    } else {
        Serial.println(F("Current task status: incomplete"));
    }
    return isTaskCompleted;
}

task_t Mediator::get_current_task() {
    Serial.print(F("Current task is: "));
    Serial.println(task_as_string(taskResults.currentTask));
    return taskResults.currentTask;
}

feature_t Mediator::get_current_feature() const {
//    Serial.print(F("Current feature is: "));
//    Serial.println(F(feature_as_string(taskResults.currentFeature)));
    return taskResults.currentFeature;
}

const char *Mediator::task_as_string(task_t task) {
    return task_names[task];
}

const char *Mediator::feature_as_string(feature_t feature) const {
    return feature_names[feature];
}

const char *Mediator::extract_value_from_json_string(const char *data, const char *key) {
    const char *start = strstr(data, key);
    if (start != nullptr) {
        start += strlen(key) + 2;  // Move past the key and the two quote characters and colon
        const char *end = strchr(start, '\"');  // Find the closing quote character
        size_t length = static_cast<size_t>(end - start);

//        // Create a buffer to store the extracted value
//        static char value[100]; // Adjust the buffer size as per your requirement
//        strncpy(value, start, length);
//        value[length] = '\0';  // Null-terminate the value string
//        Serial.println(value);
//        return value;

        // Allocate memory for the extracted value
        char *value = new char[length + 1];  // +1 for the null terminator
        strncpy(value, start, length);
        value[length] = '\0';  // Null-terminate the value string

        return value;
    }
    return nullptr;  // Key not found
}

void Mediator::clear_navigation_history() {
    // Reset navigation history
    for (byte i = 0; i < 10; ++i) {
        taskResults.featureNavigationHistory[i] = NO_FEATURE;
    }
    taskResults.featureNavigationHistorySize = 1;
    taskResults.featureNavigationHistory[1] = HOME_HANDHELD_2;
}

void Mediator::extract_registered_rfid_tags(String &payload) {
    int startPos = payload.indexOf("\"tagIds\": [") + 11;
    int endPos = payload.lastIndexOf("]");

    String extracted_list = payload.substring(startPos, endPos);
    extracted_list.trim();
    Serial.println(F("Extracted registered tags list: "));
    Serial.println(extracted_list);

    // Counters for going through the list
    int currentPos = 0;
    int nextPos = 0;
    int tagIndex = 0; // Index to keep track of the number of tags extracted

    // Iterate over the string and extract every tag EPC
    while (nextPos != -1 && tagIndex < 200) {
        nextPos = extracted_list.indexOf(',', currentPos);

        // Extract the current tag
        String tag = (nextPos != -1) ? extracted_list.substring(currentPos, nextPos) : extracted_list.substring(
                currentPos);
        tag.trim(); // Trim whitespace
        tag.replace("\"", ""); // Remove quotes

        // Assign the tag to the array of rfid_tag structs
        if (tag != "") {
            taskResults.registered_rfid_tags_from_server[tagIndex].epc = tag;
            // Increment the tag index
            tagIndex++;
        }

        // Move to the next tag
        currentPos = nextPos + 1;
    }

    // Optionally, print out the tags to verify
    taskResults.registered_rfid_tags_from_server_count = tagIndex;
    Serial.println(F("All extracted tag epc(s):"));
    for (int i = 0; i < tagIndex; i++) {
        Serial.println(taskResults.registered_rfid_tags_from_server[i].epc);
    }
}

bool Mediator::is_epc_matched(String &epc_to_be_compared, String &registered_epc) {
    if (epc_to_be_compared.equalsIgnoreCase(registered_epc)) return true;
    return false;
}