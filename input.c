#include "input.h"

#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "pins.h"
#include "display/gdfb.h"

static volatile int8_t encCnt;
static volatile cmdID cmdBuf;
static int8_t encRes = 0;
static uint8_t silenceTime;

// Previous state
static volatile uint8_t encPrev = ENC_NO;
static volatile uint8_t btnPrev = BTN_NO;

static volatile uint16_t displayTime;

#ifdef _TEMPCONTROL
static volatile uint16_t sensTimer;                 // Timer of temperature measuring process
#endif
static volatile int16_t stbyTimer = STBY_TIMER_OFF; // Standby timer
static volatile int16_t initTimer = INIT_TIMER_OFF; // Init timer
static volatile uint16_t secTimer;                  // 1 second timer
static volatile uint8_t clockTimer;
static volatile int16_t silenceTimer;               // Timer to check silence
static volatile uint16_t rcTimer;

static uint8_t rcType;
static uint8_t rcAddr;
static uint8_t rcCode[CMD_RC_END];                  // Array with rc commands

void rcCodesInit(void)
{
    rcType = eeprom_read_byte((uint8_t *)EEPROM_RC_TYPE);
    rcAddr = eeprom_read_byte((uint8_t *)EEPROM_RC_ADDR);
    eeprom_read_block(rcCode, (uint8_t *)EEPROM_RC_CMD, CMD_RC_END);

    return;
}

void inputInit(void)
{
#ifdef _atmega32
    // Setup buttons and encoder as inputs with pull-up resistors
    IN(BUTTON_1);
    IN(BUTTON_2);
    IN(BUTTON_3);
    IN(BUTTON_4);
    IN(BUTTON_5);

    IN(ENCODER_A);
    IN(ENCODER_B);

    SET(BUTTON_1);
    SET(BUTTON_2);
    SET(BUTTON_3);
    SET(BUTTON_4);
    SET(BUTTON_5);

    SET(ENCODER_A);
    SET(ENCODER_B);
#endif

    // Set timer prescaller to 128 (125 kHz) and reset on match
#ifdef _atmega32
    TCCR2 = ((1 << CS22) | (0 << CS21) | (1 << CS20) | (1 << WGM21));
    OCR2 = 125;                                     // 125000/125 => 1000 polls/sec
    TCNT2 = 0;                                      // Reset timer value
    TIMSK |= (1 << OCIE2);                          // Enable timer compare match interrupt
#else
    TCCR2B = (1 << CS22) | (0 << CS21) | (1 << CS20);
    TCCR2A = (1 << WGM21);
    OCR2A = 125;                                    // 125000/125 => 1000 polls/sec
    TIMSK2 |= (1 << OCIE2A);                        // Enable timer compare match interrupt
#endif

    rcCodesInit();

    encRes = eeprom_read_byte((uint8_t *)EEPROM_ENC_RES);
    silenceTime = eeprom_read_byte((uint8_t *)EEPROM_SILENCE_TIMER);

    encCnt = 0;
    cmdBuf = CMD_RC_END;
#ifdef _TEMPCONTROL
    sensTimer = 0;
#endif

    return;
}

static uint8_t rcCmdIndex(uint8_t cmd)
{
    uint8_t i;

    for (i = 0; i < CMD_RC_END; i++)
        if (cmd == rcCode[i])
            return i;

    return CMD_RC_END;
}

static uint8_t getPins()
{
#ifdef _atmega32
    uint8_t pins = BTN_NO;

    if (!READ(BUTTON_1))
        pins |= BTN_D0;
    if (!READ(BUTTON_2))
        pins |= BTN_D1;
    if (!READ(BUTTON_3))
        pins |= BTN_D2;
    if (!READ(BUTTON_4))
        pins |= BTN_D3;
    if (!READ(BUTTON_5))
        pins |= BTN_D4;

    if (!READ(ENCODER_A))
        pins |= ENC_A;
    if (!READ(ENCODER_B))
        pins |= ENC_B;

    return pins;
#else
    return gdGetPins();
#endif
}

