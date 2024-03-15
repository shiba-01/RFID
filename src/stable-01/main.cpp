//
// Created by Tan on 25-Oct-23.
//
#include "Arduino.h"
#include "mediator.h"

Mediator mediator;

// Serial: For debugging
// Serial 1: For communicating with rfid module
// Serial 2: For communicating with computer
void setup() {
    Serial.begin(115200);
    Serial2.begin(115200);
    mediator.init_services();
//    mediator.taskArgs.feature = BOOT;
//    mediator.set_current_feature();
//    mediator.execute_task(RENDER_FEATURE);
//    mediator.execute_task(LOAD_FS);
//    if ((mediator.taskResults.isFsLoaded)) {
//        mediator.taskArgs.operatingMode = mediator.taskResults.savedOperatingModeInFs;
//        if (mediator.taskResults.savedOperatingModeInFs == HANDHELD) {
//            mediator.taskArgs.feature = HOME_HANDHELD_1;
//        } else {
//            mediator.taskArgs.feature = HOME_TERMINAL;
//        }
//    } else {
//        mediator.taskArgs.operatingMode = HANDHELD;
//    }
//    mediator.set_current_feature();
//    mediator.execute_task(RENDER_FEATURE);

//    mediator.taskArgs.operatingMode = HANDHELD;
//    mediator.execute_task(SET_OPERATING_MODE);
    mediator.execute_task(INIT_STA_WIFI);
    mediator.taskArgs.feature = HOME_HANDHELD_2;
    mediator.execute_task(RENDER_FEATURE);
    mediator.set_current_feature();
    //mediator.execute_task(CHECK_CONNECTION);
    //mediator.execute_task(INIT_AP_WIFI);

//For testing, we execute task BLINK_LED and stop this task when we receive message from MQTT broker
//    mediator.taskArgs.task = BLINK_SCREEN;
//    mediator.set_current_task();
//    mediator.set_current_task_status(false);
//    mediator.execute_task(mediator.taskArgs.task);
//    mediator.taskArgs.task = INIT_STA_WIFI;
//    mediator.set_current_task();
//    mediator.set_current_task_status(false);
//    mediator.execute_task(mediator.taskArgs.task);
}

void loop() {
    mediator.get_current_feature();
    mediator.execute_task(RENDER_FEATURE);
    mediator.set_current_feature();
    mediator.execute_task(READ_NAVIGATION_BUTTON);
    mediator.execute_task(READ_SERIAL_COMMUNICATION_MESSAGE);
    mediator.execute_task(HANDLE_MQTT_MESSAGE);

    mediator.taskArgs.task = mediator.taskResults.currentTask;
    mediator.set_current_task();

    if ((mediator.isTaskExecutable) & (mediator.taskArgs.task != IDLE) & (mediator.taskArgs.task != NO_TASK)) {
        while ((!mediator.isTaskCompleted) & (!mediator.isTaskQueueEmpty)) {
            mediator.taskArgs.task = mediator.taskResults.currentTask;
            mediator.execute_task(mediator.taskArgs.task);
            mediator.execute_task(READ_NAVIGATION_BUTTON);
            mediator.execute_task(READ_SERIAL_COMMUNICATION_MESSAGE);
            mediator.execute_task(HANDLE_MQTT_MESSAGE);
            mediator.get_current_task_status();
            yield();
        }

        Serial.println(F("Task execution completed"));
        mediator.set_current_task_status(true);
        mediator.taskArgs.task = IDLE;
        mediator.set_current_task();
        mediator.set_current_task_status(false);
    }
}