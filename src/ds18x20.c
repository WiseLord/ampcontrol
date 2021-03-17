#include "ds18x20.h"

#include <util/delay.h>
#include <util/crc16.h>
#include "pins.h"

static ds18x20Dev devs[DS18X20_MAX_DEV];
static uint8_t devCount = 0;

static uint8_t ds18x20IsOnBus()
{
    uint8_t ret;

    OUT(ONE_WIRE);  // Pin as output (0)
    CLR(ONE_WIRE);  // Set active 0
    _delay_us(480); // Reset
    IN(ONE_WIRE);   // Pin as input (1)
    SET(ONE_WIRE);  // Enable pull-up resitor
    _delay_us(70);  // Wait for response

    ret = !(READ(ONE_WIRE));

    _delay_us(410);

    return ret;
}

static void ds18x20SendBit(uint8_t bit)
{
    OUT(ONE_WIRE);  // Pin as output (0)
    CLR(ONE_WIRE);  // Set active 0
    _delay_us(5);
    if (!bit)
        _delay_us(50);
    IN(ONE_WIRE);   // Pin as input (1)
    SET(ONE_WIRE);  // Enable pull-up resitor
    _delay_us(5);
    if (bit)
        _delay_us(50);
}

static uint8_t ds18x20GetBit()
{
    uint8_t ret;

    OUT(ONE_WIRE);  // Pin as output (0)
    CLR(ONE_WIRE);  // Set active 0
    _delay_us(5);
    IN(ONE_WIRE);   // Pin as input (1)
    SET(ONE_WIRE);  // Enable pull-up resitor
    _delay_us(5);

    ret = READ(ONE_WIRE);

    _delay_us(50);

    return ret;
}

static void ds18x20SendByte(uint8_t byte)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        ds18x20SendBit(byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t ds18x20GetByte()
{
    uint8_t i, ret;

    ret = 0;
    for (i = 0; i < 8; i++) {
        ret >>= 1;
        if (ds18x20GetBit())
            ret |= 0x80;
    }

    return ret;
}

static void ds18x20Select(ds18x20Dev *dev)
{
    uint8_t i;

    ds18x20SendByte(DS18X20_CMD_MATCH_ROM);

    for (i = 0; i < 8; i++)
        ds18x20SendByte(dev->id[i]);
}

static void ds18x20GetAllTemps()
{
    uint8_t i, j;
    uint8_t crc;

    uint8_t arr[DS18X20_SCRATCH_LEN];

    for (i = 0; i < devCount; i++) {
        if (ds18x20IsOnBus()) {
            ds18x20Select(&devs[i]);
            ds18x20SendByte(DS18X20_CMD_READ_SCRATCH);

            // Control scratchpad checksum
            crc = 0;
            for (j = 0; j < DS18X20_SCRATCH_LEN; j++) {
                arr[j] = ds18x20GetByte();
                crc = _crc_ibutton_update(crc, arr[j]);
            }

            if (crc == 0) {
                // Save first 2 bytes (temperature) of scratchpad
                for (j = 0; j < DS18X20_SCRATCH_TEMP_LEN; j++)
                    devs[i].sp[j] = arr[j];
            }
        }
    }
}

static void ds18x20ConvertTemp()
{
    ds18x20SendByte(DS18X20_CMD_SKIP_ROM);
    ds18x20SendByte(DS18X20_CMD_CONVERT);

#ifdef DS18X20_PARASITE_POWER
    // Set active 1 on port for at least 750ms as parasitic power
    SET(ONE_WIRE);
    OUT(ONE_WIRE);
#endif
}

static uint8_t ds18x20SearchRom(uint8_t *bitPattern, uint8_t lastDeviation)
{
    uint8_t currBit;
    uint8_t newDeviation = 0;
    uint8_t bitMask = 0x01;
    uint8_t bitA;
    uint8_t bitB;

    // Send SEARCH ROM command on the bus
    ds18x20SendByte(DS18X20_CMD_SEARCH_ROM);

    // Walk through all 64 bits
    for (currBit = 0; currBit < DS18X20_ID_LEN * 8; currBit++) {
        // Read bit from bus twice.
        bitA = ds18x20GetBit();
        bitB = ds18x20GetBit();

        if (bitA && bitB) {                         // Both bits 1 = ERROR
            return 0xFF;
        } else if (!(bitA || bitB)) {               // Both bits 0
            if (currBit == lastDeviation) {         // Select 1 if device has been selected
                *bitPattern |= bitMask;
            } else if (currBit > lastDeviation) {   // Select 0 if no, and remember device
                (*bitPattern) &= ~bitMask;
                newDeviation = currBit;
            } else if (!(*bitPattern & bitMask)) {  // Otherwise just remember device
                newDeviation = currBit;
            }
        } else { // Bits differ
            if (bitA)
                *bitPattern |= bitMask;
            else
                *bitPattern &= ~bitMask;
        }

        // Send the selected bit to the bus.
        ds18x20SendBit(*bitPattern & bitMask);

        // Adjust bitMask and bitPattern pointer.
        bitMask <<= 1;
        if (!bitMask) {
            bitMask = 0x01;
            bitPattern++;
        }
    }

    return newDeviation;
}

void ds18x20SearchDevices()
{
    uint8_t i, j;
    uint8_t *newID;
    uint8_t *currentID;
    uint8_t lastDeviation;
    uint8_t count = 0;

    // Reset addresses
    for (i = 0; i < DS18X20_MAX_DEV; i++)
        for (j = 0; j < DS18X20_ID_LEN; j++)
            devs[i].id[j] = 0x00;

    // Search all sensors
    newID = devs[0].id;
    lastDeviation = 0;
    currentID = newID;

    do {
        for (j = 0; j < DS18X20_ID_LEN; j++)
            newID[j] = currentID[j];

        if (!ds18x20IsOnBus()) {
            devCount = 0;
            return;
        }

        lastDeviation = ds18x20SearchRom(newID, lastDeviation);

        currentID = newID;
        count++;
        newID = devs[count].id;

    } while (lastDeviation != 0);

    devCount = count;
}

uint8_t ds18x20Process()
{
    ds18x20GetAllTemps();

    // Convert temperature
    if (ds18x20IsOnBus())
        ds18x20ConvertTemp();

    return devCount;
}

int16_t ds18x20GetTemp(uint8_t num)
{
    int16_t ret = devs[num].temp * 5;

    // DS18B20 has 8X better resolution
    if (devs[num].id[0] == 0x28)
        ret /= 8;

    // Return value is in 0.1°C units
    return ret;
}

uint8_t ds18x20GetDevCount()
{
    return devCount;
}
