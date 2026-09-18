#pragma once
#include "Arduino.h"
#include <cstring>
#include "FS.h"
#include "SD_MMC.h"
#include "TCA9554PWR.h"

#define SD_CLK_PIN      14
#define SD_CMD_PIN      17
#define SD_D0_PIN       16

extern uint16_t SDCard_Size;
extern uint16_t Flash_Size;

void SD_Init();
void Flash_test();

bool SD_Logger_Open(void);
bool SD_Logger_IsOpen(void);
void SD_Logger_Write(const char *line);
void SD_Logger_Flush(void);
void SD_Logger_Close(void);