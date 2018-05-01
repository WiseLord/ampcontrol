#include "display.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "adc.h"
#include "eeprom.h"
#include "input.h"
#include "pins.h"
#include "remote.h"
#include "rtc.h"
#include "tuner/tuner.h"

static uint8_t _br;
static int8_t brStby;                           // Brightness in standby mode
static int8_t brWork;                           // Brightness in working mode

uint8_t spMode;                                 // Spectrum mode
static char strbuf[STR_BUFSIZE + 1];            // String buffer

uint8_t *txtLabels[LABEL_END];                  // Array with text label pointers

#if defined(_KS0066)
static uint8_t userSybmols = LCD_LEVELS;
#endif

static void writeStringEeprom(const uint8_t *string)
{
    eeprom_read_block(strbuf, string, STR_BUFSIZE);

#if defined(_KS0108)
    ks0108WriteString(strbuf);
#elif defined(_KS0066)
    ks0066WriteString(strbuf);
#elif defined(_LS020)
    ls020WriteString(strbuf);
#endif
}

#if defined(_KS0066)
static void lcdGenLevels(void)
{
    ks0066SelectSymbol(0);

    uint8_t i, j;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            if (i + j >= 7) {
                ks0066WriteData(0xFF);
            } else {
                ks0066WriteData(0x00);
            }
        }
    }

    return;
}
static void lcdGenBar(void)
{
    ks0066SelectSymbol(0);

    uint8_t i;
    uint8_t pos[] = {0x00, 0x10, 0x14, 0x15, 0x05, 0x01};

    for (i = 0; i < 48; i++) {
        if ((i & 0x07) == 0x03) {
            ks0066WriteData(0x15);
        } else if ((i & 0x07) == 0x07) {
            ks0066WriteData(0x00);
        } else {
            ks0066WriteData(pos[i >> 3]);
        }
    }
    // Stereo indicator
    ks0066WriteData(0b00000000);
    ks0066WriteData(0b00011011);
    ks0066WriteData(0b00010101);
    ks0066WriteData(0b00010101);
    ks0066WriteData(0b00010101);
    ks0066WriteData(0b00011011);
    ks0066WriteData(0b00000000);
    ks0066WriteData(0b00000000);

    return;
}
#endif

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

#if defined(_KS0108)
    ks0108Init();
    ks0108LoadFont(font_ks0066_ru_08, 1);
#elif defined(_KS0066)
    ks0066Init();
    lcdGenLevels();
#elif defined(_LS020)
    ls020Init();
#endif
    OUT(DISP_BCKL);

    brStby = eeprom_read_byte((uint8_t *)EEPROM_BR_STBY);
    brWork = eeprom_read_byte((uint8_t *)EEPROM_BR_WORK);
    spMode  = eeprom_read_byte((uint8_t *)EEPROM_SP_MODE);
}

ISR (TIMER0_OVF_vect)
{
    ADCSRA |= 1 << ADSC;                                // Start ADC every second interrupt

    static uint8_t br;

    if (++br >= DISP_MAX_BR)                            // Loop brightness
        br = DISP_MIN_BR;

    if (br == _br) {
        CLR(DISP_BCKL);                                 // Turn backlight off
    } else if (br == 0)
        SET(DISP_BCKL);                                 // Turn backlight on

    return;
}

void displayClear()
{
#if defined(_KS0108)
    ks0108Clear();
#elif defined(_KS0066)
    ks0066Clear();
#elif defined(_LS020)
    ls020Clear();
#endif

    return;
}

char *mkNumString(int16_t number, uint8_t width, uint8_t lead, uint8_t radix)
{
    uint8_t numdiv;
    uint8_t sign = lead;
    int8_t i;

    if (number < 0) {
        sign = '-';
        number = -number;
    }

    for (i = 0; i < width; i++)
        strbuf[i] = lead;
    strbuf[width] = '\0';
    i = width - 1;

    while (number > 0 || i == width - 1) {
        numdiv = number % radix;
        strbuf[i] = numdiv + 0x30;
        if (numdiv >= 10)
            strbuf[i] += 7;
        i--;
        number /= radix;
    }

    if (i >= 0)
        strbuf[i] = sign;

    return strbuf;
}

