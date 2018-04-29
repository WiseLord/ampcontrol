#include "display.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "input.h"
#include "tuner/tuner.h"
#include "adc.h"
#include "rtc.h"
#include "remote.h"

static int8_t brStby;                           // Brightness in standby mode
static int8_t brWork;                           // Brightness in working mode
static char strbuf[STR_BUFSIZE + 1];            // String buffer

uint8_t *txtLabels[LABEL_END];              // Array with text label pointers

static const uint8_t barSymbols[] PROGMEM = {
    0x00, 0x10, 0x14, 0x15,
};

static void writeStringEeprom(const uint8_t *string)
{
    uint8_t i = 0;

    for (i = 0; i < STR_BUFSIZE; i++)
        strbuf[i] = eeprom_read_byte(&string[i]);

    ks0066WriteString(strbuf);
}

static void lcdGenLevels()
{
    ks0066WriteCommand(KS0066_SET_CGRAM);

    uint8_t i, j;

    for (j = 0; j < 8; j++) {
        for (i = 0; i < 8; i++) {
            if (j + i >= 7) {
                ks0066WriteData(0xFF);
            } else {
                ks0066WriteData(0x00);
            }
        }
    }
}

static void lcdGenBar()
{
    ks0066WriteCommand(KS0066_SET_CGRAM);

    uint8_t i, j;
    uint8_t sym;

    for (j = 0; j < 4; j++) {
        for (i = 0; i < 8; i++) {
            sym = j;
            if (i == 3)
                sym = 3;
            if (i == 7)
                sym = 0;
            ks0066WriteData(pgm_read_byte(&barSymbols[sym]));
        }
    }
}

static void writeNum(int8_t value, uint8_t width, uint8_t lead)
{
    uint8_t sign = lead;
    int8_t pos;

    if (value < 0) {
        sign = '-';
        value = -value;
    }

    for (pos = 0; pos < width; pos++)
        strbuf[pos] = lead;
    strbuf[width] = '\0';
    pos = width - 1;

    while (value > 0 || pos == width - 1) {
        strbuf[pos] = value % 10 + 0x30;
        pos--;
        value /= 10;
    }
    if (pos >= 0)
        strbuf[pos] = sign;

    ks0066WriteString(strbuf);
}

static void writeHexDigit(uint8_t hex)
{
    if (hex > 9)
        hex += ('A' - '9' - 1);

    ks0066WriteData(hex + '0');
}

static void showBar(int16_t min, int16_t max, int16_t value)
{
    uint8_t i;

    lcdGenBar();
    ks0066SetXY(0, 1);

    value = (value - min) * 48 / (max - min);
    for (i = 0; i < 16; i++) {
        if (value / 3 > i) {
            ks0066WriteData(0x03);
        } else {
            if (value / 3 < i) {
                ks0066WriteData(0x00);
            } else {
                ks0066WriteData(value % 3);
            }
        }
    }
}

void showRCInfo()
{
    IRData irData = getIrData();

    ks0066WriteString("RC CM");
    ks0066SetXY(0, 1);
    writeHexDigit(irData.address >> 4);
    writeHexDigit(irData.address & 0x0F);
    ks0066WriteData(' ');
    writeHexDigit(irData.command >> 4);
    writeHexDigit(irData.command & 0x0F);
}

