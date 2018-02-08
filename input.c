#include "input.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "remote.h"
#include "eeprom.h"

static volatile int8_t encCnt = 0;
static volatile uint8_t cmdBuf = CMD_RC_STBY;

// Previous state
static volatile uint8_t encPrev = ENC_NO;
static volatile uint8_t btnPrev = BTN_NO;

uint16_t dispTimer = 0;
uint16_t rtcTimer = 0;
int16_t initTimer = INIT_TIMER_OFF;

static volatile uint16_t rcTimer;

//static uint8_t rcType;
static uint8_t rcAddr;
static uint8_t rcCode[CMD_RC_END];  // Array with rc commands

void rcCodesInit(void)
{
//  rcType = eeprom_read_byte((uint8_t*)EEPROM_RC_TYPE);
    rcAddr = eeprom_read_byte((uint8_t *)EEPROM_RC_ADDR);
    eeprom_read_block(rcCode, (uint8_t *)EEPROM_RC_CMD, CMD_RC_END);

    return;
}

void inputInit()
{
    // Setup buttons and encoder as inputs with pull-up resistors
    DDR(BUTTON_1) &= ~BUTTON_1_LINE;
    DDR(BUTTON_2) &= ~BUTTON_2_LINE;
    DDR(BUTTON_3) &= ~BUTTON_3_LINE;
    DDR(BUTTON_4) &= ~BUTTON_4_LINE;
    DDR(BUTTON_5) &= ~BUTTON_5_LINE;

    DDR(ENCODER_A) &= ~ENCODER_A_LINE;
    DDR(ENCODER_B) &= ~ENCODER_B_LINE;

    PORT(BUTTON_1) |= BUTTON_1_LINE;
    PORT(BUTTON_2) |= BUTTON_2_LINE;
    PORT(BUTTON_3) |= BUTTON_3_LINE;
    PORT(BUTTON_4) |= BUTTON_4_LINE;
    PORT(BUTTON_5) |= BUTTON_5_LINE;

    PORT(ENCODER_A) |= ENCODER_A_LINE;
    PORT(ENCODER_B) |= ENCODER_B_LINE;

    // Set timer prescaller to 64 (125 kHz) and reset on match
    TCCR2 = ((1 << CS22) | (0 << CS21) | (0 << CS20) | (1 << WGM21));
    OCR2 = 125;                     // 125000/125 => 1000 polls/sec
    TCNT2 = 0;                      // Reset timer value

    rcCodesInit();
}

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

    if (~PIN(BUTTON_1) & BUTTON_1_LINE)
        pins |= BTN_D0;
    if (~PIN(BUTTON_2) & BUTTON_2_LINE)
        pins |= BTN_D1;
    if (~PIN(BUTTON_3) & BUTTON_3_LINE)
        pins |= BTN_D2;
    if (~PIN(BUTTON_4) & BUTTON_4_LINE)
        pins |= BTN_D3;
    if (~PIN(BUTTON_5) & BUTTON_5_LINE)
        pins |= BTN_D4;

    if (~PIN(ENCODER_A) & ENCODER_A_LINE)
        pins |= ENC_A;
    if (~PIN(ENCODER_B) & ENCODER_B_LINE)
        pins |= ENC_B;

    return pins;
}

ISR (TIMER2_COMP_vect)
{
    static int16_t btnCnt = 0;      // Buttons press duration value

    // Current state
    uint8_t btnNow = getPins();
    uint8_t encNow = btnNow & ENC_AB;
    btnNow &= ~ENC_AB;

    // If encoder event has happened, inc/dec encoder counter
    if (encNow == ENC_AB) {
        if (encPrev == ENC_B)
            encCnt++;
        if (encPrev == ENC_A)
            encCnt--;
    }
    encPrev = encNow;               // Save current encoder state

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
        }
        btnCnt = 0;
    }

    // Init timer
    if (initTimer > 0)
        initTimer--;

    // Time from last IR command
    if (rcTimer < RC_PRESS_LIMIT)
        rcTimer++;

    // Current display mode timer
    if (dispTimer)
        dispTimer--;

    // RTC poll timer
    if (rtcTimer)
        rtcTimer--;

    return;
};


int8_t getEncoder(void)
{
    int8_t ret = encCnt;
    encCnt = 0;
    return ret;
}

uint8_t getBtnCmd(void)
{
    uint8_t ret = cmdBuf;
    cmdBuf = CMD_RC_END;
    return ret;
}

uint8_t getRcCmd()
{

    // Place RC5 event to command buffer if enough RC5 timer ticks
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

void setDispTimer(uint8_t value)
{
    dispTimer = 1000U * value;

    return;
}
