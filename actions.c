#include "actions.h"

#include "display.h"
#include "tuner/tuner.h"
#include "pins.h"
#include "rtc.h"
#include "eeprom.h"

static uint8_t dispMode = MODE_STANDBY;
static uint8_t dispModePrev = MODE_STANDBY;
static uint8_t fmMode = MODE_FM_RADIO;
static uint8_t defDisp = MODE_SPECTRUM;

static uint8_t defDispMode()
{
    uint8_t ret;

    if (defDisp == MODE_FM_RADIO) {
        if (aproc.input || !tuner.ic)
            ret = MODE_SPECTRUM;
        else
            ret = MODE_FM_RADIO;
    } else {
        ret = defDisp;
    }
    if (aproc.mute)
        ret = MODE_MUTE;

    return ret;
}

uint8_t getAction()
{
    uint8_t action = ACTION_NOACTION;
    CmdID cmd;

    // Get command
    cmd = getBtnCmd();

    // Handle commands from remote control
    if (cmd == CMD_RC_END)
        cmd = getRcCmd();

    // Handle commands from remote control
    if (cmd < CMD_RC_END)
        action = cmd;

    // Handle commands from buttons
    switch (cmd) {
    case CMD_BTN_1:
        action = CMD_RC_STBY;
        break;
    case CMD_BTN_2:
        action = CMD_RC_IN_NEXT;
        break;
    case CMD_BTN_3:
        if (dispMode == MODE_FM_RADIO)
            action = CMD_RC_FM_DEC;
        else if (dispMode == MODE_TIME || dispMode == MODE_TIME_EDIT)
            action = CMD_RC_TIME;
        break;
    case CMD_BTN_4:
        if (dispMode == MODE_FM_RADIO)
            action = CMD_RC_FM_INC;
        else
            action = CMD_RC_MUTE;
        break;
    case CMD_BTN_5:
        if (dispMode == MODE_FM_RADIO && fmMode == MODE_RADIO_TUNE) {
            action = CMD_RC_FM_STORE;
        } else {
            action = CMD_RC_NEXT_SNDPAR;
        }
        break;
    case CMD_BTN_1_LONG:
        action = CMD_RC_BRIGHTNESS;
        break;
    case CMD_BTN_2_LONG:
        action = CMD_RC_DEF_DISPLAY;
        break;
    case CMD_BTN_3_LONG:
        action = CMD_RC_TIME;
        break;
    case CMD_BTN_4_LONG:
        action = CMD_RC_FM_MODE;
        break;
    case CMD_BTN_12_LONG:
        action = ACTION_TESTMODE;
        break;
    default:
        break;
    }

    // Remap GO_STANDBY command to EXIT_STANDBY if in standby mode
    if (action == CMD_RC_STBY && dispMode == MODE_STANDBY)
        action = ACTION_EXIT_STANDBY;

    // Remap NEXT/PREV_INPUT actions to INPUT_X
    if (action == CMD_RC_IN_NEXT) {
        action = CMD_RC_IN_0 + aproc.input;
        if (dispMode >= MODE_SND_GAIN0 && dispMode < MODE_SND_END) {
            action += 1;
            if (action >= CMD_RC_IN_0 + aproc.inCnt)
                action = CMD_RC_IN_0;
        }
    }

    // Disable actions except ZERO_DISPLAY_TIME in temp mode
    if (dispMode == MODE_TEST) {
        if (action != ACTION_NOACTION)
            setDisplayTime(DISPLAY_TIME_TEST);
        action = ACTION_NOACTION;
    }
    // Disable actions except POWERON and TESTMODE in standby mode
    if (dispMode == MODE_STANDBY) {
        if (action != ACTION_EXIT_STANDBY && action != ACTION_TESTMODE)
            action = ACTION_NOACTION;
    }
    // Disable most actions in FM edit mode
    if (dispMode == MODE_FM_RADIO && fmMode == MODE_RADIO_TUNE) {
        if (action != CMD_RC_STBY &&
                action != CMD_RC_VOL_DOWN && action != CMD_RC_VOL_UP &&
                action != CMD_RC_FM_MODE && action != CMD_RC_FM_STORE &&
                action != CMD_RC_FM_DEC && action != CMD_RC_FM_INC
           )
           action = ACTION_NOACTION;
    }

    return action;
}

