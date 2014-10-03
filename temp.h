#ifndef TEMP_H
#define TEMP_H

#include "ds18x20.h"

#include <inttypes.h>
#include "pins.h"

#define MAX_TEMP 90
#define MIN_TEMP 30

void loadTempParams(void);
void saveTempParams(void);

void tempInit(void);
void tempControlProcess(void);

void setTempTH(int8_t temp);
int8_t getTempTH(void);
void changeTempTH(int8_t diff);

#endif // TEMP_H