static void showBar(int16_t min, int16_t max, int16_t value)
{
#if defined(_KS0108)
    uint8_t data;
    uint8_t i, j;

    if (min + max) {
        value = (int16_t)85 * (value - min) / (max - min);
    } else {
        value = (int16_t)42 * value / max;
    }
    for (j = 5; j <= 6; j++) {
        ks0108SetXY(0, j);
        for (i = 0; i < 85; i++) {
            if (((min + max) && (value <= i)) || (!(min + max) &&
                                                  (((value > 0) && ((i < 42) || (value + 42 < i))) ||
                                                   ((value <= 0) && ((i > 42) || (value + 42 > i)))))) {
                if (j == 5) {
                    data = 0x80;
                } else {
                    data = 0x01;
                }
            } else {
                data = 0xFF;
            }
            if (i & 0x01) {
                data = 0x00;
            }
            ks0108WriteData(data);
        }
    }
#elif defined(_KS0066)
    uint8_t i;

    if (userSybmols != LCD_BAR) {
        lcdGenBar();
        userSybmols = LCD_BAR;
    }

    ks0066SetXY(0, 1);

    if (min + max) {
        value = (int16_t)48 * (value - min) / (max - min);
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
    } else {
        value = (int16_t)23 * value / max;
        if (value >= 0) {
            value++;
            for (i = 0; i < 7; i++) {
                ks0066WriteData(0x00);
            }
            ks0066WriteData(0x05);
            for (i = 0; i < 8; i++) {
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
        } else {
            value += 23;
            for (i = 0; i < 8; i++) {
                if (value / 3 > i) {
                    ks0066WriteData(0x00);
                } else {
                    if (value / 3 < i) {
                        ks0066WriteData(0x03);
                    } else {
                        ks0066WriteData(value % 3 + 3);
                    }
                }
            }
            ks0066WriteData(0x01);
            for (i = 0; i < 7; i++) {
                ks0066WriteData(0x00);
            }
        }
    }
#elif defined(_LS020)
    uint8_t i;

    if (min + max) {
        value = (int16_t)85 * (value - min) / (max - min);
    } else {
        value = (int16_t)42 * value / max;
    }
    ls020SetXY(0, 60);
    for (i = 0; i < 85; i++) {
        if (((min + max) && (value <= i)) || (!(min + max) &&
                                              (((value > 0) && ((i < 42) || (value + 42 < i))) ||
                                               ((value <= 0) && ((i > 42) || (value + 42 > i)))))) {
            ls020DrawRect(i * 2 + 2, 55, i * 2 + 2, 85, COLOR_BCKG);
            ls020DrawRect(i * 2 + 2, 69, i * 2 + 2, 71, COLOR_BLUE);
        } else {
            ls020DrawRect(i * 2 + 2, 55, i * 2 + 2, 85, COLOR_CYAN);
        }
    }
#endif

    return;
}

static void showParValue(int8_t value)
{
#if defined(_KS0108)
    ks0108LoadFont(font_ks0066_ru_24, 1);
    ks0108SetXY(93, 4);
    ks0108WriteString(mkNumString(value, 3, ' ', 10));
    ks0108LoadFont(font_ks0066_ru_08, 1);
#elif defined(_KS0066)
    ks0066SetXY(11, 0);
    ks0066WriteString(mkNumString(value, 3, ' ', 10));
#elif defined(_LS020)
    ls020LoadFont(font_digits_32, COLOR_CYAN, 1);
    ls020SetXY(100, 96);
    ls020WriteString(mkNumString(value, 3, ' ', 10));
#endif

    return;
}

static void showParLabel(uint8_t label, uint8_t **txtLabels)
{
#if defined(_KS0108)
    ks0108LoadFont(font_ks0066_ru_24, 1);
    ks0108SetXY(0, 0);
    writeStringEeprom(txtLabels[label]);
    ks0108LoadFont(font_ks0066_ru_08, 1);
#elif defined (_KS0066)
    ks0066SetXY(0, 0);
    writeStringEeprom(txtLabels[label]);
#elif defined(_LS020)
    ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
    ls020SetXY(4, 8);
    writeStringEeprom(txtLabels[label]);
#endif

    return;
}

void showRCInfo()
{
    IRData irData = getIrData();

#if defined(_KS0108)
    ks0108LoadFont(font_ks0066_ru_08, 1);
    ks0108SetXY(0, 0);
    ks0108WriteString("RC5:");
    ks0108SetXY(5, 1);
    ks0108WriteString("Type = ");
    ks0108WriteString(mkNumString(irData.type, 1, '0', 16));
    ks0108SetXY(5, 2);
    ks0108WriteString("Rep = ");
    ks0108WriteString(mkNumString(irData.repeat, 1, '0', 16));
    ks0108SetXY(5, 3);
    ks0108WriteString("Adr = ");
    ks0108WriteString(mkNumString(irData.address, 2, '0', 16));
    ks0108SetXY(5, 4);
    ks0108WriteString("Cmd = ");
    ks0108WriteString(mkNumString(irData.command, 2, '0', 16));
    ks0108SetXY(0, 6);
    ks0108WriteString("Input:");
    ks0108SetXY(5, 7);
    ks0108WriteString(mkNumString(getEncBuf() | getBtnBuf(), 8, '0', 2));
#elif defined(_KS0066)
    ks0066SetXY(0, 0);
    ks0066WriteString("R=");
    ks0066WriteString(mkNumString(irData.type, 1, '0', 16));
    ks0066SetXY(0, 1);
    ks0066WriteString("RP=");
    ks0066WriteString(mkNumString(irData.repeat, 1, '0', 16));
    ks0066WriteString(",RC=");
    ks0066WriteString(mkNumString(irData.address, 2, '0', 16));
    ks0066WriteString(",CM=");
    ks0066WriteString(mkNumString(irData.command, 2, '0', 16));
#elif defined(_LS020)
    ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
    ls020SetXY(4, 0);
    ls020WriteString(mkNumString(irData.type, 1, '0', 16));
    ls020SetXY(5, 24);
    ls020WriteString("Rep = ");
    ls020WriteString(mkNumString(irData.repeat, 1, '0', 16));
    ls020SetXY(6, 48);
    ls020WriteString("Adr = ");
    ls020WriteString(mkNumString(irData.address, 2, '0', 16));
    ls020SetXY(4, 72);
    ls020WriteString("Cmd = ");
    ls020WriteString(mkNumString(irData.command, 2, '0', 16));
    ls020SetXY(6, 104);
    ls020WriteString("Btn = ");
    ls020WriteString("=-=-=");
#endif

    return;
}

void showRadio(uint8_t tune)
{
    uint8_t num = tunerStationNum();
    tunerReadStatus();

    uint16_t freq = tuner.rdFreq;

#if defined(_KS0108)
    uint8_t i;

    // Frequency value
    ks0108LoadFont(font_ks0066_ru_24, 1);
    ks0108SetXY(0, 0);
    ks0108WriteString("FM ");
    ks0108WriteString(mkNumString(freq / 100, 3, ' ', 10));
    ks0108WriteString("\x7F.\x7F");
    ks0108WriteString(mkNumString(freq % 100, 2, '0', 10));
    ks0108LoadFont(font_ks0066_ru_08, 1);

    // Signal level
    ks0108SetXY (112, 0);
    for (i = 0; i < 16; i += 2) {
        if (i <= tunerLevel())
            ks0108WriteData(256 - (128 >> (i / 2)));
        else
            ks0108WriteData(0x80);
        ks0108WriteData(0x00);
    }

    // Stereo indicator
    ks0108SetXY(114, 2);
    if (tuner.mono) {
        ks0108WriteString("MO");
    } else if (tunerStereo())
        ks0108WriteString("ST");
    else
        ks0108WriteString("  ");

    // Station number
    if (num) {
        showParValue(num);
    } else {
        ks0108LoadFont(font_ks0066_ru_24, 1);
        ks0108SetXY(93, 4);
        ks0108WriteString(" --");
        ks0108LoadFont(font_ks0066_ru_08, 1);
    }

    // Frequency scale
    showBar(tuner.fMin >> 4, tuner.fMax >> 4, freq >> 4);

    ks0108LoadFont(font_ks0066_ru_08, 1);
    if (tune == MODE_RADIO_TUNE) {
        ks0108SetXY(108, 7);
        ks0108WriteString("<<>>");
    } else {
        ks0108SetXY(110, 7);
#ifdef _RDS
        if (tuner.rds) {
            if (rdsFlag)
                ks0108WriteString("RDS");
            else
                ks0108WriteString("---");
        } else {
            ks0108WriteString("   ");
        }
#else
        ks0108WriteString("   ");
#endif
    }
#elif defined(_KS0066)
    uint8_t lev;

    // Frequency value
    ks0066SetXY(0, 0);
    ks0066WriteString("FM ");
    ks0066WriteString(mkNumString(freq / 100, 3, ' ', 10));
    ks0066WriteData('.');
    ks0066WriteString(mkNumString(freq / 10 % 10, 1, ' ', 10));

    // Stereo indicator
    ks0066SetXY(9, 0);
    if (tunerStereo())
        ks0066WriteString("\x06");
    else
        ks0066WriteString(" ");

    // Signal level
    ks0066SetXY(11, 0);
    lev = tunerLevel() * 2 / 5;
    if (lev < 3) {
        ks0066WriteData(lev);
        ks0066WriteData(0x00);
    } else {
        ks0066WriteData(0x03);
        ks0066WriteData(lev - 3);
    }

    // Frequency scale
    showBar(tuner.fMin >> 4, tuner.fMax >> 4, freq >> 4);

    // Station number
    ks0066SetXY(14, 0);
    if (num) {
        ks0066WriteString(mkNumString(num, 2, ' ', 10));
    } else {
        ks0066WriteString("--");
    }
#elif defined(_LS020)

    // Frequency value
    ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
    ls020SetXY(4, 8);
    ls020WriteString("FM ");
    ls020WriteString(mkNumString(freq / 100, 3, ' ', 10));
    ls020WriteString("\x7F.\x7F");
    ls020WriteString(mkNumString(freq / 10 % 10, 1, ' ', 10));

    // Stereo indicator
    ls020LoadFont(font_ks0066_ru_08, COLOR_CYAN, 1);
    ls020SetXY(132, 12);
    if (tunerStereo())
        ls020WriteString("STEREO");
    else
        ls020WriteString("      ");

    // Frequency scale
    showBar(tuner.fMin >> 4, tuner.fMax >> 4, freq >> 4);

    // Station number
    ls020LoadFont(font_digits_32, COLOR_CYAN, 1);
    ls020SetXY(124, 96);
    if (num) {
        ls020WriteString(mkNumString(num, 3, ' ', 10));
    } else {
        ls020WriteString(" --");
    }
#endif

    return;
}

void showBoolParam(uint8_t value, uint8_t labelIndex)
{
#if defined(_KS0108)
    ks0108LoadFont(font_ks0066_ru_24, 1);
    ks0108SetXY(0, 0);
    writeStringEeprom(txtLabels[labelIndex]);
    ks0108SetXY(0, 4);
    if (value)
        writeStringEeprom(txtLabels[LABEL_ON]);
    else
        writeStringEeprom(txtLabels[LABEL_OFF]);
    ks0108LoadFont(font_ks0066_ru_08, 1);
#elif defined(_KS0066)
    ks0066SetXY(0, 0);
    writeStringEeprom(txtLabels[labelIndex]);
    ks0066SetXY(1, 1);
    if (value)
        writeStringEeprom(txtLabels[LABEL_ON]);
    else
        writeStringEeprom(txtLabels[LABEL_OFF]);
#elif defined(_LS020)
    ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
    ls020SetXY(4, 0);
    writeStringEeprom(txtLabels[labelIndex]);
    ls020SetXY(4, 32);
    if (value)
        writeStringEeprom(txtLabels[LABEL_ON]);
    else
        writeStringEeprom(txtLabels[LABEL_OFF]);
#endif

    return;
}

// Show brightness control
void showBrWork()
{
    showBar(DISP_MIN_BR, DISP_MAX_BR, brWork);
    showParValue(brWork);
    showParLabel(LABEL_BR_WORK, txtLabels);

    return;
}

void changeBrWork(int8_t diff)
{
    brWork += diff;
    if (brWork > DISP_MAX_BR)
        brWork = DISP_MAX_BR;
    if (brWork < DISP_MIN_BR)
        brWork = DISP_MIN_BR;
    setWorkBrightness();

    return;
}

// Show audio parameter
void showSndParam(uint8_t mode)
{
    sndParam *param = &sndPar[mode];

    showParLabel(mode, txtLabels);
    showParValue(((int16_t)(param->value) * (int8_t)pgm_read_byte(&param->grid->step) + 4) >> 3);
    showBar((int8_t)pgm_read_byte(&param->grid->min), (int8_t)pgm_read_byte(&param->grid->max),
            param->value);
#if defined(_KS0108)
    ks0108LoadFont(font_ks0066_ru_08, 1);
    ks0108SetXY(116, 7);
    writeStringEeprom(txtLabels[LABEL_DB]);
#elif defined (_KS0066)
    ks0066SetXY(14, 0);
    writeStringEeprom(txtLabels[LABEL_DB]);
#elif defined(_LS020)
    ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
    ls020SetXY(150, 104);
    writeStringEeprom(txtLabels[LABEL_DB]);
#endif

    return;
}

#if defined(_KS0108)
static void drawTm(uint8_t tm, const uint8_t *font)
{
    if (tm == rtc.etm)
        ks0108LoadFont(font, 0);
    else
        ks0108LoadFont(font, 1);
    ks0108WriteString(mkNumString(*((int8_t *)&rtc + tm), 2, '0', 10));
    ks0108LoadFont(font, 1);

    return;
}
#elif defined(_KS0066)
static void drawTm(uint8_t tm)
{
    ks0066WriteString(mkNumString(*((int8_t *)&rtc + tm), 2, '0', 10));

    return;
}
#elif defined(_LS020)
static void drawTm(uint8_t tm, const uint8_t *font, uint8_t mult)
{
    if (rtc.etm == tm)
        ls020LoadFont(font, COLOR_RED, mult);
    else
        ls020LoadFont(font, COLOR_CYAN, mult);
    ls020WriteString(mkNumString(*((int8_t *)&rtc + tm), 2, '0', 10));
    ls020LoadFont(font, COLOR_CYAN, mult);

    return;
}
#endif

void showTime()
{
    rtcReadTime();
#if defined(_KS0108)
    ks0108SetXY(4, 0);

    drawTm(RTC_HOUR, font_digits_32);
    ks0108WriteString("\x7F:\x7F");
    drawTm(RTC_MIN, font_digits_32);
    ks0108WriteString("\x7F:\x7F");
    drawTm(RTC_SEC, font_digits_32);

    ks0108SetXY(9, 4);

    drawTm(RTC_DATE, font_ks0066_ru_24);
    ks0108WriteString("\x7F.\x7F");
    drawTm(RTC_MONTH, font_ks0066_ru_24);
    ks0108WriteString("\x7F.\x7F");
    if (rtc.etm == RTC_YEAR)
        ks0108LoadFont(font_ks0066_ru_24, 0);
    ks0108WriteString("20");
    ks0108WriteChar('\x7F');
    drawTm(RTC_YEAR, font_ks0066_ru_24);

    ks0108LoadFont(font_ks0066_ru_08, 1);
    ks0108SetXY(32, 7);

#elif defined(_KS0066)
    ks0066SetXY(0, 0);

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
    ks0066WriteString("20");
    drawTm(RTC_YEAR);

    ks0066SetXY(0, 1);
#elif defined(_LS020)
    ls020SetXY(2, 4);

    drawTm(RTC_HOUR, font_ks0066_ru_24, 2);
    ls020WriteString("\x7F:\x7F");
    drawTm(RTC_MIN, font_ks0066_ru_24, 2);
    ls020WriteString("\x7F:\x7F");
    drawTm(RTC_SEC, font_ks0066_ru_24, 2);

    ls020SetXY(12, 64);

    drawTm(RTC_DATE, font_digits_32, 1);
    ls020WriteString("\x7F.\x7F");
    drawTm(RTC_MONTH, font_digits_32, 1);
    ls020WriteString("\x7F.\x7F");
    if (rtc.etm == RTC_YEAR)
        ls020LoadFont(font_digits_32, COLOR_RED, 1);
    ls020WriteString("20");
    ls020WriteChar('\x7F');
    drawTm(RTC_YEAR, font_digits_32, 1);

    ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
    ls020SetXY(24, 104);
#endif

    writeStringEeprom(txtLabels[LABEL_SUNDAY + rtcWeekDay() - 1]);

#if defined(_KS0066)
    if (rtc.etm == RTC_NOEDIT) {
        ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
    } else {
        switch (rtc.etm) {
        case RTC_HOUR:
            ks0066SetXY(1, 0);
            break;
        case RTC_MIN:
            ks0066SetXY(4, 0);
            break;
        case RTC_SEC:
            ks0066SetXY(7, 0);
            break;
        case RTC_DATE:
            ks0066SetXY(12, 0);
            break;
        case RTC_MONTH:
            ks0066SetXY(15, 0);
            break;
        case RTC_YEAR:
            ks0066SetXY(15, 1);
            break;
        default:
            break;
        }
        ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON | KS0066_CUR_BLINK_ON);
    }
#endif

    return;
}

