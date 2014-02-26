#ifndef EEPROM_H
#define EEPROM_H

/* Parameters values */

#define eepromVolume	((void*)0x00)
#define eepromBass		((void*)0x01)
#define eepromMiddle	((void*)0x02)
#define eepromTreble	((void*)0x03)
#define eepromPreamp	((void*)0x04)
#define eepromBalance	((void*)0x05)
#define eepromGain0		((void*)0x06)
#define eepromGain1		((void*)0x07)
#define eepromGain2		((void*)0x08)
#define eepromGain3		((void*)0x09)

#define eepromRC5Addr	((void*)0x10)
#define eepromChannel	((void*)0x11)
#define eepromSpMode	((void*)0x12)
#define eepromLoudness	((void*)0x13)
#define eepromChanCnt	((void*)0x14)
#define eepromICSelect	((void*)0x15)
#define eepromBCKL		((void*)0x16)
#define eepromEXT2		((void*)0x17)

/* Parameters min/max/step values */

#define eepromMinimums	((void*)0x20)
#define eepromMaximums	((void*)0x30)
#define eepromSteps		((void*)0x40)

/* RC5 commands array */

#define eepromRC5Cmd	((uint8_t*)0x50)

/* Text labels (maximum 15 byte followed by \0) */

#define volumeLabel		((uint8_t*)0x070)
#define bassLabel		((uint8_t*)0x080)
#define middleLabel		((uint8_t*)0x090)
#define loudnessLabel	((uint8_t*)0x090)
#define trebleLabel		((uint8_t*)0x0A0)
#define preampLabel		((uint8_t*)0x0B0)
#define balanceLabel	((uint8_t*)0x0C0)
#define gainLabel0		((uint8_t*)0x0D0)
#define gainLabel1		((uint8_t*)0x0E0)
#define gainLabel2		((uint8_t*)0x0F0)
#define gainLabel3		((uint8_t*)0x100)
#define muteLabel		((uint8_t*)0x110)
#define onLabel			((uint8_t*)0x120)
#define offLabel		((uint8_t*)0x130)

#define dbLabel			((uint8_t*)0x170)
#define mondayLabel		((uint8_t*)0x180)
#define thuesDayLabel	((uint8_t*)0x190)
#define wednesdayLabel	((uint8_t*)0x1A0)
#define thursdayLabel	((uint8_t*)0x1B0)
#define fridayLabel		((uint8_t*)0x1C0)
#define saturdayLabel	((uint8_t*)0x1D0)
#define sundayLabel		((uint8_t*)0x1E0)

#endif // EEPROM_H
