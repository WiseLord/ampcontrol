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
static int8_t brStby;                       // Brightness in standby mode
static int8_t brWork;                       // Brightness in working mode

static SpMode spMode;                       // Spectrum mode

static char strbuf[STR_BUFSIZE + 1];        // String buffer

uint8_t *txtLabels[LABEL_END];              // Array with text label pointers

#if defined(_KS0066)
static LcdSymGroup userSybmols = LCD_END;   // Generated user symbols for ks0066
static LcdUserAddSym userAddSym = SYM_END;  // Additional user symbol
#endif

#if defined(_KS0066)
static void lcdGenLevels()
{
    if (userSybmols != LCD_LEVELS) {        // Generate 7 level symbols
        userSybmols = LCD_LEVELS;

        uint8_t i, j;
        ks0066SelectSymbol(0);
        for (i = 0; i < 7; i++)
            for (j = 0; j < 8; j++)
                if (i + j >= 7)
                    ks0066WriteData(0xFF);
                else
                    ks0066WriteData(0x00);
    }
}

static void lcdGenBar(LcdUserAddSym sym)
{
    static const uint8_t bar[] PROGMEM = {
        0b00000,
        0b10000,
        0b10100,
        0b10101,
        0b00101,
        0b00001,
    };
    static const uint8_t speakerIcon[] PROGMEM = {
        0b00001,
        0b00011,
        0b11101,
        0b10101,
        0b11101,
        0b00011,
        0b00001,
        0b00000,
    };
    static const uint8_t loudnessIcon[] PROGMEM = {
        0b00000,
        0b10000,
        0b10001,
        0b10101,
        0b10101,
        0b10101,
        0b10101,
        0b00000,
    };
    static const uint8_t surroundIcon[] PROGMEM = {
        0b01010,
        0b10001,
        0b10101,
        0b10101,
        0b10101,
        0b10001,
        0b01010,
        0b00000,
    };
    static const uint8_t effect3dIcon[] PROGMEM = {
        0b01110,
        0b10001,
        0b00000,
        0b01110,
        0b00000,
        0b10001,
        0b01110,
        0b00000,
    };
    static const uint8_t toneBypassIcon[] PROGMEM = {
        0b01000,
        0b10101,
        0b00010,
        0b00000,
        0b11111,
        0b00000,
        0b11111,
        0b00000,
    };
    static const uint8_t stereoIcon[] PROGMEM = {
        0b00000,
        0b11011,
        0b10101,
        0b10101,
        0b10101,
        0b11011,
        0b00000,
        0b00000,
    };
    static const uint8_t monoIcon[] PROGMEM = {
        0b00000,
        0b00111,
        0b01001,
        0b01001,
        0b01001,
        0b00111,
        0b00000,
        0b00000,
    };
    static const uint8_t crossIcon[] PROGMEM = {
        0b00000,
        0b10001,
        0b01010,
        0b00100,
        0b01010,
        0b10001,
        0b00000,
        0b00000,
    };
    static const uint8_t degreeIcon[] PROGMEM = {
        0b00110,
        0b01001,
        0b01001,
        0b00110,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
    };

    uint8_t i;

    if (userSybmols != LCD_BAR || userAddSym != sym) {
        // Generate main 6 bar symbols
        userSybmols = LCD_BAR;
        ks0066SelectSymbol(0);
        for (i = 0; i < 48; i++) {
            if ((i & 0x07) == 0x03) {
                ks0066WriteData(0x15);
            } else if ((i & 0x07) == 0x07) {
                ks0066WriteData(0x00);
            } else {
                ks0066WriteData(pgm_read_byte(&bar[i >> 3]));
            }
        }
        // Generate two additional symbols
        userAddSym = sym;
        switch (sym) {
        case SYM_MUTE_CROSS:
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&speakerIcon[i]));
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&crossIcon[i]));
            break;
        case SYM_LOUDNESS_CROSS:
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&loudnessIcon[i]));
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&crossIcon[i]));
            break;
        case SYM_SURROUND_CROSS:
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&surroundIcon[i]));
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&crossIcon[i]));
            break;
        case SYM_EFFECT_3D_CROSS:
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&effect3dIcon[i]));
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&crossIcon[i]));
            break;
        case SYM_TONE_BYPASS_CROSS:
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&toneBypassIcon[i]));
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&crossIcon[i]));
            break;
        case SYM_STEREO_MONO:
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&stereoIcon[i]));
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&monoIcon[i]));
            break;
        default:
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&stereoIcon[i]));
            for (i = 0; i < 8; i++)
                ks0066WriteData(pgm_read_byte(&degreeIcon[i]));
            break;
        }
    }
}
#endif

