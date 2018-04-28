#include "input.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "remote.h"
#include "eeprom.h"

static volatile int8_t encCnt;
static volatile uint8_t cmdBuf;
static volatile uint16_t rc5SaveBuf;

static volatile uint16_t displayTime;

static volatile uint16_t rcTimer;

//static uint8_t rcType;
static uint8_t rcAddr;
static uint8_t rcCode[CMD_RC_END];       // Array with rc5 commands

void inputInit()
{
    // Setup buttons and encoders as inputs with pull-up resistors
    INPUT_DDR &= ~(BTN_MASK | ENC_AB);
    INPUT_PORT |= (BTN_MASK | ENC_AB);

    // Set timer prescaller to 128 (125 kHz) and reset on match
    TCCR2 = ((1 << CS22) | (0 << CS21) | (1 << CS20) | (1 << WGM21));
    OCR2 = 125;                     // 125000/125 => 1000 polls/sec
    TCNT2 = 0;                      // Reset timer value
    TIMSK |= (1 << OCIE2);          // Enable timer compare match interrupt

    // Load RC5 device address and commands from eeprom
    rcAddr = eeprom_read_byte((uint8_t *)EEPROM_RC_ADDR);
    eeprom_read_block(rcCode, (uint8_t *)EEPROM_RC_CMD, CMD_RC_END);

    encCnt = 0;
    cmdBuf = CMD_END;
}

static CmdID rcCmdIndex(uint8_t rcCmd)
{
    CmdID i;

    for (i = 0; i < CMD_RC_END; i++)
        if (rcCmd == rcCode[i])
            return i;

    return CMD_RC_END;
}

ISR (TIMER2_COMP_vect)
{
    static int16_t btnCnt = 0;      // Buttons press duration value
    static uint16_t rc5Timer;

    // Previous state
    static uint8_t encPrev = ENC_0;
    static uint8_t btnPrev = 0;
    // Current state
    uint8_t encNow = ~INPUT_PIN & ENC_AB;
    uint8_t btnNow = ~INPUT_PIN & BTN_MASK;

    // If encoder event has happened, inc/dec encoder counter
    switch (encNow) {
    case ENC_AB:
        if (encPrev == ENC_B)
            encCnt++;
        if (encPrev == ENC_A)
            encCnt--;
        break;
//    case ENC_A:
//        if (encPrev == ENC_AB)
//            encCnt++;
//        if (encPrev == ENC_0)
//            encCnt--;
//        break;
//    case ENC_B:
//        if (encPrev == ENC_0)
//            encCnt++;
//        if (encPrev == ENC_AB)
//            encCnt++;
//        break;
//    case ENC_0:
//        if (encPrev == ENC_A)
//            encCnt++;
//        if (encPrev == ENC_B)
//            encCnt++;
//        break;
    }
    encPrev = encNow;               // Save current encoder state

    // If button event has happened, place it to command buffer
    if (btnNow) {
        if (btnNow == btnPrev) {
            btnCnt++;
            if (btnCnt == LONG_PRESS) {
                switch (btnPrev) {
                case BTN_1:
                    cmdBuf = CMD_BTN_1_LONG;
                    break;
                case BTN_2:
                    cmdBuf = CMD_BTN_2_LONG;
                    break;
                case BTN_3:
                    cmdBuf = CMD_BTN_3_LONG;
                    break;
                case BTN_4:
                    cmdBuf = CMD_BTN_4_LONG;
                    break;
                case BTN_5:
                    cmdBuf = CMD_BTN_5_LONG;
                    break;
                case BTN_TEST_INPUT:
                    cmdBuf = CMD_BTN_TESTMODE;
                    break;
                }
            }
        } else {
            btnPrev = btnNow;
        }
    } else {
        if ((btnCnt > SHORT_PRESS) && (btnCnt < LONG_PRESS)) {
            switch (btnPrev) {
            case BTN_1:
                cmdBuf = CMD_BTN_1;
                break;
            case BTN_2:
                cmdBuf = CMD_BTN_2;
                break;
            case BTN_3:
                cmdBuf = CMD_BTN_3;
                break;
            case BTN_4:
                cmdBuf = CMD_BTN_4;
                break;
            case BTN_5:
                cmdBuf = CMD_BTN_5;
                break;
            }
        }
        btnCnt = 0;
    }

    // Timer of current display mode
    if (displayTime)
        displayTime--;

    // Time from last IR command
    if (rcTimer < RC_PRESS_LIMIT)
        rcTimer++;

    // Time from last IR command
    if (rc5Timer < 1000)
        rc5Timer++;

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

void setDisplayTime(uint8_t value)
{
    displayTime = value;
    displayTime <<= 10;
}

uint8_t getDisplayTime(void)
{
    return (displayTime | 0x3F) >> 10;
}
