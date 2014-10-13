#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)			x ## y

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)

/* Standby/Mute port definitions */
#define STMU_MUTE			C
#define STMU_MUTE_LINE		(1<<5)

#endif /* PINS_H */
