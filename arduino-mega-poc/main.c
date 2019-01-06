#define F_CPU 16000000UL
#define BAUD 115200

#include <avr/io.h>
#include <util/setbaud.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

void uart_init(void) {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

void uart_putchar(char c, FILE *stream) {
    if (c == '\n') {
        uart_putchar('\r', stream);
    }
    UDR0 = c;
    loop_until_bit_is_set(UCSR0A, UDRE0);
}

char uart_getchar(FILE *stream) {
    loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
}


static FILE uart_stdin = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
static FILE uart_stdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

int8_t postbl[] = {
                   0, // 2'b00
                   1, // 2'b01
                   3, // 2'b10
                   2, // 2'b11
};

int main(void) {
    uart_init();
    stdout = &uart_stdout;
    stdin  = &uart_stdin;

    DDRA = 0x00;
    PORTA = 0xFF;

    uint8_t enc_prev[2] = {0, 0};
    uint8_t enc_cur[2] = {0, 0};

    int32_t xpos = 0;
    int32_t ypos = 0;

    int16_t cnt = 0;

    for(;;) {
        uint8_t buf = PINA;
        enc_cur[0] = buf & 0x03;
        enc_cur[1] = (buf & 0x0C) >> 2;

        for (int i = 0; i < 2; i++) {
            if (enc_prev[i] != enc_cur[i]) {
                char dir = i == 1 ? 'x' : 'y';
                //printf("coord = %c, enc_prev = %d, enc_cur = %d, pos_prev = %d, pos_cur = %d\r\n", dir, enc_prev[i], enc_cur[i], postbl[enc_prev[i]], postbl[enc_cur[i]]);

                //printf("dir = %d\r\n", postbl[enc_cur[i]] - postbl[enc_prev[i]]);
                switch (postbl[enc_cur[i]] - postbl[enc_prev[i]]) {
                case 1:
                case -3:
                    if (dir == 'x') {
                        xpos ++;
                    } else {
                        ypos ++;
                    }
                    break;
                case -1:
                case 3:
                    if (dir == 'x') {
                        xpos --;
                    } else {
                        ypos --;
                    }
                    break;
                }

                enc_prev[i] = enc_cur[i];
                cnt++;

                if (cnt > 100) {
                    printf("x = %ld, y = %ld\r\n", xpos, ypos);
                    cnt = 0;
                }
            }
        }
    }
}
