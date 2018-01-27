#ifndef RTC_H
#define RTC_H

#include <inttypes.h>

//#define RTC_PCF8563

#ifdef RTC_PCF8563
#define RTC_I2C_ADDR        0xA2
#else
#define RTC_I2C_ADDR        0xD0
#endif

#ifdef RTC_PCF8563
enum {
    PCF8563_CS1,
    PCF8563_CS2,
    PCF8563_SEC,
    PCF8563_MIN,
    PCF8563_HOUR,
    PCF8563_DATE,
    PCF8563_WDAY,
    PCF8563_CENT_MONTH,
    PCF8563_YEAR,
};
#else
enum {
    DS1307_SEC,
    DS1307_MIN,
    DS1307_HOUR,
    DS1307_WDAY,
    DS1307_DATE,
    DS1307_MONTH,
    DS1307_YEAR,
    DS1307_CTRL,
};
#endif

enum {
    RTC_HOUR,
    RTC_MIN,
    RTC_SEC,
    RTC_DATE,
    RTC_MONTH,
    RTC_YEAR,

    RTC_NOEDIT,
};

typedef struct {
    int8_t hour;
    int8_t min;
    int8_t sec;
    int8_t date;
    int8_t month;
    int8_t year;

    int8_t etm;
} RTC_type;

extern RTC_type rtc;

int8_t rtcWeekDay(void);
void rtcReadTime(void);
void rtcNextEditParam(void);
void rtcChangeTime(int8_t diff);

uint8_t rtcBinDecToDec(uint8_t num);
uint8_t rtcDecToBinDec(uint8_t num);

#endif // RTC_H