void handleAction(uint8_t action)
{
    switch (action) {
    case ACTION_EXIT_STANDBY:
        SET(STMU_STBY); // Power up audio and tuner
        setWorkBrightness();
        setInitTimer(INIT_TIMER_START);
        dispMode = MODE_SND_GAIN0 + aproc.input;
        setDisplayTime(DISPLAY_TIME_GAIN_START);
        break;
    case ACTION_INIT_HARDWARE:
        tunerPowerOn();
        sndPowerOn();

        tunerSetMute(aproc.input);
        tunerSetFreq();

        setInitTimer(INIT_TIMER_OFF);
        break;
    case CMD_RC_STBY:
        sndSetMute(1);
        sndPowerOff();
        tunerPowerOff();
        displayPowerOff();

        CLR(STMU_STBY);

        setStbyBrightness();
        rtc.etm = RTC_NOEDIT;
        setInitTimer(INIT_TIMER_OFF);
        dispMode = MODE_STANDBY;
        break;
    case CMD_RC_TIME:
        if ((dispMode == MODE_TIME || dispMode == MODE_TIME_EDIT) && rtc.etm != RTC_YEAR) {
            rtcNextEditParam();
            dispMode = MODE_TIME_EDIT;
            setDisplayTime(DISPLAY_TIME_TIME_EDIT);
        } else {
            rtc.etm = RTC_NOEDIT;
            dispMode = MODE_TIME;
            setDisplayTime(DISPLAY_TIME_TIME);
        }
        break;
    case CMD_RC_MUTE:
        displayClear();
        sndSetMute(!aproc.mute);
        tunerSetMute(aproc.mute || aproc.input);
        dispMode = MODE_MUTE;
        setDisplayTime(DISPLAY_TIME_AUDIO);
        break;
    case CMD_RC_NEXT_SNDPAR:
        sndNextParam(&dispMode);
        setDisplayTime(DISPLAY_TIME_AUDIO);
        break;
    case CMD_RC_BRIGHTNESS:
        dispMode = MODE_BR;
        setDisplayTime(DISPLAY_TIME_BR);
        break;
    case CMD_RC_DEF_DISPLAY:
        defDisp = MODE_SPECTRUM;
        if (aproc.input == 0) {
            if (dispMode == MODE_SPECTRUM) {
                fmMode = MODE_RADIO_CHAN;
                defDisp = MODE_FM_RADIO;
            }
        }
        dispMode = defDisp;
        break;
    case ACTION_TESTMODE:
        dispMode = MODE_TEST;
        setDisplayTime(DISPLAY_TIME_TEST);
        break;
    case CMD_RC_LOUDNESS:
    case CMD_RC_SURROUND:
    case CMD_RC_EFFECT_3D:
    case CMD_RC_TONE_BYPASS:
        sndSwitchExtra(1 << (action - CMD_RC_LOUDNESS));
        dispMode = MODE_LOUDNESS + (action - CMD_RC_LOUDNESS);
        setDisplayTime(DISPLAY_TIME_AUDIO);
        break;
    case CMD_RC_IN_0:
    case CMD_RC_IN_1:
    case CMD_RC_IN_2:
    case CMD_RC_IN_3:
    case CMD_RC_IN_4:
        sndSetInput(action - CMD_RC_IN_0);
        dispMode = MODE_SND_GAIN0 + aproc.input;
        setDisplayTime(DISPLAY_TIME_GAIN);
        tunerSetMute(aproc.mute || aproc.input);
        break;
    default:
        if (!aproc.input && tuner.ic) {
            if (action >= CMD_RC_FM_RDS && action <= CMD_RC_FM_9) {
                if (dispMode != MODE_FM_RADIO)
                    fmMode = MODE_RADIO_CHAN;
                dispMode = MODE_FM_RADIO;
                setDisplayTime(DISPLAY_TIME_FM_RADIO);
            } else {
                break;
            }
            switch (action) {
            case CMD_RC_FM_MODE:
                fmMode = !fmMode;
                break;
            case CMD_RC_FM_INC:
                tunerNextStation(SEARCH_UP);
                break;
            case CMD_RC_FM_DEC:
                tunerNextStation(SEARCH_DOWN);
                break;
            case CMD_RC_FM_STORE:
                tunerStoreStation();
                break;
            case CMD_RC_FM_MONO:
                tunerSetMono(!tuner.mono);
                break;
#ifdef _RDS
            case CMD_RC_FM_RDS:
                tunerSetRDS(!tuner.rds);
#endif
            default:
                if (action == CMD_RC_FM_0) {
                    tunerLoadStation(9);
                } else {
                    tunerLoadStation(action - CMD_RC_FM_0 - 1);
                }
                break;
            }
        }
        break;
    }

}