void showRadio(uint8_t mode)
{
    uint8_t num = tunerStationNum();
    tunerReadStatus();

    // Frequency value
    ks0066WriteString("FM");
    writeNum(tuner.freq / 100, 4, ' ');
    ks0066WriteData('.');
    writeNum(tuner.freq % 100, 2, '0');

    // Mono/stereo indicator
    ks0066SetXY(10, 0);
    ks0066WriteData(tunerStereo() ? 'S' : 'M');

    // Signal level
    // Temporary disabled

    // Station number
    ks0066SetXY(14, 0);
    if (mode == MODE_RADIO_TUNE && getClockTimer() < RTC_POLL_TIME / 2)
        ks0066WriteString("  ");
    else if (num)
        writeNum(num, 2, ' ');
    else
        ks0066WriteString("--");

    // Frequency scale
#ifdef _RDS
    uint8_t rdsFlag = rdsGetFlag();
    if (rdsFlag) {
        // RDS data
        ks0066SetXY(0, 1);
        ks0066WriteString("RDS:    ");
        ks0066WriteString(rdsGetText());
    } else {
#else
    {
#endif
        showBar(0, (tuner.fMax - tuner.fMin) >> 4, (tuner.freq - tuner.fMin) >> 4);
    }
}

void showBoolParam(uint8_t value, uint8_t labelIndex)
{
    writeStringEeprom(txtLabels[labelIndex]);
    ks0066SetXY(1, 1);
    if (value)
        writeStringEeprom(txtLabels[LABEL_ON]);
    else
        writeStringEeprom(txtLabels[LABEL_OFF]);
}

// Show brightness control
void showBrWork()
{
    writeStringEeprom(txtLabels[LABEL_BR_WORK]);

    ks0066SetXY(13, 0);
    writeNum(brWork, 3, ' ');

    showBar(DISP_MIN_BR, DISP_MAX_BR, brWork);
}

void changeBrWork(int8_t diff)
{
    brWork += diff;
    if (brWork > DISP_MAX_BR)
        brWork = DISP_MAX_BR;
    if (brWork < DISP_MIN_BR)
        brWork = DISP_MIN_BR;
    setWorkBrightness();
}

// Show audio parameter
void showSndParam(uint8_t mode)
{
    sndParam *param = &sndPar[mode];

    writeStringEeprom(txtLabels[mode]);

    //  (((int16_t)(param->value) * param->step + 4) >> 3);
    ks0066SetXY(11, 0);
    writeNum(param->value, 3, ' ');

    ks0066SetXY(14, 0);
    writeStringEeprom(txtLabels[LABEL_DB]);

    showBar((int8_t)pgm_read_byte(&param->grid->min), (int8_t)pgm_read_byte(&param->grid->max),
            param->value);
}

static void drawTm(uint8_t tm)
{
    if (tm == rtc.etm && getClockTimer() < RTC_POLL_TIME / 2)
        ks0066WriteString("  ");
    else
        writeNum(*((int8_t *)&rtc + tm), 2, '0');
}

void showTime()
{
    drawTm(RTC_HOUR);
    ks0066WriteData(':');
    drawTm(RTC_MIN);
    ks0066WriteData(':');
    drawTm(RTC_SEC);

    ks0066SetXY(11, 0);
    drawTm(RTC_DATE);
    ks0066WriteData('.');
    drawTm(RTC_MONTH);

    ks0066SetXY(12, 1);
    writeNum(20, 2, '0');
    drawTm(RTC_YEAR);

    ks0066SetXY(0, 1);

    writeStringEeprom(txtLabels[LABEL_SUNDAY + rtcWeekDay() - 1]);
}

void showSpectrum()
{
    uint8_t i;
    uint8_t val;
    uint8_t *buf = getSpData();

    lcdGenLevels();

    for (i = 0; i < 16; i++) {
        val = buf[i];
        if (val > 15)
            val = 15;
        ks0066SetXY(i, 0);
        if (val < 8)
            ks0066WriteData(0x20);
        else
            ks0066WriteData(val - 8);
        ks0066SetXY(i, 1);
        if (val < 8)
            ks0066WriteData(val);
        else
            ks0066WriteData(0xFF);
    }
}

void setWorkBrightness()
{
    setDispBr(brWork);
}

void setStbyBrightness()
{
    setDispBr(brStby);
}

void displayInit()
{
    uint8_t i;
    uint8_t *addr;

    // Load text labels from EEPROM
    addr = (uint8_t *)EEPROM_LABELS_ADDR;
    i = 0;

    while (i < LABEL_END && addr < (uint8_t *)EEPROM_SIZE) {
        if (eeprom_read_byte(addr) != '\0') {
            txtLabels[i] = addr;
            addr++;
            i++;
            while (eeprom_read_byte(addr) != '\0' &&
                    addr < (uint8_t *)EEPROM_SIZE) {
                addr++;
            }
        } else {
            addr++;
        }
    }

    ks0066Init();                       // Display

    brStby = eeprom_read_byte((uint8_t *)EEPROM_BR_STBY);
    brWork = eeprom_read_byte((uint8_t *)EEPROM_BR_WORK);
}

void displayPowerOff()
{
    eeprom_update_byte((uint8_t *)EEPROM_BR_WORK, brWork);
}
