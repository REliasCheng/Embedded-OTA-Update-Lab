#ifndef __APP_H__
#define __APP_H__

#include "gd32f4xx.h"
#include <stdio.h>
#include <string.h>

// ----------------------- App_Input --------------------

void App_Input_init();
void App_Input_task();

// ----------------------- App_OTA --------------------

void App_OTA_init();
void App_OTA_task();

// ----------------------- App_Wifi --------------------

void App_Wifi_init();
void App_Wifi_task();

#endif