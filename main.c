#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "adc.h"
#include "input.h"
#include "remote.h"
#ifdef _UARTCONTROL
#include "uart.h"
#endif
#include "i2c.h"
#include "display.h"
#include "tuner/tuner.h"
#ifdef _TEMPCONTROL
#include "temp.h"
#endif
#include "actions.h"
#include "pins.h"
#ifdef _SPISW
#include "spisw.h"
#endif

// Hardware initialization
static void hwInit(uint8_t extFunc)
{
#ifdef _TEMPCONTROL
    loadTempParams();
#endif
    if (extFunc == USE_DS18B20) {
#ifdef _TEMPCONTROL
        ds18x20SearchDevices();
        tempInit();                         // Init temperature control
        setSensTimer(TEMP_MEASURE_TIME);
#endif
    } else {
#ifdef _SPISW
        SPIswInitLines(extFunc);
#endif
    }

    I2CInit();                              // I2C bus
    displayInit();                          // Load params and text labels before fb scan started
    rcInit();                               // IR Remote control
    inputInit();                            // Buttons/encoder polling
#ifdef _UARTCONTROL
    uartInit();                             // USART
#endif
    adcInit();                              // Analog-to-digital converter
    rtc.etm = RTC_NOEDIT;
    alarmInit();
    alarm0.eam = ALARM_NOEDIT;
    sei();                                  // Gloabl interrupt enable

    tunerInit();                            // Tuner

    OUT(STMU_STBY);                         // Standby port
    OUT(STMU_MUTE);                         // Mute port
    sndInit();                              // Load labels/icons/etc

    setStbyTimer(0);

    return;
}

int main(void)
{
    int8_t encCnt = 0;
    uint8_t action = ACTION_NOACTION;
    uint8_t extFunc = eeprom_read_byte((uint8_t *)EEPROM_EXT_FUNC);

    // Init hardware
    hwInit(extFunc);

    while (1) {
#ifdef _TEMPCONTROL
        // Control temperature
        if (extFunc == USE_DS18B20) {
            if (getSensTimer() == 0) {
                ds18x20Process();
                setSensTimer(SENSOR_POLL_INTERVAL);
            }
            tempControlProcess();
        }
#endif
        // Emulate poweroff if any of timers expired
        if (getStbyTimer() == 0 || getSilenceTimer() == 0)
            action = CMD_RC_STBY;

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

        // Reset silence timer on any action
        if (encCnt || (action != ACTION_NOACTION && action != ACTION_EXIT_STANDBY))
            enableSilenceTimer();

        // Reset handled action
        action = ACTION_NOACTION;

        // Check if we need exit to default mode
        handleExitDefaultMode();

        // Switch to timer mode if it expires
        handleTimers();

        // Clear screen if mode has changed
        handleModeChange();

        // Show things
        showScreen();
    }

    return 0;
}
