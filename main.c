#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "adc.h"
#include "input.h"
#include "remote.h"
#include "i2c.h"

#include "audio/audio.h"
#include "display.h"

#include "tuner/tuner.h"

uint8_t *txtLabels[LABEL_END];   // Array with text label pointers

// Save parameters to EEPROM
static void saveParams(void)
{
    sndPowerOff();
    saveDisplayParams();
    tunerPowerOff();

    return;
}

// Handle leaving standby mode
static void powerOn(void)
{
    STMU_PORT |= STDBY;
    setWorkBrightness();

    _delay_ms(500);                 // Wait while power is being set up

    tunerPowerOn();
    tunerSetMute(0);
    sndPowerOff();

    return;
}

// Handle entering standby mode
static void powerOff(void)
{
    sndSetMute(1);
    tunerPowerOff();

    _delay_ms(500);

    STMU_PORT &= ~STDBY;

    setStbyBrightness();
    stopEditTime();

    return;
}

// Load text labels from EEPROM
static void loadLabels(uint8_t **txtLabels)
{
    uint8_t i;
    uint8_t *addr;

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

    return;
}

// Hardware initialization
static void hwInit(void)
{
#ifdef _HARDWARE_RST
#ifdef _SI470X
    si470xReset();
#endif
#endif
    I2CInit();                      // I2C bus

    loadLabels(txtLabels);          // Load text labels from EEPROM
    loadDispParams();               // Load display params
    displayInit();

    inputInit();                    // Buttons/encoder polling

    rcInit();                       // IR Remote control
    adcInit();                      // Analog-to-digital converter

    STMU_DDR |= (STDBY | MUTE)  ;   // Standby/Mute port
    STMU_PORT &= ~STDBY;

    sei();                          // Gloabl interrupt enable

    tunerInit();                    // Tuner

    return;
}

