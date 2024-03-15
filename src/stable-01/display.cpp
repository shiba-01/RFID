//
// Created by Tan on 25-Oct-23.
//

#include "display.h"

AnimatedGIF gif;
TFT_eSPI tft = TFT_eSPI();
QRcode_eSPI qrcode(&tft);

// This section is used for display GIF---------------------------------------------------
#ifdef USE_DMA
uint16_t usTemp[2][BUFFER_SIZE]; // Definition for DMA buffer
#else
uint16_t usTemp[1][BUFFER_SIZE]; // Definition for non-DMA buffer
#endif
bool dmaBuf = 0; // Definition of dmaBuf
// End section----------------------------------------------------------------------------

Display::Display() {

}

void Display::init(feature_layout_t _feature_layout) {
    // Initialize display
    feature_layout = _feature_layout;
    tft.init();
    qrcode.init();
    tft.setTextFont(2);
    tft.setSwapBytes(true);
    if (_feature_layout == PORTRAIT) {
        tft.setRotation(0);
        // Display resolution
        SCREEN_WIDTH = 320;
        SCREEN_HEIGHT = 480;
    } else {
        tft.setRotation(1);
        // Display resolution
        SCREEN_WIDTH = 480;
        SCREEN_HEIGHT = 320;
    }
    tft.fillScreen(backgroundColor);
    draw_layout(feature_layout);
    gif.begin(BIG_ENDIAN_PIXELS);
}

void Display::draw_layout(feature_layout_t _feature_layout) const {
    // Clear the screen before drawing the layout
    tft.fillScreen(backgroundColor);

    // Set text font and size
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM); // Top-Left datum

    switch (_feature_layout) {
        case PORTRAIT: {
            // Draw header at the top
            tft.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, headerColor);

            // Set text color for the header
            tft.setTextColor(TFT_WHITE, headerColor);

            // Wi-Fi and server status icons
            tft.pushImage(10, 11, 19, 14, company_logo_icon);
            tft.pushImage(39, 10, 16, 16, server_connection_successful_icon);
            tft.drawString("Server not connected", 58, 10);
            tft.pushImage(294, 10, 16, 16, wifi_connection_successful_icon);

            // Draw date and time aligned to the top-right of the header
            tft.setTextDatum(TR_DATUM); // Align to the top-right
            //tft.drawString(dateTime, SCREEN_WIDTH - 5, 10);

            break;
        }
        case LANDSCAPE:
            // Draw header at the top
            tft.fillRect(0, 0, SCREEN_HEIGHT, HEADER_HEIGHT, headerColor);

            // Set text color for the header
            tft.setTextColor(TFT_WHITE, headerColor);

            // Draw Wi-Fi status at the top-left
            tft.drawString(wifiStatus, 5, 5);

            // Draw login status directly below Wi-Fi status
            tft.drawString(loginStatus, 5, 20);

            // Draw date and time at the top-right
//            dateTimeWidthLandscape = get_string_width(dateTime);
//            tft.drawString(dateTime, SCREEN_HEIGHT - dateTimeWidthLandscape - 5, 5);
//
//            // Draw server status directly below date and time
//            serverStatusWidthLandscape = get_string_width(serverStatus);
//            tft.drawString(serverStatus, SCREEN_HEIGHT - serverStatusWidthLandscape - 5, 20);

            // Draw viewport below the header
            tft.drawRect(0, HEADER_HEIGHT, SCREEN_HEIGHT, SCREEN_WIDTH - HEADER_HEIGHT, headerColor);
            break;
    }
}

const menu_icon *Display::get_icon_by_name(const char *icon_name) {
    for (uint16_t i = 0; i < numIcons; ++i) {
        if (strcmp(icons[i].name, icon_name) == 0) {
//            Serial.println(F("Got icon name: "));
//            Serial.println(String(icon_name));
//            Serial.println(icons[i].name);
//            Serial.println(i);
            return &icons[i];
        }
    }
    return nullptr; // Return nullptr if the icon is not found
}

void Display::put_icon(int x, int y, const char *icon_name) {
    const menu_icon *icon = get_icon_by_name(icon_name);
    if (icon != nullptr) {
//        Serial.println("Icon to be put: ");
//        Serial.println(String(icon_name));
        tft.pushImage(x, y, iconWidth, iconHeight, icon->icon_data);
    } else {
        // Handle the error, for example, by printing to the serial port
        Serial.print("Icon not found: ");
        Serial.println(icon_name);
    }
}

