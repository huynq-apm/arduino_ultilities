#ifndef UART_RING_H
#define UART_RING_H

//#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#include "ring/ring.h"
#include "uart_print/uart_print.h"

/* Buffer size for UART receive data */
#define UART_RING_RX_SIZE (64)

int uart_ring_init();
bool is_uart_available();
ring_data_t uart_getc();
void uart_flush(const ring_data_t terminate);
void uart_ring_test();

#endif /* UART_RING_H */
