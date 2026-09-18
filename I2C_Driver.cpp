#include "I2C_Driver.h"

static SemaphoreHandle_t i2cMutex = NULL;

void I2C_Init(void) {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_MASTER_FREQ_HZ);
  if (i2cMutex == NULL) {
    i2cMutex = xSemaphoreCreateMutex();
  }
}

void I2C_Lock(void)  { if (i2cMutex) xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)); }
void I2C_Unlock(void){ if (i2cMutex) xSemaphoreGive(i2cMutex); }

bool I2C_Read(uint8_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
  if (i2cMutex && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    printf("I2C Read lock timeout\r\n");
    return -1;
  }
  Wire.beginTransmission(Driver_addr);
  Wire.write(Reg_addr);
  if (Wire.endTransmission(true)) {
    printf("I2C Read fail\r\n");
    if (i2cMutex) xSemaphoreGive(i2cMutex);
    return -1;
  }
  Wire.requestFrom(Driver_addr, Length);
  for (int i = 0; i < Length; i++) {
    *Reg_data++ = Wire.read();
  }
  if (i2cMutex) xSemaphoreGive(i2cMutex);
  return 0;
}

bool I2C_Write(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
  if (i2cMutex && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    printf("I2C Write lock timeout\r\n");
    return -1;
  }
  Wire.beginTransmission(Driver_addr);
  Wire.write(Reg_addr);
  for (int i = 0; i < Length; i++) {
    Wire.write(*Reg_data++);
  }
  if (Wire.endTransmission(true)) {
    printf("I2C Write fail\r\n");
    if (i2cMutex) xSemaphoreGive(i2cMutex);
    return -1;
  }
  if (i2cMutex) xSemaphoreGive(i2cMutex);
  return 0;
}