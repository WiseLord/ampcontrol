#ifndef TEMP_H
#define TEMP_H

#include "ds18x20.h"

#include <inttypes.h>

#define MAX_TEMP 120
#define MIN_TEMP 20

void loadTempParams();
void saveTempParams();

void tempInit();
void tempControlProcess();

void setTempTH(int8_t temp);
int8_t getTempTH();
void changeTempTH(int8_t diff);

#endif // TEMP_H
