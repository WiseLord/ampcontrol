#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#include "ds1307.h"

/* Spectrum output mode */
#define SP_MODE_STEREO		0
#define SP_MODE_MIXED		1

#define LABELS_COUNT		22

enum {
	LABEL_VOLUME,
	LABEL_BASS,
	LABEL_MIDDLE, /* Also loudness */
	LABEL_TREBLE,
	LABEL_PREAMP,
	LABEL_BALANCE,
	LABEL_GAIN_0,
	LABEL_GAIN_1,
	LABEL_GAIN_2,
	LABEL_GAIN_3,
	LABEL_MUTE,
	LABEL_ON,
	LABEL_OFF,
	LABEL_DB,
	LABEL_MONDAY,
	LABEL_THUESDAY,
	LABEL_WEDNESDAY,
	LABEL_THURSDAY,
	LABEL_FRIDAY,
	LABEL_SADURDA,
	LABEL_SUNDAY
};

void showRC5Info(uint16_t rc5Buf);
void showRadio(uint8_t *buf);
void showParLabel(const uint8_t *parLabel, uint8_t **txtLabels);
void showBoolParam(uint8_t value, const uint8_t *parLabel, uint8_t **txtLabels);
void showBar(int8_t min, int8_t max, int8_t value);
void showParValue(int8_t value);
void drawTm(timeMode tm, const uint8_t *font);
void showTime(uint8_t **txtLabels);
void drawSpectrum(uint8_t *buf, uint8_t mode);

#endif /* DISPLAY_H */
