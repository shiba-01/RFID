//
// Created by Tan on 25-Oct-23.
//

#ifndef RFID_DISPLAY_H
#define RFID_DISPLAY_H

#include "Arduino.h"
#include "qrcode_espi.h"
#include "GIFDraw.h"
#include "icons/loading_animation.h"
#include "AnimatedGIF.h"
#include "enums.h"
#include "structs.h"
#include "SPI.h"
#include "TFT_eSPI.h"
#include "icons/connect-to-device.h"
#include "icons/connect-to-server.h"
#include "icons/co-working.h"
#include "icons/scan-nearby-devices.h"
#include "icons/export.h"
#include "icons/rfid.h"
#include "icons/scan.h"
#include "icons/scan-history.h"
#include "icons/modify-rfid-tag.h"
#include "icons/register_rfid_tag.h"
#include "icons/setting.h"
#include "icons/wifi-setting.h"
#include "icons/user.h"
#include "icons/login.h"
#include "icons/logout.h"
#include "icons/import.h"
#include "icons/import-from-sd-card.h"
#include "icons/import-from-server.h"
#include "icons/import-from-computer.h"
#include "icons/package.h"
#include "icons/sync-data.h"
#include "icons/database-setting.h"
#include "icons/package-details.h"
#include "icons/wifi-connection-successful.h"
#include "icons/wifi-connection-failed.h"
#include "icons/server-connection-successful.h"
#include "icons/server-connection-failed.h"
#include "icons/register_rfid_tag_button.h"
#include "icons/setting_button.h"
#include "icons/incoming-packed-boxes-banner.h"
#include "icons/outgoing-packed-boxes-banner.h"
#include "icons/product-counting-banner.h"
#include "icons/incoming-box.h"
#include "icons/outgoing-box.h"
#include "icons/green_tick_icon.h"
#include "icons/red_x_icon.h"
#include "icons/qr-code-placeholder-banner.h"
#include "icons/blurred_register_rfid_tag_button.h"
#include "icons/blurred_scan_button.h"
#include "icons/mes_package_dialog_banner.h"
#include "icons/start_scanning_button.h"
#include "icons/submit_button.h"
#include "icons/scan_button.h"
#include "icons/text_scan_button.h"
#include "icons/company_logo.h"
#include "icons/clear_button.h"

extern TFT_eSPI tft;

void GIFDraw(GIFDRAW *pDraw);

class Display {
public:
    feature_layout_t feature_layout;
    feature_item_type_t current_feature_item_type;
    feature_t current_screen_features[10];

    task_t current_screen_tasks[10];
    task_t current_screen_background_tasks[10];

    screen_selector current_screen_selector;
    screen_item_position screen_items[10];

    bool is_background_task_required = false;
    bool is_background_task_completed = false;
    bool is_loading_animation_displayed = false;
    bool is_back_to_home = false;
    bool is_viewport_cleared = true;

    int SCREEN_WIDTH = 320;
    int SCREEN_HEIGHT = 480;

    byte screen_item_count;

    String current_screen_list_items[8] = {""};

    // For QR code
    String qr_code_type = "";

    const byte HEADER_HEIGHT = 36;
    const byte NAV_BAR_HEIGHT = HEADER_HEIGHT;
    const byte VIEWPORT_HEIGHT = HEADER_HEIGHT - NAV_BAR_HEIGHT;

    // Define colors for different UI elements
    uint32_t headerColor = 0x3B2D;
    uint32_t backgroundColor = 0x84B2;
    uint16_t screen_selector_border_color = backgroundColor;

    // Placeholder text for various status indicators
    const char *wifiStatus = "WiFi: Connected";
    const char *dateTime = "2023-12-20 14:30";
    const char *serverStatus = "Server: Connected";
    const char *loginStatus = "User: Logged In";

    // Constants for the grid layout
    int iconWidth = 64; // Width of the icon
    int iconHeight = 64; // Height of the icon
    static const byte numIcons = 47; //  Number of icons