int main(void)
{
    uint8_t dispMode = MODE_STANDBY;
    uint8_t dispModePrev = dispMode;

    int8_t encCnt = 0;
    uint8_t cmd = CMD_END;
    uint16_t rc5Buf = RC5_BUF_EMPTY;
    uint16_t rc5BufPrev = RC5_BUF_EMPTY;

    hwInit();

    while (1) {
        encCnt = getEncoder();
        cmd = getBtnCmd();
        rc5Buf = getRcCmd();

        // Don't handle any command in test mode
        if (dispMode == MODE_TEST) {
            if (cmd != CMD_END)
                setDisplayTime(DISPLAY_TIME_TEST);
            cmd = CMD_END;
        }

        // Don't handle any command in standby mode except power on
        if (dispMode == MODE_STANDBY) {
            if (cmd != CMD_BTN_1 && cmd != CMD_RC5_STBY &&
                    cmd != CMD_BTN_TESTMODE)
                cmd = CMD_END;
        }

        // Handle command
        switch (cmd) {
        case CMD_BTN_1:
        case CMD_RC5_STBY:
            switch (dispMode) {
            case MODE_STANDBY:
                powerOn();
                dispMode = MODE_SPECTRUM;
                break;
            default:
                powerOff();
                saveParams();
                dispMode = MODE_STANDBY;
                break;
            }
            break;
        case CMD_BTN_2:
        case CMD_RC_IN_NEXT:
/*
            switch (dispMode) {
            case MODE_GAIN:
                nextChan();
                clearDisplay();
            default:
                curSndParam = sndParAddr(SND_GAIN0 + getChan());
                dispMode = MODE_GAIN;
                setDisplayTime(DISPLAY_TIME_GAIN);
                break;
            }
*/
            break;
        case CMD_BTN_3:
        case CMD_RC_TIME:
            switch (dispMode) {
            case MODE_TIME:
            case MODE_TIME_EDIT:
                editTime();
                dispMode = MODE_TIME_EDIT;
                setDisplayTime(DISPLAY_TIME_TIME_EDIT);
                if (!isETM())
                    setDisplayTime(DISPLAY_TIME_TIME);
                break;
            case MODE_FM_RADIO:
                if (cmd == CMD_BTN_3) {
                    tunerChangeFreq(-1);
                    setDisplayTime(DISPLAY_TIME_FM_RADIO);
                    break;
                }
            default:
                stopEditTime();
                dispMode = MODE_TIME;
                setDisplayTime(DISPLAY_TIME_TIME);
                break;
            }
            break;
        case CMD_BTN_4:
        case CMD_RC5_MUTE:
            switch (dispMode) {
            case MODE_FM_RADIO:
                if (cmd == CMD_BTN_4) {
                    tunerChangeFreq(+1);
                    setDisplayTime(DISPLAY_TIME_FM_RADIO);
                    break;
                }
            default:
                clearDisplay();
                sndSetMute(!aproc.mute);
                dispMode = MODE_MUTE;
                setDisplayTime(DISPLAY_TIME_CHAN);
                break;
            }
            break;
        case CMD_BTN_5:
        case CMD_RC_NEXT_SNDPAR:
            sndNextParam(&dispMode);
            setDisplayTime(DISPLAY_TIME_AUDIO);
            break;
        case CMD_BTN_1_LONG:
        case CMD_RC_BRIGHTNESS:
            dispMode = MODE_BR;
            setDisplayTime(DISPLAY_TIME_BR);
            break;
        case CMD_BTN_2_LONG:
        case CMD_RC_DEF_DISPLAY:
            switch (getDefDisplay()) {
            case MODE_SPECTRUM:
                if (aproc.input == 0) {
                    setDefDisplay(MODE_FM_RADIO);
                    break;
                }
            case MODE_FM_RADIO:
                setDefDisplay(MODE_TIME);
                break;
            default:
                setDefDisplay(MODE_SPECTRUM);
                break;
            }
            dispMode = getDefDisplay();
            break;
        case CMD_BTN_3_LONG:
        case CMD_BTN_4_LONG:
        case CMD_BTN_5_LONG:
        case CMD_RC_FM_STORE:
            if (dispMode == MODE_FM_RADIO) {
                if (cmd == CMD_BTN_3_LONG)
                    tunerNextStation(SEARCH_DOWN);
                else if (cmd == CMD_BTN_4_LONG)
                    tunerNextStation(SEARCH_UP);
                else
                    tunerStoreStation();
                setDisplayTime(DISPLAY_TIME_FM_RADIO);
            } else {
                if (cmd == CMD_BTN_3_LONG) {
                    switchSpMode();
                    dispMode = MODE_SPECTRUM;
                    setDisplayTime(DISPLAY_TIME_SP);
#if defined(TDA7313)
                } else if (cmd == CMD_BTN_4_LONG) {
                    clearDisplay();
                    switchLoudness();
                    dispMode = MODE_LOUDNESS;
                    setDisplayTime(DISPLAY_TIME_AUDIO);
#endif
                }
            }
            break;
        case CMD_BTN_TESTMODE:
            switch (dispMode) {
            case MODE_STANDBY:
                dispMode = MODE_TEST;
                setDisplayTime(DISPLAY_TIME_TEST);
                break;
            }
            break;
#if defined(TDA7313)
        case CMD_RC5_LOUDNESS:
            clearDisplay();
            switchLoudness();
            dispMode = MODE_LOUDNESS;
            setDisplayTime(DISPLAY_TIME_AUDIO);
            break;
#endif
        case CMD_RC_IN_0:
        case CMD_RC_IN_1:
        case CMD_RC_IN_2:
        case CMD_RC_IN_3:
        case CMD_RC_IN_4:
            clearDisplay();
            sndSetInput(cmd - CMD_RC_IN_0);
            dispMode = MODE_SND_GAIN0 + aproc.input;
            setDisplayTime(DISPLAY_TIME_GAIN);
            break;
        case CMD_RC_NEXT_SPMODE:
            switchSpMode();
            dispMode = MODE_SPECTRUM;
            setDisplayTime(DISPLAY_TIME_SP);
            break;
        case CMD_RC_FM_INC:
        case CMD_RC_FM_DEC:
            sndSetInput(0);
            if (dispMode == MODE_FM_RADIO) {
                switch (cmd) {
                case CMD_RC_FM_INC:
                    tunerNextStation(SEARCH_UP);
                    break;
                case CMD_RC_FM_DEC:
                    tunerNextStation(SEARCH_DOWN);
                    break;
                }
            }
            dispMode = MODE_FM_RADIO;
            setDisplayTime(DISPLAY_TIME_FM_RADIO);
            break;
        case CMD_RC_FM_MONO:
            if (aproc.input == 0) {
                tunerSetMono(!tuner.mono);
                dispMode = MODE_FM_RADIO;
                setDisplayTime(DISPLAY_TIME_FM_RADIO);
            }
            break;
        case CMD_RC_FM_0:
        case CMD_RC_FM_1:
        case CMD_RC_FM_2:
        case CMD_RC_FM_3:
        case CMD_RC_FM_4:
        case CMD_RC_FM_5:
        case CMD_RC_FM_6:
        case CMD_RC_FM_7:
        case CMD_RC_FM_8:
        case CMD_RC_FM_9:
            sndSetInput(0);
            tunerLoadStation(cmd == CMD_RC_FM_0 ? 9 : cmd - CMD_RC_FM_1);
            dispMode = MODE_FM_RADIO;
            setDisplayTime(DISPLAY_TIME_FM_RADIO);
            break;
        }

        // Emulate RC5 VOL_UP/VOL_DOWN as encoder actions
        if (cmd == CMD_RC_VOL_UP)
            encCnt++;
        if (cmd == CMD_RC_VOL_DOWN)
            encCnt--;

        // Handle encoder
        if (encCnt) {
            switch (dispMode) {
            case MODE_STANDBY:
                break;
            case MODE_TEST:
                setDisplayTime(DISPLAY_TIME_TEST);
                break;
            case MODE_TIME_EDIT:
                changeTime(encCnt);
                setDisplayTime(DISPLAY_TIME_TIME_EDIT);
                break;
            case MODE_BR:
                changeBrWork(encCnt);
                setDisplayTime(DISPLAY_TIME_BR);
                break;
            case MODE_MUTE:
            case MODE_LOUDNESS:
            case MODE_SPECTRUM:
            case MODE_TIME:
            case MODE_FM_RADIO:
                dispMode = MODE_SND_VOLUME;
            default:
                sndSetMute(0);
                sndChangeParam(dispMode, encCnt);
                setDisplayTime(DISPLAY_TIME_GAIN);
                break;
            }
        }

        // Exit to default mode and save params to EEPROM
        if (getDisplayTime() == 0) {
            switch (dispMode) {
            case MODE_STANDBY:
                break;
            case MODE_TEST:
                setStbyBrightness();
                break;
            default:
                dispMode = getDefDisplay();
                if (dispMode == MODE_FM_RADIO && aproc.input)
                    dispMode = MODE_SPECTRUM;
                break;
            }
        }

        // Clear screen if mode has changed
        if (dispMode != dispModePrev)
            clearDisplay();

        // Show things
        switch (dispMode) {
        case MODE_STANDBY:
            showTime(txtLabels);
            if (dispModePrev == MODE_TEST)
                setStbyBrightness();
            break;
        case MODE_TEST:
            showRC5Info(rc5Buf);
            setWorkBrightness();
            if (rc5Buf != rc5BufPrev)
                setDisplayTime(DISPLAY_TIME_TEST);
            break;
        case MODE_SPECTRUM:
            getSpData(1);
            drawSpectrum();
            break;
        case MODE_FM_RADIO:
            tunerReadStatus();
            showRadio();
            break;
        case MODE_MUTE:
            showBoolParam(aproc.mute, txtLabels[LABEL_MUTE], txtLabels);
            break;
        case MODE_TIME:
        case MODE_TIME_EDIT:
            showTime(txtLabels);
            break;
        case MODE_BR:
            showBrWork(txtLabels);
            break;
        default:
            showSndParam(dispMode, txtLabels);
            break;
        }

        // Save current mode
        dispModePrev = dispMode;
        // Save current RC5 raw buf
        rc5BufPrev = rc5Buf;
    }

    return 0;
}
