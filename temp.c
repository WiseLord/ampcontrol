#include "temp.h"

#include <avr/eeprom.h>
#include "eeprom.h"
#include "input.h"
#include "pins.h"

static int8_t tempTH;

void loadTempParams(void)
{
    tempTH = eeprom_read_byte((uint8_t *)EEPROM_TEMP_TH);

    return;
}

void saveTempParams(void)
{
    eeprom_update_byte((uint8_t *)EEPROM_TEMP_TH, tempTH);

    return;
}

void tempInit(void)
{
    OUT(FAN1);
    OUT(FAN2);

    CLR(FAN1);
    CLR(FAN2);

    return;
}

void tempControlProcess(void)
{
    int8_t temp1, temp2;

    temp1 = ds18x20GetTemp(0) / 10;
    temp2 = ds18x20GetTemp(1) / 10;

    if (temp1 >= tempTH)
        SET(FAN1);
    else if (temp1 <= tempTH - 5)
        CLR(FAN1);

    if (temp2 >= tempTH)
        SET(FAN2);
    else if (temp2 <= tempTH - 5)
        CLR(FAN2);

    return;
}

void setTempTH(int8_t temp)
{
    temp = temp;

    return;
}

int8_t getTempTH(void)
{
    return tempTH;
}

void changeTempTH(int8_t diff)
{
    tempTH += diff;
    if (tempTH > MAX_TEMP)
        tempTH = MAX_TEMP;
    if (tempTH < MIN_TEMP)
        tempTH = MIN_TEMP;

    return;
}