void showSpectrum()
{
    getSpData(1);

#if defined(_KS0108)
    uint8_t i, j, k;
    int8_t row;
    uint8_t data;
    uint8_t val;
    ks0108SetXY(0, 0);
    for (i = 0; i < KS0108_ROWS; i++) {
        for (j = 0, k = 32; j < 32; j++, k++) {
            switch (spMode) {
            case SP_MODE_STEREO:
                if (i < KS0108_ROWS / 2) {
                    val = buf[j];
                    row = 3 - val / 8;
                } else {
                    val = buf[k];
                    row = 7 - val / 8;
                }
                break;
            default:
                val = buf[j] + buf[k];
                row = 7 - val / 8;
                break;
            }
            data = 0xFF;
            if (i == row) {
                data = 0xFF << (7 - val % 8);
            } else if (i < row) {
                data = 0x00;
            }
            ks0108WriteData(data);
            ks0108WriteData(data);
            ks0108WriteData(data);
            ks0108WriteData(0x00);
        }
    }
#elif defined(_KS0066)
    uint8_t i;
    uint8_t lcdBuf[16];

    if (userSybmols != LCD_LEVELS) {
        lcdGenLevels();
        userSybmols = LCD_LEVELS;
    }

    for (i = 0; i < 16; i++) {
        lcdBuf[i] = buf[2 * i] + buf[2 * i + 1];
        lcdBuf[i] += buf[32 + 2 * i] + buf[32 + 2 * i + 1];
        lcdBuf[i] >>= 2;
    }

    for (i = 0; i < 16; i++) {
        ks0066SetXY(i, 0);
        if (lcdBuf[i] < 8)
            ks0066WriteData(0x20);
        else
            ks0066WriteData(lcdBuf[i] - 8);
        ks0066SetXY(i, 1);
        if (lcdBuf[i] < 8)
            ks0066WriteData(lcdBuf[i]);
        else
            ks0066WriteData(0xFF);
    }

#elif defined(_LS020)
    uint8_t i;
    uint8_t val;

    for (i = 0; i < 29; i++) {
        switch (spMode) {
        case SP_MODE_STEREO:
            val = buf[i] * 2;
            ls020DrawRect(2 + i * 6, 2, 5 + i * 6, 2 + (63 - val) - 1, COLOR_BCKG);
            ls020DrawRect(2 + i * 6, 2 + (63 - val), 5 + i * 6, 65, COLOR_YELLOW);
            val = buf[i + 32] * 2;
            ls020DrawRect(2 + i * 6, 2 + 64, 5 + i * 6, 2 + (63 - val) - 1 + 64, COLOR_BCKG);
            ls020DrawRect(2 + i * 6, 2 + (63 - val) + 64, 5 + i * 6, 65 + 64, COLOR_YELLOW);
            break;
        default:
            val = (buf[i] + buf[i + 32]) * 2;
            ls020DrawRect(2 + i * 6, 2, 5 + i * 6, 2 + (127 - val) - 1, COLOR_BCKG);
            ls020DrawRect(2 + i * 6, 2 + (127 - val), 5 + i * 6, 129, COLOR_YELLOW);
            break;
        }
    }
#endif

    return;
}

void setWorkBrightness(void)
{
    _br = brWork;

#if defined(_KS0066_WIRE_PCF8574)
    if (brWork == DISP_MAX_BR)
        pcf8574IntBacklight(BACKLIGHT_ON);
    else
        pcf8574IntBacklight(BACKLIGHT_OFF);
#endif

    return;
}

void setStbyBrightness(void)
{
    _br = brStby;

#if defined(_KS0066_WIRE_PCF8574)
    pcf8574IntBacklight(BACKLIGHT_OFF);
#endif

    return;
}

void displayPowerOff(void)
{
    eeprom_update_byte((uint8_t *)EEPROM_BR_WORK, brWork);
    eeprom_update_byte((uint8_t *)EEPROM_SP_MODE, spMode);

    return;
}

// Change spectrum mode
void switchSpMode()
{
    spMode = !spMode;

    return;
}
