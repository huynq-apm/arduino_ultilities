#ifndef RING_H
#define RING_H

#include <stdbool.h>
#include <stdint-gcc.h>
#include <stddef.h>
#include <stdlib.h>

#include "uart_print/uart_print.h"

typedef uint8_t ring_index_t;
typedef int8_t ring_data_t;

typedef struct st_ring
{
	ring_data_t* data;
	ring_index_t size;
	ring_index_t hold;
	ring_index_t next_in;
	ring_index_t next_out;
} ring_t;

int ring_constructor(ring_t** ring_handle, ring_index_t size);
void ring_destructor(ring_t** ring_handle);
bool is_ring_empty(const ring_t* ring);
bool is_ring_full(const ring_t* ring);
int ring_in(ring_t* ring, ring_data_t data);
int ring_out(ring_t* ring, ring_data_t* data);
int ring_peek(const ring_t* ring, ring_data_t* data);
int ring_empty(ring_t* ring);
void ring_uart_expose(const ring_t* ring);
void ring_test();

#endif /* RING_H */
