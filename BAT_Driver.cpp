#include "BAT_Driver.h"

float BAT_analogVolts = 0;

void BAT_Init(void)
{
    analogReadResolution(12);
}

float BAT_Get_Volts(void)
{
    int Volts = analogReadMilliVolts(BAT_ADC_PIN);
    BAT_analogVolts = (float)(Volts * 3.0 / 1000.0) / Measurement_offset;
    return BAT_analogVolts;
}