void Display::render_feature(feature_t _feature, task_results &_taskResults) {
    // Clear the viewport
    if (is_viewport_cleared) {
        Serial.println("Viewport is cleared now");
        tft.fillRect(0, NAV_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - HEADER_HEIGHT, backgroundColor);

        // Clear screen items and reset screen selector
        clear_screen_selector();
        clear_screen_items();
    }

    switch (_feature) {
        case BOOT:
            // Code to handle BOOT feature
            break;
        case LOADING: {
            tft.setSwapBytes(false);
            tft.setFreeFont(&FreeSans9pt7b);
            tft.setTextColor(TFT_WHITE, 0x84B2);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("Loading...", SCREEN_WIDTH / 2, 270);
            for (int i = 0; i < 2; i++) {
                if (gif.open((uint8_t *) loading_animation, sizeof(loading_animation), GIFDraw)) {
                    Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(),
                                  gif.getCanvasHeight());
                    tft.startWrite();  // The TFT chip select is locked low
                    while (gif.playFrame(true, NULL)) {
                        yield();
                    }
                    gif.close();
                    tft.endWrite();  // Release TFT chip select for other SPI devices
                }
            }
            tft.setSwapBytes(true);
            reset_display_setting();
            //draw_layout(feature_layout);
            // Reset current screen features
            memset(current_screen_features, NO_FEATURE, 10);
            break;
        }
        case QR_CODE_SCANNING: {
            // Reset current screen background tasks
            for (byte i = 0; i < 10; ++i) {
                current_screen_background_tasks[i] = NO_TASK;
            }
            //Put QR code placeholder-------------------------------
            iconWidth = 230;
            iconHeight = 268;
            put_icon(45, 76, menu_icon_names[37]);
            // Generate and display QR code on the screen, content is mac address
            // mes-package/mes-package-group/shipment-plan
            String qr_code_type_string = R"({"qrCodeType":")";
            qr_code_type = "";
            switch (_taskResults.currentScreenItemIndex) {
                case 0:
                    qr_code_type = "MES-PACKAGE";
                    _taskResults.selected_mes_package = "";
                    Serial.println(F("Waiting for MES package message arrives"));
                    break;
                case 1:
                    qr_code_type = "MES-PACKAGE-GROUP";
                    _taskResults.selected_mes_package_group = "";
                    Serial.println(F("Waiting for MES package group message arrives"));
                    break;
                case 2:
                    qr_code_type = "SHIPMENT-PLAN";
                    break;
            }
            String mac_addr = R"(","macAddress":")";
            Serial.println(
                    "Generated QR code string: " + qr_code_type_string + qr_code_type + mac_addr +
                    _taskResults.mac_address +
                    "\"}");
            qrcode.create(qr_code_type_string + qr_code_type + mac_addr + _taskResults.mac_address + "\"}");
            break;
        }
        case HOME_HANDHELD_2: {
            // Define which icons to display for the HOME HANDHELD 1 case
            const byte homeHandheld1IconIndices[] = {28, 27, 9, 10, 0};

            // Put elements by hand-------------------------------------------------
            byte screen_item_index = 0;
            screen_item_position _item_position;

            // Put current counted packed boxes-------------------------------------
            iconWidth = 300;
            iconHeight = 131;
            put_icon(10, 46, menu_icon_names[32]);
            tft.setFreeFont(&FreeSansBold9pt7b);
            tft.setTextColor(TFT_WHITE);
            tft.setTextFont(2);
            tft.setTextSize(1);
            tft.fillRect(10, 173, 300, 4, 0x95B7);
            if (_taskResults.selected_mes_package != "") {
                tft.fillRect(58, 10, 150, 20, headerColor);
                tft.drawString("Server connected", 58, 10);
                tft.setTextColor(0x573F);
                tft.drawString("Connected", 185, 85);
                tft.drawString(String(_taskResults.mes_target), 185, 105);
                tft.setTextColor(0xa554);
                tft.drawString("Op Name: ", 20, 131);
                tft.setTextColor(0x573F);
                tft.drawString(_taskResults.mes_operation_name, 80, 131);
                tft.drawString(_taskResults.selected_mes_package, 20, 153);
                tft.fillRect(10, 173, 300, 4, 0x2E3B);
                tft.pushImage(20, 56, 70, 70, _taskResults.mes_img_buffer);
            } else {
                tft.fillRect(58, 10, 150, 20, headerColor);
                tft.setTextColor(TFT_WHITE);
                tft.drawString("Server not connected", 58, 10);
                tft.drawString("Not connected", 185, 85);
                tft.drawString("[ Target ]", 185, 105);
                tft.drawString("[ Op Name ]", 20, 131);
                tft.drawString("[ MES Package ]", 20, 153);
            }
            // Update accordingly screen item
            _item_position = {10, 46, iconWidth, iconHeight};
            update_screen_item(screen_item_index, _item_position);
            ++screen_item_index;

            iconWidth = 300;
            iconHeight = 102;
            // Put Incoming packed boxes banner--------------------------------------
            put_icon(10, 191, menu_icon_names[30]);
            // Update accordingly screen item
            _item_position = {10, 191, iconWidth, iconHeight};
            update_screen_item(screen_item_index, _item_position);
            ++screen_item_index;
            if (_taskResults.selected_mes_package_group != "") {
                if (_taskResults.selected_mes_package == "") {
                    tft.fillRect(58, 10, 150, 20, headerColor);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString("Server connected", 58, 10);
                    tft.setTextColor(0x573F);
                    tft.fillRect(185, 85, 100, 20, 0x1B2E);
                    tft.drawString("Connected", 185, 85);
                    tft.fillRect(10, 173, 300, 4, 0x2E3B);
                    tft.pushImage(20, 56, 70, 70, _taskResults.mes_img_buffer);
                }
                tft.drawString(_taskResults.selected_mes_package_group, 20, 273);
            } else {
                tft.setTextColor(TFT_WHITE);
                tft.drawString("[ Package groups: ]", 20, 273);
                if (_taskResults.selected_mes_package == "") {
                    tft.fillRect(58, 10, 150, 20, headerColor);
                    tft.drawString("Server not connected", 58, 10);
                    tft.fillRect(185, 85, 100, 20, 0x1B2E);
                    tft.drawString("Not connected", 185, 85);
                }
            }

            // Put Outgoing packed boxes banner------------------------------------
            put_icon(10, 307, menu_icon_names[31]);
            tft.drawString("[ Shipment Plan: ]", 20, 390);
            // Update accordingly screen item
            _item_position = {10, 307, iconWidth, iconHeight};
            update_screen_item(screen_item_index, _item_position);
            ++screen_item_index;

            iconWidth = 144;
            iconHeight = 45;
            if ((_taskResults.selected_mes_package != "") or (_taskResults.selected_mes_package_group != "")) {
                Serial.println("MES Package or MES Package Group is selected");
                // Put the Register button on the bottom left------------------------------------
                put_icon(10, 425, menu_icon_names[27]);
                // Update accordingly screen item
                _item_position = {10, 425, 144, 45};
                update_screen_item(screen_item_index, _item_position);
                ++screen_item_index;

                // Put Scan button------------------------------------W
                put_icon(166, 425, menu_icon_names[29]);
                // Update accordingly screen item
                _item_position = {166, 425, 144, 45};
                update_screen_item(screen_item_index, _item_position);
                ++screen_item_index;
            } else {
                Serial.println("MES Package or MES Package Group is not selected");
                // Put the Register button on the bottom left------------------------------------
                put_icon(10, 425, menu_icon_names[38]);
                // Put Scan button------------------------------------
                put_icon(166, 425, menu_icon_names[39]);
            }

            // Put Setting button on the bottom right------------------------------------
//            iconWidth = 290;
//            iconHeight = 40;
//            put_icon(15, 425, menu_icon_names[28]);
//            // Update accordingly screen item
//            _item_position = {15, 425, 290, 40};
//            update_screen_item(screen_item_index, _item_position);
//            ++screen_item_index;

            screen_selector_border_color = backgroundColor;
            screen_item_count = screen_item_index;
            reset_display_setting();
            // Start to set screen selector to the first one item
            update_screen_selector(0);
            current_feature_item_type = MENU_ICON;
            // Reset current screen features
            memset(current_screen_features, NO_FEATURE, 10);
            current_screen_features[0] = QR_CODE_SCANNING;
            current_screen_features[1] = QR_CODE_SCANNING;
            current_screen_features[2] = QR_CODE_SCANNING;
            if ((_taskResults.selected_mes_package != "") or (_taskResults.selected_mes_package_group != "")) {
                current_screen_features[3] = RFID_REGISTER_TAG;
                current_screen_features[4] = RFID_SCAN_DETAILS_REVIEW;
            }
            break;
        }
        case HOME_TERMINAL:
            // Code to handle HOME_TERMINAL feature
            break;
        case RFID_SCAN_DETAILS_REVIEW: {
            tft.setFreeFont(&FreeSansBold9pt7b);
            tft.setTextColor(0x12AC);
            tft.drawString("RECONFIRM MES PACKAGE", 41, 46);
            tft.fillRect(10, 71, 300, 327, 0x1B2E);
            iconWidth = 280;
            iconHeight = 41;
            put_icon(20, 91, menu_icon_names[40]);
            //tft.drawString("Package groups: " + _taskResults.selected_list_items[2], 32, 91); // Commented out, adjusted to comment position
            tft.setTextFont(2);
            tft.setTextSize(1);
            tft.setTextColor(TFT_WHITE);
            //tft.drawString(_taskResults.selected_list_items[3], 32, 128); // Commented out, adjusted to comment position
            tft.drawString(_taskResults.selected_mes_package, 20, 135);
            // Draw the product image
            tft.pushImage(20, 160, 70, 70, _taskResults.mes_img_buffer);
            // Draw package information
            tft.setTextFont(2);
            tft.setTextColor(0xa554);
            tft.drawString("BuyerPO: ", 100, 157);
            tft.setTextFont(2);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(_taskResults.buyer_po, 180, 157);
            tft.setTextFont(2);
            tft.setTextColor(0xa554);
            tft.drawString("Line: ", 100, 177);
            tft.setTextFont(2);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(_taskResults.line_no, 180, 177);
            tft.setTextFont(2);
            tft.setTextColor(0xa554);
            tft.drawString("Module name: ", 100, 197);
            tft.setTextFont(2);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(_taskResults.module_name, 190, 197);
            tft.setTextFont(2);
            tft.setTextColor(0xa554);
            tft.drawString("MES package target: ", 100, 217);
            tft.setFreeFont(&FreeSans9pt7b);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(String(_taskResults.mes_target), 230, 217);
            tft.setTextFont(2);
            tft.setTextColor(0xa554);
            tft.drawString("Op name: ", 20, 243);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(_taskResults.mes_operation_name, 120, 243);
            tft.setTextColor(0xa554);
            tft.drawString("Style: ", 20, 261);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(_taskResults.style_text, 120, 261);
            tft.setTextColor(0xa554);
            tft.drawString("Style color: ", 20, 279);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(_taskResults.style_color, 120, 279);
            tft.setTextColor(0xa554);
            tft.drawString("AO No: ", 20, 297);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(_taskResults.ao_no, 120, 297);
            tft.setTextColor(0xa554);
            tft.drawString("TargetQty: ", 20, 315);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(_taskResults.target_qty, 120, 315);
            tft.setTextColor(0xa554);
            tft.drawString("Destination: ", 20, 333);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(_taskResults.destination, 120, 333);
            tft.setTextColor(0xa554);
            tft.drawString("Delivery data: ", 20, 351);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(_taskResults.delivery_date, 120, 351);
            tft.fillRect(10, 398, 300, 4, 0x126b);

            // Draw the start scanning button
            iconWidth = 300;
            iconHeight = 45;
            put_icon(10, 423, menu_icon_names[41]);

            // Update accordingly screen item
            screen_item_position _item_position = {10, 423, 300, 45};
            update_screen_item(0, _item_position);
            screen_selector_border_color = backgroundColor;
            screen_item_count = 1;
            // Start to set screen selector to the first one item
            update_screen_selector(0);
            current_feature_item_type = MENU_ICON;
            // Reset current screen features
            memset(current_screen_features, NO_FEATURE, 10);
            //current_screen_features[0] = RFID_SCAN_RESULT;
            current_screen_features[0] = RFID_SCAN_RESULT;
            // Reset display settings
            reset_display_setting();
            break;
        }
        case RFID_SCAN_RESULT: {
            // Check if the background task is completed, if yes, start rendering, else, set background tasks and return
            if (is_background_task_completed) {
                // Update the "Submitted/target" field with the latest count
                String submittedTargetStr =
                        String(_taskResults.current_matched_mes_scanned_rfid_tag_count) + "/" +
                        String(_taskResults.registered_rfid_tags_from_server_count);
                tft.fillRect(223, 251, 60, 50, 0x1b2e); // Clear the previous area
                tft.setTextColor(0xe751); // Color for the count
                tft.setFreeFont(&FreeSansBold9pt7b);
                tft.drawString(submittedTargetStr, 223, 267);

                // Update the "Quantity" field with the latest count
                String quantityStr = String(_taskResults.current_scanned_rfid_tag_count);
                tft.fillRect(223, 343, 60, 48, 0xdf7e); // Clear the previous area
                tft.setFreeFont(&FreeSansBold12pt7b);
                tft.setTextColor(0x1b2e);
                tft.drawString(quantityStr, 223, 357);
                // Do nothing, lets user choose to Submit scan results to server or Clear and scan again
            } else {
                // Check if this is the first time this feature is rendered
                if (_taskResults.currentFeature == RFID_SCAN_DETAILS_REVIEW) {
                    Serial.println(F("This is the first time RFID_SCAN_RESULT feature is rendered"));

                    // Reset RFID scan results
                    _taskResults.current_scanned_rfid_tag_count = 0;
                    _taskResults.is_the_first_scan = true;

                    // Check provided arguments which are selected items from previous lists
                    Serial.println(F("Selected items from previous lists: "));
                    for (byte i = 0; i < 10; ++i) {
                        if (_taskResults.selected_list_items[i] != "")
                            Serial.println(_taskResults.selected_list_items[i]);
                    }

                    tft.setFreeFont(&FreeSansBold9pt7b);
                    tft.setTextColor(0x12AC);
                    tft.drawString("SCAN RESULT", 100, 46);
                    tft.fillRect(10, 71, 300, 112, 0x1B2E);
                    // Draw the product image
                    tft.pushImage(22, 91, 70, 70, _taskResults.mes_img_buffer);
                    // Draw package information
                    tft.setTextFont(2);
                    tft.setTextColor(0xa554);
                    tft.drawString("BuyerPO:", 102, 88);
                    tft.setTextFont(2);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(_taskResults.buyer_po, 170, 88);
                    tft.setTextFont(2);
                    tft.setTextColor(0xa554);
                    tft.drawString("Line: ", 102, 108);
                    tft.setTextFont(2);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(_taskResults.line_no, 170, 108);
                    tft.setTextFont(2);
                    tft.setTextColor(0xa554);
                    tft.drawString("Module name: ", 102, 128);
                    tft.setTextFont(2);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(_taskResults.module_name, 190, 128);
                    tft.setTextFont(2);
                    tft.setTextColor(0xa554);
                    tft.drawString("MES package target: ", 102, 148);
                    tft.setFreeFont(&FreeSans9pt7b);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(String(_taskResults.mes_target), 230, 148);
                    tft.setFreeFont(&FreeSans9pt7b);
                    tft.fillRect(10, 183, 300, 4, 0x126b);

                    // Submitted status
                    tft.fillRect(10, 205, 300, 205, 0xdf7e);
                    tft.setFreeFont(&FreeSansBold9pt7b);
                    tft.setTextColor(0x8c51);
                    tft.drawString("Submitted status", 22, 224);
                    tft.setFreeFont(&FreeSans9pt7b);
                    tft.setTextColor(TFT_WHITE);
                    tft.fillRect(22, 251, 276, 50, 0x1b2e);
                    tft.drawString("Submitted/target", 42, 267);
                    tft.setTextColor(0xe751);
                    tft.setFreeFont(&FreeSansBold9pt7b);
                    tft.drawString(
                            String(_taskResults.current_scanned_rfid_tag_count) + "/" +
                            String(_taskResults.registered_rfid_tags_from_server_count),
                            223, 267);
                    // Current scan status
                    tft.setFreeFont(&FreeSansBold9pt7b);
                    tft.setTextColor(0x8c51);
                    tft.drawString("Quantity scanned", 22, 315);
                    tft.setFreeFont(&FreeSans9pt7b);
                    tft.drawRect(22, 342, 276, 50, 0x1b2e);
                    tft.setTextColor(0x8c51);
                    tft.drawString("Quantity", 42, 357);
                    tft.setFreeFont(&FreeSansBold12pt7b);
                    tft.setTextColor(0x1b2e);
                    tft.drawString(String(_taskResults.current_scanned_rfid_tag_count), 223, 357);
                    tft.setFreeFont(&FreeSans9pt7b);

                    iconWidth = 110;
                    iconHeight = 45;
                    put_icon(10, 425, menu_icon_names[42]);
                    put_icon(200, 425, menu_icon_names[44]);
                    iconWidth = 56;
                    put_icon(132, 425, menu_icon_names[43]);

                    // Update accordingly screen item
                    iconWidth = 110;
                    screen_item_position _item_position = {10, 425, iconWidth, iconHeight};
                    update_screen_item(0, _item_position);

                    iconWidth = 110;
                    _item_position = {200, 425, iconWidth, iconHeight};
                    update_screen_item(2, _item_position);

                    iconWidth = 56;
                    _item_position = {132, 425, iconWidth, iconHeight};
                    update_screen_item(1, _item_position);

                    screen_selector_border_color = backgroundColor;
                    screen_item_count = 3;
                    // Start to set screen selector to the first one item
                    update_screen_selector(0);

                    //current_feature_item_type = MENU_ICON;
                    // Reset current screen tasks
                    memset(current_screen_tasks, NO_TASK, 10);
                    current_screen_tasks[0] = READ_RFID_TAG;
                    current_screen_tasks[1] = REGISTER_RFID_TAG;
                    current_screen_tasks[2] = REGISTER_RFID_TAG;
                    current_feature_item_type = TASK_ITEM;
                    // Reset current screen features
                    memset(current_screen_features, NO_FEATURE, 10);
                    // Reset display settings
                    reset_display_setting();
                    is_viewport_cleared = false;
                } else {
                    is_background_task_required = true;
                    // Reset current screen background tasks
                    for (byte i = 0; i < 10; ++i) {
                        current_screen_background_tasks[i] = NO_TASK;
                    }
                    switch (_taskResults.currentScreenItemIndex) {
                        case 0:
                            // Start scanning task
                            Serial.println(F("Start background scanning task"));
                            current_screen_background_tasks[0] = READ_RFID_TAG;
                            break;
                        case 1:
                            // Start resetting scan result task
                            Serial.println(F("Start background resetting scan result task"));
                            current_screen_background_tasks[0] = RESET_SCANNED_RFID_TAG_COUNT;
                            break;
                        case 2:
                            // Start submitting task
                            Serial.println(F("Start background submitting task"));
                            current_screen_background_tasks[0] = RESET_SCANNED_RFID_TAG_COUNT;
                            current_screen_background_tasks[1] = NO_TASK;
                            is_viewport_cleared = true;
                            is_back_to_home = true;
                            break;
                    }
                }
            }
            break;
        }
        case RFID_REGISTER_TAG: {
            // Check if the background task is completed, if yes, start rendering, else, set background tasks and return
            if (is_background_task_completed) {
                // Just display the recently scanned result and increase total scans
                tft.setFreeFont(&FreeSansBold12pt7b);
                tft.setTextColor(0x350F);
                // Clear the area where the "0/200" is displayed
                tft.fillRect(235, 361, 70, 50, TFT_WHITE);
                // Draw the new string with the updated count
                tft.drawString(String(_taskResults.current_scanned_rfid_tag_count), 245, 375);
                // Do nothing, lets user choose to Submit scan results to server or Clear and scan again
            } else {
                // Check if this is the first time this feature is rendered
                if (_taskResults.currentScreenItemIndex == 3) {
                    Serial.println(F("This is the first time RFID_REGISTER_TAG feature is rendered"));

                    // Reset RFID scan results
                    _taskResults.current_scanned_rfid_tag_count = 0;
                    _taskResults.is_the_first_scan = true;

                    tft.setFreeFont(&FreeSansBold9pt7b);
                    tft.setTextColor(0x12AC);
                    tft.drawString("RFID REGISTRATION", 73, 46);
                    tft.fillRect(10, 71, 300, 241, 0x1B2E);
                    iconWidth = 280;
                    iconHeight = 41;
                    put_icon(20, 91, menu_icon_names[40]);
                    //tft.drawString("Package groups: " + _taskResults.selected_list_items[2], 32, 91); // Commented out, adjusted to comment position
                    tft.setTextFont(2);
                    tft.setTextSize(1);
                    tft.setTextColor(TFT_WHITE);
                    //tft.drawString(_taskResults.selected_list_items[3], 32, 128); // Commented out, adjusted to comment position
                    tft.drawString(_taskResults.selected_mes_package, 20, 135);
                    // Draw the product image
                    tft.pushImage(20, 160, 70, 70, _taskResults.mes_img_buffer);
                    // Draw package information
                    tft.setTextFont(2);
                    tft.setTextColor(0xa554);
                    tft.drawString("BuyerPO: ", 100, 157);
                    tft.setTextFont(2);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(_taskResults.buyer_po, 180, 157);
                    tft.setTextFont(2);
                    tft.setTextColor(0xa554);
                    tft.drawString("Line: ", 100, 177);
                    tft.setTextFont(2);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(_taskResults.line_no, 180, 177);
                    tft.setTextFont(2);
                    tft.setTextColor(0xa554);
                    tft.drawString("Module name: ", 100, 197);
                    tft.setTextFont(2);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(_taskResults.module_name, 190, 197);
                    tft.setTextFont(2);
                    tft.setTextColor(0xa554);
                    tft.drawString("MES package target: ", 100, 217);
                    tft.setFreeFont(&FreeSans9pt7b);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(String(_taskResults.mes_target), 230, 217);
                    tft.setTextFont(2);
                    tft.setTextColor(0xa554);
                    tft.drawString("Op name: ", 20, 243);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(_taskResults.mes_operation_name, 100, 243);
                    tft.setTextColor(0xa554);
                    tft.drawString("Style: ", 20, 261);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(_taskResults.style_text, 100, 261);
                    tft.setTextColor(0xa554);
                    tft.drawString("Style color: ", 20, 279);
                    tft.setTextColor(TFT_WHITE);
                    tft.drawString(_taskResults.style_color, 100, 279);
                    tft.fillRect(10, 312, 300, 4, 0x126b);

                    tft.setFreeFont(&FreeSansBold9pt7b);
                    tft.fillRect(215, 328, 94, 27, 0xFE6C);
                    tft.setTextColor(TFT_BLACK);
                    tft.drawString(String(_taskResults.registered_rfid_tags_from_server_count), 245, 335);
                    tft.setTextColor(0x4228);
                    tft.setTextFont(2);
                    tft.setTextSize(1);
                    tft.setFreeFont(&FreeSansBold9pt7b);
                    tft.drawString("Total registered tags:", 10, 334);
                    tft.fillRect(10, 361, 300, 50, TFT_WHITE);
                    tft.setTextColor(0x8c51);
                    tft.setFreeFont(&FreeSansBold9pt7b);
                    tft.drawString("Detected", 30, 379);
                    tft.setFreeFont(&FreeSansBold12pt7b);
                    tft.setTextColor(0x350F);
                    tft.drawString("", 218, 375);

                    iconWidth = 110;
                    iconHeight = 45;
                    put_icon(10, 425, menu_icon_names[42]);
                    put_icon(200, 425, menu_icon_names[44]);
                    iconWidth = 56;
                    put_icon(132, 425, menu_icon_names[43]);

                    // Update accordingly screen item
                    iconWidth = 110;
                    screen_item_position _item_position = {10, 425, iconWidth, iconHeight};
                    update_screen_item(0, _item_position);

                    iconWidth = 110;
                    _item_position = {200, 425, iconWidth, iconHeight};
                    update_screen_item(2, _item_position);

                    iconWidth = 56;
                    _item_position = {132, 425, iconWidth, iconHeight};
                    update_screen_item(1, _item_position);

                    screen_selector_border_color = backgroundColor;
                    screen_item_count = 3;
                    // Start to set screen selector to the first one item
                    update_screen_selector(0);

                    //current_feature_item_type = MENU_ICON;
                    // Reset current screen tasks
                    memset(current_screen_tasks, NO_TASK, 10);
                    current_screen_tasks[0] = READ_RFID_TAG;
                    current_screen_tasks[1] = REGISTER_RFID_TAG;
                    current_screen_tasks[2] = REGISTER_RFID_TAG;
                    current_feature_item_type = TASK_ITEM;
                    // Reset current screen features
                    memset(current_screen_features, NO_FEATURE, 10);
                    // Reset display settings
                    reset_display_setting();
                    is_viewport_cleared = false;
                } else {
                    is_background_task_required = true;
                    // Reset current screen background tasks
                    for (byte i = 0; i < 10; ++i) {
                        current_screen_background_tasks[i] = NO_TASK;
                    }
                    switch (_taskResults.currentScreenItemIndex) {
                        case 0:
                            // Start scanning task
                            Serial.println(F("Start background scanning task"));
                            current_screen_background_tasks[0] = READ_RFID_TAG;
                            break;
                        case 1:
                            // Start resetting the scanned rfid tag count task
                            Serial.println(F("Start background clearing scan result task"));
                            current_screen_background_tasks[0] = RESET_SCANNED_RFID_TAG_COUNT;
                            break;
                        case 2:
                            // Start submitting task
                            Serial.println(F("Start background submitting task"));
                            // Clear scan result task
                            current_screen_background_tasks[0] = REGISTER_RFID_TAG;
                            current_screen_background_tasks[1] = RESET_SCANNED_RFID_TAG_COUNT;
                            is_viewport_cleared = true;
                            is_back_to_home = true;
                            break;
                    }
                }
            }
            break;
        }
        default:
            // Code to handle unknown feature
            break;
    }
}

