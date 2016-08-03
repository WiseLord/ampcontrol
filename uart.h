#ifndef UART_H
#define UART_H

#include "inttypes.h"

#define USART_BAUDRATE 9600UL
#define BAUD_PRESCALE ((F_CPU/(USART_BAUDRATE * 8)) - 1)

enum {
	UART_CMD_RC,				// Remote control command

	UART_CMD_NO = 0x0F			// No command
};

typedef struct {
	uint8_t type;
	uint8_t command;
} UARTData;

void uartInit(void);
void uartWriteString(char *string);

UARTData getUartData(void);

#endif // UART_H
