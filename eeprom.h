#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>

// Parameters values
#define eepromVolume    ((void*)0x00)
#define eepromBass      ((void*)0x01)
#define eepromMiddle    ((void*)0x02)
#define eepromTreble    ((void*)0x03)
#define eepromPreamp    ((void*)0x04)
#define eepromBalance   ((void*)0x05)
#define eepromGain0     ((void*)0x06)
#define eepromGain1     ((void*)0x07)
#define eepromGain2     ((void*)0x08)
#define eepromGain3     ((void*)0x09)

// Some values stored in EEPROM
#define eepromLoudness  ((void*)0x0C)
#define eepromChannel   ((void*)0x0D)

#define eepromSpMode    ((void*)0x10)
#define eepromDisplay   ((void*)0x11)
#define eepromBrStby    ((void*)0x12)
#define eepromBrWork    ((void*)0x13)

#define EEPROM_FM_FREQ  0x16
#define EEPROM_FM_CTRL  0x18
#define EEPROM_FM_MONO  0x19
#define EEPROM_FM_STEP  0x1A

#define eepromRC5Addr   ((void*)0x1C)
#define eepromTempTH    ((void*)0x1D)

#define EEPROM_ADC_CORR_L       0x1E
#define EEPROM_ADC_CORR_R       0x1F

// RC5 commands array
#define eepromRC5Cmd    ((uint8_t*)0x40)

// FM stations
#define EEPROM_STATIONS         0x70
#define EEPROM_FAV_STATIONS     0x70

// Text labels (maximum 15 byte followed by \0)
#define labelsAddr      ((uint8_t*)0xF0)

#define EEPROM_SIZE             0x200

// EEPROM saved labels
enum {
    LABEL_VOLUME,
    LABEL_BASS,
    LABEL_MIDDLE,
    LABEL_TREBLE,
    LABEL_PREAMP,
    LABEL_FRONTREAR,
    LABEL_BALANCE,
    LABEL_GAIN0,
    LABEL_GAIN1,
    LABEL_GAIN2,
    LABEL_GAIN3,
    LABEL_MUTE,
    LABEL_LOUDNESS,
    LABEL_ON,
    LABEL_OFF,
    LABEL_DB,

    LABEL_SUNDAY,
    LABEL_MONDAY,
    LABEL_THUESDAY,
    LABEL_WEDNESDAY,
    LABEL_THURSDAY,
    LABEL_FRIDAY,
    LABEL_SADURDAY,

    LABEL_BR_WORK
};

#define LABELS_COUNT            24

#endif // EEPROM_H