void Display::blink_screen(bool &isTaskCompleted) {
    static int blink_count = 0;
    if (blink_count <= 10) {
        Serial.print(F("Execute time: "));
        Serial.println(blink_count);
        tft.fillScreen(random(TFT_WHITE));
        delay(500);
        tft.fillScreen(TFT_BLACK);
        delay(500);
        blink_count++;
    } else {
        isTaskCompleted = true;
    }
}

void Display::reset_display_setting() {
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(TL_DATUM);
    iconWidth = 64;
    iconHeight = 64;
}

void Display::update_screen_item(byte _index, screen_item_position _item_position) {
    screen_items[_index].x = _item_position.x;
    screen_items[_index].y = _item_position.y;
    screen_items[_index].w = _item_position.w;
    screen_items[_index].h = _item_position.h;
    Serial.println(F("Updated screen item"));
}

void Display::clear_screen_items() {
    memset(screen_items, 0, sizeof(screen_items));
    Serial.println(F("Cleared screen items array"));
}

void Display::update_screen_selector(byte _screen_item_index) {
    current_screen_selector.old_position = current_screen_selector.current_position;
    current_screen_selector.current_position = screen_items[_screen_item_index];
    current_screen_selector.screen_item_index = _screen_item_index;
    Serial.println(F("Updated screen selector. Re render now"));
//    Serial.print(F("Current position w: "));
//    Serial.println(current_screen_selector.current_position.w);
//    Serial.print(F("Current position h: "));
//    Serial.println(current_screen_selector.current_position.h);
    // Define border thickness
    byte border_thickness = 2; // Adjust the thickness of your border here

    // Define the color for the selector border
    uint16_t border_color = 0xEF51;

    // Draw new screen selector border
    // Draw top border
    tft.fillRect(current_screen_selector.current_position.x - border_thickness,
                 current_screen_selector.current_position.y - border_thickness,
                 current_screen_selector.current_position.w + (2 * border_thickness),
                 border_thickness, border_color);
    // Draw bottom border
    tft.fillRect(current_screen_selector.current_position.x - border_thickness,
                 current_screen_selector.current_position.y + current_screen_selector.current_position.h,
                 current_screen_selector.current_position.w + (2 * border_thickness),
                 border_thickness, border_color);
    // Draw the left border
    tft.fillRect(current_screen_selector.current_position.x - border_thickness,
                 current_screen_selector.current_position.y - border_thickness,
                 border_thickness,
                 current_screen_selector.current_position.h + (2 * border_thickness), border_color);
    // Draw right border
    tft.fillRect(current_screen_selector.current_position.x + current_screen_selector.current_position.w,
                 current_screen_selector.current_position.y - border_thickness,
                 border_thickness,
                 current_screen_selector.current_position.h + (2 * border_thickness), border_color);
}

