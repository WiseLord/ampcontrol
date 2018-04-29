#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "adc.h"
#include "input.h"
#include "remote.h"
#include "i2c.h"
#include "display.h"
#include "tuner/tuner.h"
#include "actions.h"

#include "rtc.h"

// Hardware initialization
static void hwInit()
{
#ifdef _HARDWARE_RST
#ifdef _SI470X
    si470xReset();
#endif
#endif
    I2CInit();                              // I2C bus
    displayInit();                          // Load params and text labels before fb scan started
    rcInit();                               // IR Remote control
    inputInit();                            // Buttons/encoder polling
    adcInit();                              // Analog-to-digital converter

    rtc.etm = RTC_NOEDIT;

    sei();                                  // Gloabl interrupt enable

    tunerInit();                            // Tuner

    OUT(STMU_STBY);                         // Standby port
    sndInit();                              // Load labels/icons/etc
}

int main()
{
    int8_t encCnt = 0;
    uint8_t action = ACTION_NOACTION;

    // Init hardware
    hwInit();

    while (1) {
        // Init hardware if init timer expired
        if (getInitTimer() == 0)
            action = ACTION_INIT_HARDWARE;

        // Check alarm and update time
        if (action == ACTION_NOACTION)
            action = checkAlarmAndTime();

        // Convert input command to action
        if (action == ACTION_NOACTION)
            action = getAction();

        // Handle action
        handleAction(action);

        // Handle encoder
        encCnt = getEncoder();              // Get value from encoder
        if (action == CMD_RC_VOL_UP)        // Emulate VOLUME_UP action as encoder action
            encCnt++;
        if (action == CMD_RC_VOL_DOWN)      // Emulate VOLUME_DOWN action as encoder action
            encCnt--;
        handleEncoder(encCnt);

        // Reset handled action
        action = ACTION_NOACTION;

        // Check if we need exit to default mode
        handleExitDefaultMode();

        // Clear screen if mode has changed
        handleModeChange();

        // Show things
        showScreen();
    }

    return 0;
}
