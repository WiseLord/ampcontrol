#ifndef ACTIONS_H
#define ACTIONS_H

#include "inttypes.h"

#include "audio/audioproc.h"

void powerOn(void);
void powerOff(void);
void handleSetInput(uint8_t *dispMode);
void handleEditTime(uint8_t *dispMode);
void handleSwitchMute(uint8_t *dispMode);
void handleSetDefDisplay(uint8_t *dispMode);
void handleSwitchSpMode(uint8_t *dispMode);
void handleSwitchFmMode(uint8_t *dispMode);
void handleChangeTimer(uint8_t *dispMode, int16_t stbyTimer);
void handleStoreStation(uint8_t *dispMode);
void handleChangeFM(uint8_t *dispMode, uint8_t step);
void handleEditAlarm(uint8_t *dispMode);
void checkAlarm(uint8_t *dispMode);

#endif /* ACTIONS_H */
