#include "uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "input.h"

static struct {
    uint8_t pos;
    uint8_t ready;
    char buf[6];
} uRaw;

void uartInit(void)
{
#ifdef _atmega32
    // Set UART prescaler (both UBRRH and UBRRL)
    UBRRH = 0;
    UBRRL = BAUD_PRESCALE;

    // Set double transmission speed
    UCSRA = (1 << U2X);

    // Enable RX complete interrupt and receiver but not transmitter
    UCSRB = (1 << RXCIE) | (1 << RXEN) | (0 << TXEN);

    // Set frame format (8 data bits, 1 stop bit)
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
#else
    // Set UART prescaler (both UBRR0H and UBRR0L)
    UBRR0 = BAUD_PRESCALE;

    // Set double transmission speed
    UCSR0A = (1 << U2X0);

    // Enable RX complete interrupt and both receiver and transmitter
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);

    // Set frame format (8 data bits, 1 stop bit)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
#endif
    uRaw.pos = 0;
    uRaw.ready = 0;
}

static void uartWriteByte(uint8_t data)
{
#ifdef _atmega32
    // Wait for empty transmit buffer
    while (!(UCSRA & (1 << UDRE)));

    // Put data into buffer, sends data
    UDR = data;
#else
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));

    // Put data into buffer, sends data
    UDR0 = data;
#endif
}

void uartWriteString(char *string)
{
    while (*string)
        uartWriteByte(*string++);

    uartWriteByte('\r');
    uartWriteByte('\n');
}

#ifdef _atmega32
ISR (USART_RXC_vect)
#else
ISR (USART_RX_vect)
#endif
{
#ifdef _atmega32
    char ch = UDR;
#else
    char ch = UDR0;
#endif
    if (ch == '\r' || ch == '\n') {
        if (uRaw.pos)
            uRaw.buf[uRaw.pos] = '\0';
        uRaw.pos = 0;
        uRaw.ready = 1;
    } else {
        if (uRaw.pos < sizeof(uRaw.buf) - 1) {
            uRaw.buf[uRaw.pos] = ch;
            uRaw.pos++;
        }
    }
}

static uint8_t uartParseHex(const char *hexStr)
{
    uint8_t ret = 0;
    uint8_t i;

    char ch;

    for (i = 0; i < 2; i++) {
        ret <<= 4;
        ch = hexStr[i];
        if (ch >= '0' && ch <= '9')
            ret += (ch - ('0' - 0x00));
        else if (ch >= 'A' && ch <= 'F')
            ret += (ch - ('A' - 0x0A));
        else
            return 0xFF;
    }

    return ret;
}

UARTData getUartData(void)
{
    UARTData ret = {UART_CMD_NO, CMD_RC_END};

    if (uRaw.ready) {
        uRaw.ready = 0;
#ifdef _atmega328p
        uartWriteString(uRaw.buf);
#endif

        // Check command type
        if (strncmp(&uRaw.buf[0], "RC ", 3) == 0) {
            ret.type = UART_CMD_RC;
            ret.command = uartParseHex(&uRaw.buf[3]);
        }

    }

    return ret;
}