#ifdef _atmega32
ISR (TIMER2_COMP_vect)
#else
ISR (TIMER2_COMPA_vect)
#endif
{
    static int16_t btnCnt = 0;                      // Buttons press duration value

    // Current state
    uint8_t btnNow = getPins();
    uint8_t encNow = btnNow;

    // If encoder event has happened, inc/dec encoder counter
    if (encRes) {
        encNow = btnNow & ENC_AB;
        btnNow &= ~ENC_AB;

        if ((encPrev == ENC_NO && encNow == ENC_A) ||
                (encPrev == ENC_A && encNow == ENC_AB) ||
                (encPrev == ENC_AB && encNow == ENC_B) ||
                (encPrev == ENC_B && encNow == ENC_NO))
            encCnt++;
        if ((encPrev == ENC_NO && encNow == ENC_B) ||
                (encPrev == ENC_B && encNow == ENC_AB) ||
                (encPrev == ENC_AB && encNow == ENC_A) ||
                (encPrev == ENC_A && encNow == ENC_NO))
            encCnt--;
        encPrev = encNow;
    }

    // If button event has happened, place it to command buffer
    if (btnNow) {
        if (btnNow == btnPrev) {
            btnCnt++;
            if (btnCnt == LONG_PRESS) {
                switch (btnPrev) {
                case BTN_D0:
                    cmdBuf = CMD_BTN_1_LONG;
                    break;
                case BTN_D1:
                    cmdBuf = CMD_BTN_2_LONG;
                    break;
                case BTN_D2:
                    cmdBuf = CMD_BTN_3_LONG;
                    break;
                case BTN_D3:
                    cmdBuf = CMD_BTN_4_LONG;
                    break;
                case BTN_D4:
                    cmdBuf = CMD_BTN_5_LONG;
                    break;
                case BTN_D0 | BTN_D1:
                    cmdBuf = CMD_BTN_12_LONG;
                    break;
#ifdef _TEMPCONTROL
                case BTN_D0 | BTN_D2:
                    cmdBuf = CMD_BTN_13_LONG;
                    break;
#endif
                }
            } else if (!encRes) {
                if (btnCnt == LONG_PRESS + AUTOREPEAT) {
                    switch (btnPrev) {
                    case ENC_A:
                        encCnt++;
                        break;
                    case ENC_B:
                        encCnt--;
                        break;
                    }
                    btnCnt = LONG_PRESS + 1;
                }
            }
        } else {
            btnPrev = btnNow;
        }
    } else {
        if ((btnCnt > SHORT_PRESS) && (btnCnt < LONG_PRESS)) {
            switch (btnPrev) {
            case BTN_D0:
                cmdBuf = CMD_BTN_1;
                break;
            case BTN_D1:
                cmdBuf = CMD_BTN_2;
                break;
            case BTN_D2:
                cmdBuf = CMD_BTN_3;
                break;
            case BTN_D3:
                cmdBuf = CMD_BTN_4;
                break;
            case BTN_D4:
                cmdBuf = CMD_BTN_5;
                break;
            }
            if (!encRes) {
                switch (btnPrev) {
                case ENC_A:
                    encCnt++;
                    break;
                case ENC_B:
                    encCnt--;
                    break;
                }
            }
        }
        btnCnt = 0;
    }
    btnPrev = btnNow;

    // Timer of current display mode
    if (displayTime)
        displayTime--;

    // Time from last IR command
    if (rcTimer < 1000)
        rcTimer++;


    if (secTimer) {
        secTimer--;
    } else {
        secTimer = 1000;
        // Timer of standby mode
        if (stbyTimer > 0)
            stbyTimer--;
        // Silence timer
        if (silenceTimer > 0)
            silenceTimer--;
#ifdef _TEMPCONTROL
        // Timer of temperature measurement
        if (sensTimer)
            sensTimer--;
#endif
    }

    // Timer clock update
    if (clockTimer)
        clockTimer--;

    // Init timer
    if (initTimer > 0)
        initTimer--;

    return;
}


int8_t getEncoder(void)
{
    int8_t ret = 0;

    if (encRes) {
        if (encRes > 0) {
            while (encCnt >= encRes) {
                ret++;
                encCnt -= encRes;
            }
            while (encCnt <= -encRes) {
                ret--;
                encCnt += encRes;
            }
        } else {
            while (encCnt <= encRes) {
                ret++;
                encCnt -= encRes;
            }
            while (encCnt >= -encRes) {
                ret--;
                encCnt += encRes;
            }
        }
    } else {
        ret = encCnt;
        encCnt = 0;
    }

    return ret;
}

cmdID getBtnCmd(void)
{
    cmdID ret = cmdBuf;
    cmdBuf = CMD_RC_END;
    return ret;
}

cmdID getRcCmd(void)
{
    // Place RC event to command buffer if enough RC timer ticks
    IRData ir = takeIrData();

    uint8_t rcCmdBuf = CMD_RC_END;

    if (ir.ready && (ir.type == rcType) && (ir.address == rcAddr)) {
        if (!ir.repeat || (rcTimer > 800)) {
            rcTimer = 0;
            rcCmdBuf = rcCmdIndex(ir.command);
        }
        if (ir.command == rcCode[CMD_RC_VOL_UP] || ir.command == rcCode[CMD_RC_VOL_DOWN]) {
            if (rcTimer > 400) {
                rcTimer = 360;
                rcCmdBuf = rcCmdIndex(ir.command);
            }
        }
    }

    return rcCmdBuf;
}

uint16_t getBtnBuf(void)
{
    return btnPrev;
}

uint16_t getEncBuf(void)
{
    return encPrev;
}

void setDisplayTime(uint16_t value)
{
    displayTime = value;

    return;
}

uint16_t getDisplayTime(void)
{
    return displayTime;
}

#ifdef _TEMPCONTROL
uint8_t getSensTimer(void)
{
    return sensTimer;
}

void setSensTimer(uint8_t val)
{
    sensTimer = val;

    return;
}
#endif

int16_t getStbyTimer(void)
{
    return stbyTimer;
}

void setStbyTimer(int16_t val)
{
    stbyTimer = val;

    return;
}

void setSecTimer(uint16_t val)
{
    secTimer = val;

    return;
}

int16_t getSecTimer(void)
{
    return secTimer;
}

void setClockTimer(uint8_t value)
{
    clockTimer = value;

    return;
}

uint8_t getClockTimer(void)
{
    return clockTimer;
}

void enableSilenceTimer(void)
{
    if (silenceTime)
        silenceTimer = 60 * silenceTime;
    else
        silenceTimer = STBY_TIMER_OFF;

    return;
}

int16_t getSilenceTimer(void)
{
    return silenceTimer;
}

void disableSilenceTimer(void)
{
    silenceTimer = STBY_TIMER_OFF;

    return;
}

void setInitTimer(int16_t value)
{
    initTimer = value;

    return;
}

int16_t getInitTimer(void)
{
    return initTimer;
}