    // Define an array of menu icon names corresponding to the header files
    const char *menu_icon_names[numIcons] = {
            "setting_icon",
            "rfid_icon",
            "package_icon",
            "co-working_icon",
            "database-setting_icon",
            "sync-data_icon",
            "connect-to-device_icon",
            "connect-to-server_icon",
            "scan-nearby-devices_icon",
            "scan_icon",
            "scan-history_icon",
            "modify-rfid-tag_icon",
            "wifi-setting_icon",
            "user_icon",
            "login_icon",
            "logout_icon",
            "import_icon",
            "export_icon",
            "import-from-sd-card_icon",
            "import-from-server_icon",
            "import-from-computer_icon",
            "package-details_icon",
            "register_rfid_tag_icon",
            "wifi_connection_successful_icon",
            "wifi_connection_failed_icon",
            "server_connection_successful_icon",
            "server_connection_failed_icon",
            "register_rfid_tag_button_icon",
            "setting_button_icon",
            "scan_button_icon",
            "incoming_packed_boxes_banner_icon",
            "outgoing_packed_boxes_banner_icon",
            "product_counting_banner_icon",
            "incoming_box_icon",
            "outgoing_box_icon",
            "green_tick_icon",
            "red_x_icon",
            "qr_code_placeholder_banner_icon",
            "blurred_register_rfid_tag_button_icon",
            "blurred_scan_button_icon",
            "mes_package_dialog_banner_icon",
            "start_scanning_button_icon",
            "text_scan_button_icon",
            "clear_button_icon",
            "submit_button_icon",
            "company_logo_icon",
    };

    // Map menu names to menu icon data arrays
    menu_icon icons[numIcons] = {
            {"co-working_icon",                       co_working_icon},
            {"connect-to-device_icon",                connect_to_device_icon},
            {"connect-to-server_icon",                connect_to_server_icon},
            {"scan-nearby-devices_icon",              scan_nearby_devices_icon},
            {"rfid_icon",                             rfid_icon},
            {"scan_icon",                             scan_icon},
            {"scan-history_icon",                     scan_history_icon},
            {"modify-rfid-tag_icon",                  modify_rfid_tag_icon},
            {"setting_icon",                          setting_icon},
            {"wifi-setting_icon",                     wifi_setting_icon},
            {"user_icon",                             user_icon},
            {"login_icon",                            login_icon},
            {"logout_icon",                           logout_icon},
            {"database-setting_icon",                 database_setting_icon},
            {"import_icon",                           import_icon},
            {"import-from-sd-card_icon",              import_from_sd_card_icon},
            {"import-from-server_icon",               import_from_server_icon},
            {"import-from-computer_icon",             import_from_computer_icon},
            {"export_icon",                           export_icon},
            {"package_icon",                          package_icon},
            {"package-details_icon",                  package_details_icon},
            {"sync-data_icon",                        sync_data_icon},
            {"register_rfid_tag_icon",                register_rfid_tag_icon},
            {"wifi_connection_successful_icon",       wifi_connection_successful_icon},
            {"wifi_connection_failed_icon",           wifi_connection_failed_icon},
            {"server_connection_successful_icon",     server_connection_successful_icon},
            {"server_connection_failed_icon",         server_connection_failed_icon},
            {"register_rfid_tag_button_icon",         register_rfid_tag_button_icon},
            {"setting_button_icon",                   setting_button_icon},
            {"scan_button_icon",                      scan_button_icon},
            {"incoming_packed_boxes_banner_icon",     incoming_packed_boxes_banner_icon},
            {"outgoing_packed_boxes_banner_icon",     outgoing_packed_boxes_banner_icon},
            {"product_counting_banner_icon",          product_counting_banner_icon},
            {"incoming_box_icon",                     incoming_box_icon},
            {"outgoing_box_icon",                     outgoing_box_icon},
            {"green_tick_icon",                       green_tick_icon},
            {"red_x_icon",                            red_x_icon},
            {"qr_code_placeholder_banner_icon",       qr_code_placeholder_banner_icon},
            {"blurred_register_rfid_tag_button_icon", blurred_register_rfid_tag_button_icon},
            {"blurred_scan_button_icon",              blurred_scan_button_icon},
            {"mes_package_dialog_banner_icon",        mes_package_dialog_banner_icon},
            {"start_scanning_button_icon",            start_scanning_button_icon},
            {"text_scan_button_icon",                 text_scan_button_icon},
            {"clear_button_icon",                     clear_button_icon},
            {"submit_button_icon",                    submit_button_icon},
            {"company_logo_icon",                     company_logo_icon}
    };

    Display();

    void init(feature_layout_t _feature_layout);

    void draw_layout(feature_layout_t _feature_layout) const;

    const menu_icon *get_icon_by_name(const char *icon_name);

    void put_icon(int x, int y, const char *icon_name);

    void render_feature(feature_t _feature, task_results &_taskResults);

    static void blink_screen(bool &isTaskCompleted);

    void reset_display_setting();

    void update_screen_item(byte _index, screen_item_position _item_position);

    void clear_screen_items();

    void update_screen_selector(byte _screen_item_index);

    void clear_screen_selector() const;

    void set_screen_selector_border_color(feature_t _next_feature);
};

#endif //RFID_DISPLAY_H