void Display::clear_screen_selector() const {
    //static uint16_t border_color = backgroundColor;
    // Define border thickness
    byte border_thickness = 2; // Adjust the thickness of your border here

    // Define the color for clearing the border if it changes
    //if (_border_color != border_color) border_color = _border_color;

    // Clear old screen selector border by drawing over it with the background color
    // Clear top border
    tft.fillRect(current_screen_selector.current_position.x - border_thickness,
                 current_screen_selector.current_position.y - border_thickness,
                 current_screen_selector.current_position.w + (2 * border_thickness),
                 border_thickness, screen_selector_border_color);
    // Clear bottom border
    tft.fillRect(current_screen_selector.current_position.x - border_thickness,
                 current_screen_selector.current_position.y + current_screen_selector.current_position.h,
                 current_screen_selector.current_position.w + (2 * border_thickness),
                 border_thickness, screen_selector_border_color);
    // Clear left border
    tft.fillRect(current_screen_selector.current_position.x - border_thickness,
                 current_screen_selector.current_position.y - border_thickness,
                 border_thickness,
                 current_screen_selector.current_position.h + (2 * border_thickness), screen_selector_border_color);
    // Clear right border
    tft.fillRect(current_screen_selector.current_position.x + current_screen_selector.current_position.w,
                 current_screen_selector.current_position.y - border_thickness,
                 border_thickness,
                 current_screen_selector.current_position.h + (2 * border_thickness), screen_selector_border_color);
}

