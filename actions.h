#ifndef ACTIONS_H
#define ACTIONS_H

#include "inttypes.h"

#include "audio/audio.h"

void handleSwitchPower(uint8_t *dispMode);
void handleNextInput(uint8_t *dispMode);
void handleEditTime(uint8_t *dispMode);
void handleSwitchMute(uint8_t *dispMode);
void handleNextSndParam(uint8_t *dispMode, sndParam **curSndParam);
void handleSetDefDisplay(uint8_t *dispMode);
void handleSwitchSpMode(uint8_t *dispMode);
void handleSwitchFmMode(uint8_t *dispMode);


#endif /* ACTIONS_H */
