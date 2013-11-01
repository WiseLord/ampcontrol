#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>

#define BTN_DDR		DDRD
#define BTN_PORT	PORTD
#define BTN_PIN		PIND

#define BTN_MENU	(1<<PD7)
#define BTN_UP		(1<<PD4)
#define BTN_DOWN	(1<<PD5)
#define BTN_LEFT	(1<<PD6)
#define BTN_RIGHT	(1<<PD0)

#define ENC_A (1<<PD2)
#define ENC_B (1<<PD1)
#define ENC_AB (ENC_A | ENC_B)
#define ENC_0 0

void btnInit(void);
int8_t getEncValue(void);

#endif // INPUT_H