void Display::set_screen_selector_border_color(feature_t _next_feature) {
    // Set screen selector border color accordingly to the next feature
    switch (_next_feature) {
        //Serial.println(F("Set screen selector border color accordingly to next feature"));
//        case RFID_FACTORY_SELECT:
//        case RFID_PACKAGE_GROUPS_LIST:
//        case RFID_MES_PACKAGES_LIST:
//            screen_selector_border_color = 0x4208;
//            break;
        default:
            screen_selector_border_color = backgroundColor;
            break;
    }
}

void GIFDraw(GIFDRAW *pDraw) {
    uint8_t *s;
    uint16_t *d, *usPalette;
    int x, y, iWidth, iCount, centerX, centerY;

    // Calculate centered x and y coordinates
    centerX = (DISPLAY_WIDTH - pDraw->iWidth) / 2;
    centerY = (DISPLAY_HEIGHT - pDraw->iHeight) / 2;

    // Set drawing start point (top-left corner of the image) to the centered coordinates
    pDraw->iX = centerX;
    pDraw->iY = centerY;

    // Continue with the original code but now with the centered position
    iWidth = pDraw->iWidth;
    if (iWidth + pDraw->iX > DISPLAY_WIDTH)
        iWidth = DISPLAY_WIDTH - pDraw->iX;
    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line

    // Ensure that the drawing does not go outside the display bounds
    if (y >= DISPLAY_HEIGHT || pDraw->iX >= DISPLAY_WIDTH || iWidth < 1)
        return;

    // Old image disposal
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
        for (x = 0; x < iWidth; x++) {
            if (s[x] == pDraw->ucTransparent)
                s[x] = pDraw->ucBackground;
        }
        pDraw->ucHasTransparency = 0;
    }

    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) // if transparency used
    {
        uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
        pEnd = s + iWidth;
        x = 0;
        iCount = 0; // count non-transparent pixels
        while (x < iWidth) {
            c = ucTransparent - 1;
            d = &usTemp[0][0];
            while (c != ucTransparent && s < pEnd && iCount < BUFFER_SIZE) {
                c = *s++;
                if (c == ucTransparent) // done, stop
                {
                    s--; // back up to treat it like transparent
                } else // opaque
                {
                    *d++ = usPalette[c];
                    iCount++;
                }
            } // while looking for opaque pixels
            if (iCount) // any opaque pixels?
            {
                // DMA would degrade performance here due to short line segments
                tft.setAddrWindow(pDraw->iX + x, y, iCount, 1);
                tft.pushPixels(usTemp, iCount);
                x += iCount;
                iCount = 0;
            }
            // no, look for a run of transparent pixels
            c = ucTransparent;
            while (c == ucTransparent && s < pEnd) {
                c = *s++;
                if (c == ucTransparent)
                    x++;
                else
                    s--;
            }
        }
    } else {
        s = pDraw->pPixels;

        // Unroll the first pass to boost DMA performance
        // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
        if (iWidth <= BUFFER_SIZE)
            for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
        else
            for (iCount = 0; iCount < BUFFER_SIZE; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

#ifdef USE_DMA // 71.6 fps (ST7796 84.5 fps)
        tft.dmaWait();
    tft.setAddrWindow(pDraw->iX, y, iWidth, 1);
    tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
    dmaBuf = !dmaBuf;
#else // 57.0 fps
        tft.setAddrWindow(pDraw->iX, y, iWidth, 1);
        tft.pushPixels(&usTemp[0][0], iCount);
#endif

        iWidth -= iCount;
        // Loop if pixel buffer smaller than width
        while (iWidth > 0) {
            // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
            if (iWidth <= BUFFER_SIZE)
                for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
            else
                for (iCount = 0; iCount < BUFFER_SIZE; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

#ifdef USE_DMA
            tft.dmaWait();
      tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
      dmaBuf = !dmaBuf;
#else
            tft.pushPixels(&usTemp[0][0], iCount);
#endif
            iWidth -= iCount;
        }
    }
} /* GIFDraw() */