#ifndef TDA7493_H
#define TDA7493_H

#include <inttypes.h>

typedef struct {
	int8_t value;
	int8_t min;
	int8_t max;
	uint8_t label[16];
	int8_t (*i2cValue)(int8_t value);
	int8_t (*dbValue)(int8_t value);
} regParam;

void changeParam(regParam *param, int8_t encVal);
void showParam(regParam *param);
regParam *nextParam(regParam * param);
void loadParams(void);
void saveParams(void);

void incParam(regParam *param);
void decParam(regParam *param);

#endif /* TDA7493_H */
