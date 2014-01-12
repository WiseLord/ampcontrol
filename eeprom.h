#ifndef EEPROM_H
#define EEPROM_H

/* Addresses in EEPROM with text labels */
#define volumeLabel		((uint8_t*)0x20)
#define bassLabel		((uint8_t*)0x30)
#define middleLabel		((uint8_t*)0x40)
#define trebleLabel		((uint8_t*)0x50)
#define balanceLabel	((uint8_t*)0x60)
#define preampLabel		((uint8_t*)0x70)
#define gainLabel		((uint8_t*)0x80)
#define fanLabel		((uint8_t*)0x90)
#define dbLabel			((uint8_t*)0xA0)
#define weekdayLabel	((uint8_t*)0xB0)

#endif // EEPROM_H
