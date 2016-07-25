#ifndef ACTIONS_H
#define ACTIONS_H

#include "inttypes.h"

#include "audio/audio.h"
#include "input.h"

/* Timers for different screens */
#define DISPLAY_TIME_TEST					20000U
#define DISPLAY_TIME_TEMP					20000U
#define DISPLAY_TIME_GAIN					3000U
#define DISPLAY_TIME_GAIN_START				1500U
#define DISPLAY_TIME_TIME					3000U
#define DISPLAY_TIME_TIME_EDIT				10000U
#define DISPLAY_TIME_TIMER					5000U
#define DISPLAY_TIME_TIMER_EXP				64000U
#define DISPLAY_TIME_ALARM					3000U
#define DISPLAY_TIME_ALARM_EDIT				10000U
#define DISPLAY_TIME_FM_RADIO				5000U
#define DISPLAY_TIME_FM_TUNE				10000U
#define DISPLAY_TIME_CHAN					2000U
#define DISPLAY_TIME_AUDIO					3000U
#define DISPLAY_TIME_SP						3000U
#define DISPLAY_TIME_BR						3000U

enum {
	ACTION_NEXT_RC_CMD = CMD_RC_END,	/* First non-RC command */
	ACTION_ZERO_DISPLAYTIME,
	ACTION_EXIT_STANDBY,
	ACTION_INIT_HARDWARE,
	ACTION_TESTMODE,
	ACTION_TEMPMODE,

	ACTION_NOACTION
};

/* Display modes */
enum {
	MODE_SPECTRUM = MODE_SND_END,
	MODE_STANDBY,
	MODE_FM_RADIO,
	MODE_FM_TUNE,

	MODE_TIME,
	MODE_TIME_EDIT,
	MODE_TIMER,
	MODE_SILENCE_TIMER,
	MODE_ALARM,
	MODE_ALARM_EDIT,
	MODE_MUTE,
	MODE_LOUDNESS,
	MODE_SURROUND,
	MODE_EFFECT_3D,
	MODE_TONE_DEFEAT,
	MODE_TEST,

	MODE_BR,
	MODE_TEMP,
	
	MODE_END
};

uint8_t getAction(void);

void handleAction(uint8_t action);
void handleEncoder(int8_t encCnt);
void handleChangeFM(uint8_t step);

uint8_t checkAlarmAndTime(void);

void handleExitDefaultMode(void);
void handleTimers(void);
void handleModeChange(void);

void showScreen(void);

#endif /* ACTIONS_H */
