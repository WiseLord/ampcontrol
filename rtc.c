#include "rtc.h"

#include <avr/pgmspace.h>
#include "i2c.h"

RTC_type rtc;

const static RTC_type rtcMin PROGMEM = {0, 0, 0, 1, 1, 1, RTC_NOEDIT};
const static RTC_type rtcMax PROGMEM = {23, 59, 0, 31, 12, 99, RTC_NOEDIT};

static uint8_t rtcAddr = DS1307_I2C_ADDR;

static uint8_t rtcDaysInMonth()
{
    uint8_t ret = rtc.month;

    if (ret == 2) {
        ret = rtc.year & 0x03;
        ret = (ret ? 28 : 29);
    } else {
        if (ret > 7)
            ret++;
        ret |= 30;
    }

    return ret;
}

void rtcInit()
{
    rtc.etm = RTC_NOEDIT;
    // Try to find PCF8563
    if ((I2CStart(PCF8563_I2C_ADDR) & 0x18) == 0x18) {
        rtcAddr = PCF8563_I2C_ADDR;
    }
    I2CStop();
}

int8_t rtcWeekDay()
{
    uint8_t a, y, m;
    int8_t ret;

    a = (rtc.month > 2 ? 0 : 1);
    y = 12 + rtc.year - a;
    m = rtc.month + 12 * a - 2;

    ret = (rtc.date + y + (y / 4) + ((31 * m) / 12)) % 7;
    if (ret == 0)
        ret = 7;

    return ret;
}

void rtcReadTime()
{
    I2CStart(rtcAddr);
    if (rtcAddr == PCF8563_I2C_ADDR) {
        I2CWriteByte(PCF8563_SEC);
    } else {
        I2CWriteByte(DS1307_SEC);
    }

    I2CStart(rtcAddr | I2C_READ);

    rtc.sec = rtcBinDecToDec(I2CReadByte(I2C_ACK) & 0x7F);
    rtc.min = rtcBinDecToDec(I2CReadByte(I2C_ACK) & 0x7F);
    rtc.hour = rtcBinDecToDec(I2CReadByte(I2C_ACK) & 0x3F);
    if (rtcAddr == PCF8563_I2C_ADDR) {
        rtc.date = rtcBinDecToDec(I2CReadByte(I2C_ACK) & 0x3F);
        I2CReadByte(I2C_ACK);   // weekday
    } else {
        I2CReadByte(I2C_ACK);   // weekday
        rtc.date = rtcBinDecToDec(I2CReadByte(I2C_ACK) & 0x3F);
    }
    rtc.month = rtcBinDecToDec(I2CReadByte(I2C_ACK) & 0x1F);
    rtc.year = rtcBinDecToDec(I2CReadByte(I2C_NOACK) & 0xFF);

    I2CStop();

    return;
}

static void rtcSaveTime()
{
    uint8_t etm = rtc.etm;

    if (etm >= RTC_DATE)
        etm = RTC_DATE;

    I2CStart(rtcAddr);

    switch (etm) {
    case RTC_HOUR: {
        if (rtcAddr == PCF8563_I2C_ADDR) {
            I2CWriteByte(PCF8563_HOUR);
        } else {
            I2CWriteByte(DS1307_HOUR);
        }
        I2CWriteByte(rtcDecToBinDec(rtc.hour));
    }
    break;
    case RTC_MIN: {
        if (rtcAddr == PCF8563_I2C_ADDR) {
            I2CWriteByte(PCF8563_MIN);
        } else {
            I2CWriteByte(DS1307_MIN);
        }
        I2CWriteByte(rtcDecToBinDec(rtc.min));
        break;
    }
    case RTC_SEC: {
        if (rtcAddr == PCF8563_I2C_ADDR) {
            I2CWriteByte(PCF8563_SEC);
        } else {
            I2CWriteByte(DS1307_SEC);
        }
        I2CWriteByte(rtcDecToBinDec(rtc.sec));
    }
    break;
    case RTC_DATE:
    case RTC_MONTH:
    case RTC_YEAR: {
        if (rtcAddr == PCF8563_I2C_ADDR) {
            I2CWriteByte(PCF8563_DATE);
            I2CWriteByte(rtcDecToBinDec(rtc.date));
            I2CWriteByte(rtcDecToBinDec(rtcWeekDay()));
        } else {
            I2CWriteByte(DS1307_WDAY);
            I2CWriteByte(rtcDecToBinDec(rtcWeekDay()));
            I2CWriteByte(rtcDecToBinDec(rtc.date));
        }
        I2CWriteByte(rtcDecToBinDec(rtc.month));
        I2CWriteByte(rtcDecToBinDec(rtc.year));
    }
    break;
    default:
        break;
    }

    I2CStop();

    return;
}

void rtcNextEditParam()
{
    if (++rtc.etm >= RTC_NOEDIT)
        rtc.etm = RTC_HOUR;

    return;
}

void rtcChangeTime(int8_t diff)
{
    uint8_t etm = rtc.etm;

    if (etm >= RTC_NOEDIT)
        return;

    int8_t *time = (int8_t *)&rtc + etm;
    int8_t timeMax = pgm_read_byte((int8_t *)&rtcMax + etm);
    int8_t timeMin = pgm_read_byte((int8_t *)&rtcMin + etm);

    if (etm == RTC_DATE)
        timeMax = rtcDaysInMonth();

    *time += diff;

    if (*time > timeMax)
        *time = timeMin;
    if (*time < timeMin)
        *time = timeMax;

    rtcSaveTime();

    return;
}


uint8_t rtcBinDecToDec(uint8_t num)
{
    return (num >> 4) * 10 + (num & 0x0F);
}


uint8_t rtcDecToBinDec(uint8_t num)
{
    return ((num / 10) << 4) + (num % 10);
}