static void showBar(int16_t min, int16_t max, int16_t value)
{
#if defined(_KS0066)
    uint8_t i;

    lcdGenBar(userAddSym);

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
#else
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
#endif

    return;
}


static void writeStringEeprom(const uint8_t *string)
{
    eeprom_read_block(strbuf, string, STR_BUFSIZE);
    writeString(strbuf);
}

static void writeNum(int16_t number, uint8_t width, uint8_t lead, uint8_t radix)
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

    writeString(strbuf);
}


static void showParValue(int8_t value)
{
#if defined(_KS0066)
    ks0066SetXY(11, 0);
    writeNum(value, 3, ' ', 10);
#else
    ks0108LoadFont(font_ks0066_ru_24, 1);
    ks0108SetXY(93, 4);
    writeNum(value, 3, ' ', 10);
    ks0108LoadFont(font_ks0066_ru_08, 1);
#endif
}

static void showParLabel(uint8_t label)
{
#if defined (_KS0066)
    ks0066SetXY (0, 0);
    writeStringEeprom(txtLabels[label]);
#else
    ks0108LoadFont(font_ks0066_ru_24, 1);
    ks0108SetXY(0, 0);
    writeStringEeprom(txtLabels[label]);
#endif

    return;
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

#if defined(_KS0066)
    ks0066Init();
    lcdGenLevels();
#else
    ks0108Init();
    ks0108LoadFont(font_ks0066_ru_08, 1);
#endif
    OUT(DISP_BCKL);

    brStby = eeprom_read_byte((uint8_t *)EEPROM_BR_STBY);
    brWork = eeprom_read_byte((uint8_t *)EEPROM_BR_WORK);
    spMode  = eeprom_read_byte((uint8_t *)EEPROM_SP_MODE);
}

ISR (TIMER0_OVF_vect)
{
    // 1MHz / (255 - 155) = 10000Hz => 10kHz Fourier analysis
    TCNT0 = 155;

    ADCSRA |= 1 << ADSC;

    static uint8_t br;

    if (++br >= MAX_BRIGHTNESS)                         // Loop brightness
        br = MIN_BRIGHTNESS;

    if (br == _br) {
        CLR(DISP_BCKL);                                 // Turn backlight off
    } else if (br == 0)
        SET(DISP_BCKL);                                 // Turn backlight on

    return;
}

void showRCInfo()
{
    IRData irData = getIrData();

#if defined(_KS0066)
    ks0066SetXY(0, 0);
    writeString("R=");
    writeNum(irData.type, 1, '0', 16);
    ks0066SetXY(0, 1);
    writeString("RP=");
    writeNum(irData.repeat, 1, '0', 16);
    writeString(",RC=");
    writeNum(irData.address, 2, '0', 16);
    writeString(",CM=");
    writeNum(irData.command, 2, '0', 16);
#else
    ks0108LoadFont(font_ks0066_ru_08, 1);
    ks0108SetXY(0, 0);
    ks0108WriteString("RC5:");
    ks0108SetXY(5, 1);
    ks0108WriteString("Type = ");
    writeNum(irData.type, 1, '0', 16);
    ks0108SetXY(5, 2);
    ks0108WriteString("Rep = ");
    writeNum(irData.repeat, 1, '0', 16);
    ks0108SetXY(5, 3);
    ks0108WriteString("Adr = ");
    writeNum(irData.address, 2, '0', 16);
    ks0108SetXY(5, 4);
    ks0108WriteString("Cmd = ");
    writeNum(irData.command, 2, '0', 16);
    ks0108SetXY(0, 6);
    ks0108WriteString("Input:");
    ks0108SetXY(5, 7);
    writeNum(getEncBuf() | getBtnBuf(), 8, '0', 2);
#endif

    return;
}