void handleEncoder(int8_t encCnt)
{
    if (encCnt) {
        switch (dispMode) {
        case MODE_STANDBY:
            break;
        case MODE_TEST:
            setDisplayTime(DISPLAY_TIME_TEST);
            break;
        case MODE_TIME_EDIT:
            rtcChangeTime(encCnt);
            setDisplayTime(DISPLAY_TIME_TIME_EDIT);
            break;
        case MODE_BR:
            changeBrWork(encCnt);
            setDisplayTime(DISPLAY_TIME_BR);
            break;
        case MODE_FM_RADIO: {
            if (fmMode == MODE_RADIO_TUNE) {
                tunerChangeFreq(encCnt);
                setDisplayTime(DISPLAY_TIME_FM_TUNE);
                break;
            }
        }
        case MODE_SPECTRUM:
        case MODE_TIME:
        case MODE_MUTE:
        case MODE_LOUDNESS:
            dispMode = MODE_SND_VOLUME;
        default:
            sndSetMute(0);
            sndChangeParam(dispMode, encCnt);
            setDisplayTime(DISPLAY_TIME_GAIN);
            break;
        }
    }
}

uint8_t checkAlarmAndTime()
{
    // Poll RTC for time
    if (getClockTimer() == 0) {
        rtcReadTime();
        setClockTimer(RTC_POLL_TIME);            // Limit check interval
    }

    return ACTION_NOACTION;
}

void handleExitDefaultMode()
{
    // Exit to default mode
    if (getDisplayTime() == 0) {
        switch (dispMode) {
        case MODE_STANDBY:
            setStbyBrightness();
        case MODE_TEST:
            dispMode = MODE_STANDBY;
            break;
        default:
            dispMode = defDispMode();
            break;
        }
    }
}

void handleModeChange()
{
    if (dispMode != dispModePrev)
        displayClear();
}

void showScreen()
{
    ks0066SetXY(0, 0);
    switch (dispMode) {
    case MODE_STANDBY:
    case MODE_TIME:
    case MODE_TIME_EDIT:
        showTime();
        break;
    case MODE_TEST:
        setWorkBrightness();
        showRCInfo();
        break;
    case MODE_SPECTRUM:
        showSpectrum();
        break;
    case MODE_FM_RADIO:
        showRadio(fmMode);
        break;
    case MODE_MUTE:
        showBoolParam(aproc.mute, LABEL_MUTE);
        break;
    case MODE_LOUDNESS:
    case MODE_SURROUND:
    case MODE_EFFECT_3D:
    case MODE_TONE_DEFEAT:
        showBoolParam(aproc.extra & (1 << (dispMode - MODE_LOUDNESS)),
                      LABEL_LOUDNESS + (dispMode - MODE_LOUDNESS));
        break;
    case MODE_BR:
        showBrWork();
        break;
    default:
        showSndParam(dispMode);
        break;
    }

    // Save current mode
    dispModePrev = dispMode;
}
