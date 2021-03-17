#include "input.h"

#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "remote.h"
#include "eeprom.h"
#include "pins.h"

static int8_t encCnt = 0;
static CmdID cmdBuf = CMD_END;

// Previous state
static uint8_t encPrev = ENC_NO;
static uint8_t btnPrev = BTN_NO;

static uint16_t displayTime = 0;                    // Display mode timer
static int16_t initTimer = INIT_TIMER_OFF;          // Init timer
static uint8_t clockTimer = 0;                      // RTC poll timer

static uint16_t rcTimer;

static uint8_t rcType;
static uint8_t rcAddr;
static uint8_t rcCode[CMD_RC_END];                  // Array with rc commands

static CmdID rcCmdIndex(uint8_t rcCmd)
{
    CmdID i;

    for (i = 0; i < CMD_RC_END; i++)
        if (rcCmd == rcCode[i])
            return i;

    return CMD_RC_END;
}

static uint8_t getPins()
{
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
}

void rcCodesInit()
{
    rcType = eeprom_read_byte((uint8_t *)EEPROM_RC_TYPE);
    rcAddr = eeprom_read_byte((uint8_t *)EEPROM_RC_ADDR);
    eeprom_read_block(rcCode, (uint8_t *)EEPROM_RC_CMD, CMD_RC_END);
}

void inputInit()
{
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

#if defined(_atmega8) || defined(_atmega16) || defined(_atmega32)
#if F_CPU == 8000000L
    // Set timer prescaller to 64 (125 kHz) and reset on match
    TCCR2 = (1 << CS22) | (0 << CS21) | (0 << CS20) | (1 << WGM21);
#else
    // Set timer prescaller to 128 (125 kHz) and reset on match
    TCCR2 = ((1 << CS22) | (0 << CS21) | (1 << CS20) | (1 << WGM21));
#endif
    OCR2 = 125;                                     // 125000/125 => 1000 polls/sec
    TCNT2 = 0;                                      // Reset timer value
    TIMSK |= (1 << OCIE2);                           // Enable timer compare match interrupt
#else
    TCCR2A = (1 << WGM21);
    TCCR2B = (1 << CS22) | (0 << CS21) | (0 << CS20);
    OCR2A = 125;                                    // 125000/125 => 1000 polls/sec
    TCNT2 = 0;                                      // Reset timer value
    TIMSK2 |= (1 << OCIE2A);                         // Enable timer compare match interrupt
#endif
    rcCodesInit();
}

#if defined(_atmega8) || defined(_atmega16) || defined(_atmega32)
ISR (TIMER2_COMP_vect)
#else
ISR (TIMER2_COMPA_vect)
#endif
{
    static int16_t btnCnt = 0;                      // Buttons press duration value

    // Current state
    uint8_t btnNow = getPins();
    uint8_t encNow = btnNow & ENC_AB;
    btnNow &= ~ENC_AB;

    // If encoder event has happened, inc/dec encoder counter
    if (encNow == ENC_AB) {
        if (encPrev == ENC_A)
            encCnt++;
        if (encPrev == ENC_B)
            encCnt--;
    }
    encPrev = encNow;                               // Save current encoder state

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
                }
            }
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
        }
        btnCnt = 0;
    }
    btnPrev = btnNow;

    // Timer of current display mode
    if (displayTime)
        displayTime--;

    // Time from last IR command
    if (rcTimer < RC_PRESS_LIMIT)
        rcTimer++;

    // Timer clock update
    if (clockTimer)
        clockTimer--;

    // Init timer
    if (initTimer > 0)
        initTimer--;
}

int8_t getEncoder()
{
    int8_t ret = encCnt;
    encCnt = 0;
    return ret;
}

CmdID getBtnCmd()
{
    CmdID ret = cmdBuf;
    cmdBuf = CMD_RC_END;
    return ret;
}

CmdID getRcCmd()
{
    // Place RC event to command buffer if enough RC timer ticks
    IRData ir = takeIrData();

    CmdID rcCmdBuf = CMD_RC_END;

    if (ir.ready && (ir.address == rcAddr)) {
        if (!ir.repeat || (rcTimer > RC_LONG_PRESS)) {
            rcTimer = 0;
            rcCmdBuf = rcCmdIndex(ir.command);
        }
        if (ir.command == rcCode[CMD_RC_VOL_UP] || ir.command == rcCode[CMD_RC_VOL_DOWN]) {
            if (rcTimer > RC_VOL_REPEAT) {
                rcTimer = RC_VOL_DELAY;
                rcCmdBuf = rcCmdIndex(ir.command);
            }
        }
    }

    return rcCmdBuf;
}

uint16_t getBtnBuf()
{
    return btnPrev;
}

uint16_t getEncBuf()
{
    return encPrev;
}

void setDisplayTime(uint16_t value)
{
    displayTime = value;
}

uint16_t getDisplayTime()
{
    return displayTime;
}

void setClockTimer(uint8_t value)
{
    clockTimer = value;
}

uint8_t getClockTimer()
{
    return clockTimer;
}

void setInitTimer(int16_t value)
{
    initTimer = value;
}

int16_t getInitTimer()
{
    return initTimer;
}