void showRadio(uint8_t tune)
{
    tunerReadStatus();

    uint8_t i;

    uint8_t level = tunerLevel();
    uint8_t num = tunerStationNum();
#ifdef _RDS
    uint8_t rdsFlag = rdsGetFlag();
#endif

#if defined(_KS0066)
    lcdGenBar(SYM_STEREO_MONO);

    // Frequency value
    ks0066SetXY(0, 0);
    writeString("FM ");
    writeNum(tuner.rdFreq / 100, 3, ' ', 10);
    ks0066WriteData('.');
    writeNum(tuner.rdFreq % 100, 2, '0', 10);

    // Signal level
    ks0066SetXY(12, 0);
    level = level * 2 / 5;
    if (level < 3) {
        ks0066WriteData(level);
        ks0066WriteData(0x00);
    } else {
        ks0066WriteData(0x03);
        ks0066WriteData(level - 3);
    }

    // Stereo indicator
    ks0066SetXY(10, 0);
    if (tuner.mono)
        writeString("\x07");
    else if (tunerStereo())
        writeString("\x06");
    else
        writeString(" ");

    // Station number
    ks0066SetXY(14, 1);
    if (num) {
        writeNum(num, 2, ' ', 10);
    } else {
        writeString("--");
    }

    // Select between RDS and spectrum mode
#ifdef _RDS
    if (rdsFlag) {
        // RDS data
        ks0066SetXY(0, 1);
        writeString("RDS:");
        writeString(rdsGetText ());
    } else {
#endif
        // Frequency scale
        uint8_t value = (int16_t)36 * ((tuner.rdFreq - tuner.fMin) >> 4) / ((tuner.fMax - tuner.fMin) >> 4);
        ks0066SetXY(0, 1);
        for (i = 0; i < 12; i++) {
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
#ifdef _RDS
    }
#endif

    // Tune status
    ks0066SetXY (12, 1);
    if (tune == MODE_RADIO_TUNE) {
        writeString("<>");
    } else {
        writeString("  ");
    }
#else
    // Frequency value
    ks0108LoadFont(font_ks0066_ru_24, 1);
    ks0108SetXY(0, 0);
    ks0108WriteString("FM ");
    writeNum(tuner.rdFreq / 100, 3, ' ', 10);
    ks0108WriteString("\x7F.\x7F");
    writeNum(tuner.rdFreq % 100, 2, '0', 10);
    ks0108LoadFont(font_ks0066_ru_08, 1);

    // Signal level
    ks0108SetXY (112, 0);
    for (i = 0; i < 16; i += 2) {
        if (i <= level)
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
    showBar(tuner.fMin >> 4, tuner.fMax >> 4, tuner.rdFreq >> 4);

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
#endif

    return;
}

void showBoolParam(uint8_t value, uint8_t labelIndex)
{
    showParLabel(LABEL_MUTE);

#if defined(_KS0066)
    sndParam *param = &sndPar[MODE_SND_VOLUME];

    lcdGenBar(SYM_MUTE_CROSS + (labelIndex - LABEL_MUTE));
    ks0066SetXY(14, 0);
    ks0066WriteData(0x06);
    if (value)
        ks0066WriteData(0x07);
    else
        ks0066WriteData(' ');
    showBar((int8_t)pgm_read_byte(&param->grid->min), (int8_t)pgm_read_byte(&param->grid->max),
            param->value);
#else
    ks0108SetXY(0, 4);
    if (value)
        writeStringEeprom(txtLabels[LABEL_ON]);
    else
        writeStringEeprom(txtLabels[LABEL_OFF]);
    ks0108LoadFont(font_ks0066_ru_08, 1);
#endif

    return;
}

// Show brightness control
void showBrWork()
{
    showBar(MIN_BRIGHTNESS, MAX_BRIGHTNESS, brWork);
    showParValue(brWork);
    showParLabel(LABEL_BR_WORK);

    return;
}

void changeBrWork(int8_t diff)
{
    brWork += diff;
    if (brWork > MAX_BRIGHTNESS)
        brWork = MAX_BRIGHTNESS;
    if (brWork < MIN_BRIGHTNESS)
        brWork = MIN_BRIGHTNESS;
    setWorkBrightness();

    return;
}

// Show audio parameter
void showSndParam(uint8_t mode)
{
    sndParam *param = &sndPar[mode];

    showParLabel(mode);
    showParValue(((int16_t)(param->value) * (int8_t)pgm_read_byte(&param->grid->step) + 4) >> 3);
    showBar((int8_t)pgm_read_byte(&param->grid->min), (int8_t)pgm_read_byte(&param->grid->max),
            param->value);
#if defined (_KS0066)
    ks0066SetXY(14, 0);
    writeStringEeprom(txtLabels[LABEL_DB]);
#else
    ks0108LoadFont(font_ks0066_ru_08, 1);
    ks0108SetXY(116, 7);
    writeStringEeprom(txtLabels[LABEL_DB]);
#endif

    return;
}

#if defined(_KS0066)
static void drawTm(uint8_t tm)
{
    if (tm == rtc.etm && getClockTimer() < RTC_POLL_TIME / 2) {
        writeString("  ");
    } else {
        writeNum(*((int8_t *)&rtc + tm), 2, '0', 10);
    }
}
#else
static void drawTm(uint8_t tm, const uint8_t *font)
{
    if (tm == rtc.etm)
        ks0108LoadFont(font, 0);
    else
        ks0108LoadFont(font, 1);
    writeNum(*((int8_t *)&rtc + tm), 2, '0', 10);
    ks0108LoadFont(font, 1);

    return;
}
#endif

void showTime()
{
    rtcReadTime();
#if defined(_KS0066)
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
    writeString("20");
    drawTm(RTC_YEAR);

    ks0066SetXY(0, 1);
#else
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

#endif

    writeStringEeprom(txtLabels[LABEL_SUNDAY + rtcWeekDay() - 1]);

#if defined(_KS0066)
    ks0066SetXY(0, 0);
    drawTm (RTC_HOUR);
    ks0066WriteData (':');
    drawTm (RTC_MIN);
    ks0066WriteData (':');
    drawTm (RTC_SEC);

    ks0066SetXY(11, 0);
    drawTm(RTC_DATE);
    ks0066WriteData('.');
    drawTm(RTC_MONTH);

    ks0066SetXY(12, 1);
    ks0066WriteString("20");
    drawTm(RTC_YEAR);

    ks0066SetXY(0, 1);
#endif

    return;
}

void showSpectrum()
{
    getSpData(1);

#if defined(_KS0066)
    uint8_t i, data;
    uint16_t left, right;

    switch (spMode) {
    case SP_MODE_STEREO:
        lcdGenLevels();
        ks0066SetXY(0, 0);
        for (i = 0; i < KS0066_SCREEN_WIDTH; i++) {
            data = buf[i] >> 2;
            if (data >= 7)
                data = 0xFF;
            ks0066WriteData(data);
        }
        ks0066SetXY(0, 1);
        for (i = 0; i < KS0066_SCREEN_WIDTH; i++) {
            data = buf[FFT_SIZE / 2 + i] >> 2;
            if (data >= 7)
                data = 0xFF;
            ks0066WriteData(data);
        }
        break;
    case SP_MODE_MIXED:
        lcdGenLevels();
        for (i = 0; i < KS0066_SCREEN_WIDTH; i++) {
            data = buf[i];
            data += buf[FFT_SIZE / 2 + i];
            data >>= 2;
            ks0066SetXY(i, 0);
            if (data < 8)
                ks0066WriteData(' ');
            else if (data < 15)
                ks0066WriteData(data - 8);
            else
                ks0066WriteData(0xFF);
            ks0066SetXY(i, 1);
            if (data < 7)
                ks0066WriteData(data);
            else
                ks0066WriteData(0xFF);
        }
        break;
    default:
        lcdGenBar(userAddSym);
        left = 0;
        right = 0;
        for (i = 0; i < FFT_SIZE / 2; i++) {
            left += buf[i];
            right += buf[FFT_SIZE / 2 + i];
        }
        left >>= 4;
        right >>= 4;

        ks0066SetXY(0, 0);
        ks0066WriteData('L');
        for (i = 0; i < KS0066_SCREEN_WIDTH - 1; i++) {
            if (left / 3 > i) {
                ks0066WriteData(0x03);
            } else {
                if (left / 3 < i) {
                    ks0066WriteData(0x00);
                } else {
                    ks0066WriteData(left % 3);
                }
            }
        }
        ks0066SetXY(0, 1);
        ks0066WriteData('R');
        for (i = 0; i < KS0066_SCREEN_WIDTH - 1; i++) {
            if (right / 3 > i) {
                ks0066WriteData(0x03);
            } else {
                if (right / 3 < i) {
                    ks0066WriteData(0x00);
                } else {
                    ks0066WriteData(right % 3);
                }
            }
        }
        break;
    }
#else
    uint8_t i, j, k;
    int8_t row;
    uint8_t data;
    uint8_t val;

    if (spMode == SP_MODE_METER)
        spMode = SP_MODE_STEREO;

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
#endif

    return;
}

void setWorkBrightness(void)
{
    _br = brWork;

#if defined(KS0066_WIRE_PCF8574)
    if (brWork)
        pcf8574SetBacklight(BACKLIGHT_ON);
    else
        pcf8574SetBacklight(BACKLIGHT_OFF);
#endif

    return;
}

void setStbyBrightness(void)
{
    _br = brStby;

#if defined(KS0066_WIRE_PCF8574)
    pcf8574SetBacklight(BACKLIGHT_OFF);
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
    if (++spMode >= SP_MODE_END)
        spMode = SP_MODE_METER;
}
