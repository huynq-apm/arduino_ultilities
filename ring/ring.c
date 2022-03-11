#include "ring.h"

int ring_constructor(ring_t** ring_handle, ring_index_t size)
{
	if(*ring_handle != NULL) return 1;
	*ring_handle = (ring_t*) calloc(1, sizeof(ring_t));
	if(*ring_handle == NULL) goto FREE_AND_RETURN;
	
	ring_t* ring = *ring_handle;
	// some sizeof() and pointer value used for debug
	/* uart_print("sizeof(ring_t) = "); uart_printnum((pnum_int64_t) sizeof(ring_t), PNUM_DEC | PNUM_LN, PNUM_SIZE_MIN);
	uart_print("*ring_handle = "); uart_printnum((pnum_int64_t) *ring_handle, PNUM_HEX_0x | PNUM_LN, PNUM_SIZE_16BIT);
	uart_print("ring = "); uart_printnum((pnum_int64_t) ring, PNUM_HEX_0x | PNUM_LN, PNUM_SIZE_16BIT);
	uart_print("&(ring->data) = "); uart_printnum((pnum_int64_t) &(ring->data), PNUM_HEX_0x | PNUM_LN, PNUM_SIZE_16BIT);
	uart_print("&(ring->size) = "); uart_printnum((pnum_int64_t) &(ring->size), PNUM_HEX_0x | PNUM_LN, PNUM_SIZE_16BIT);
	uart_print("&(ring->hold) = "); uart_printnum((pnum_int64_t) &(ring->hold), PNUM_HEX_0x | PNUM_LN, PNUM_SIZE_16BIT);
	uart_print("&(ring->next_in) = "); uart_printnum((pnum_int64_t) &(ring->next_in), PNUM_HEX_0x | PNUM_LN, PNUM_SIZE_16BIT);
	uart_print("&(ring->next_out) = "); uart_printnum((pnum_int64_t) &(ring->next_out), PNUM_HEX_0x | PNUM_LN, PNUM_SIZE_16BIT);
	uart_print("ring->data = "); uart_printnum((pnum_int64_t) ring->data, PNUM_HEX_0x | PNUM_LN, PNUM_SIZE_16BIT); */
	ring->data = (ring_data_t*) calloc(size, sizeof(ring_data_t));
	// uart_println("after calloc() for ring->data");
	// uart_print("ring->data = "); uart_printnum((pnum_int64_t) ring->data, PNUM_HEX_0x | PNUM_LN, PNUM_SIZE_16BIT);
	if(ring->data == NULL) goto FREE_AND_RETURN;
	ring->size = size;
	return 0;

FREE_AND_RETURN:
	ring_destructor(ring_handle);
	return 2;
}

void ring_destructor(ring_t** ring_handle)
{
	if(*ring_handle != NULL)
	{
		if((*ring_handle)->data != NULL)
		{
			free((*ring_handle)->data);
			(*ring_handle)->data = NULL;
		}
		
		free(*ring_handle);
		*ring_handle = NULL;
	}
}

bool is_ring_empty(const ring_t* ring)
{
	return (ring->hold == 0);
}

bool is_ring_full(const ring_t* ring)
{
	return (ring->hold == ring->size);
}

int ring_in(ring_t* ring, ring_data_t data)
{
	if (!is_ring_full(ring))
	{
		ring->data[ring->next_in] = data;
		ring->next_in = (ring->next_in + 1) % ring->size;
		ring->hold++;
		return 0;
	}
	else return 1;
}

int ring_out(ring_t* ring, ring_data_t* data)
{
	int peek_return = ring_peek(ring, data);
	if(peek_return == 0)
	{
		ring->next_out = (ring->next_out + 1) % ring->size;
		ring->hold--;
		return 0;
	}
	else return 1;
}

int ring_peek(const ring_t* ring, ring_data_t* data)
{
	if (!is_ring_empty(ring))
	{
		*data =	ring->data[ring->next_out];
		return 0;
	}
	else return 1;
}

int ring_empty(ring_t* ring)
{
	ring->next_in = ring->next_out;
	ring->hold = 0;
	
	return 0;
}

void ring_uart_expose(const ring_t* ring)
{
	// ring->hold = 6; // with const ring_t* ring: this line is marked error when compile -> correct
	uart_println(">> RING CONTENT:");
	uart_print("Currently hold ");
	uart_printnum(ring->hold, PNUM_DEFAULT, PNUM_SIZE_MIN);
	// if(ring->hold != 0) uart_print("<< hold != 0 >>");
	uart_print(" / ");
	uart_printnum(ring->size, PNUM_DEFAULT, PNUM_SIZE_MIN);
	if(is_ring_empty(ring))		uart_println(" (empty)");
	else if(is_ring_full(ring))	uart_println(" (full) ");
	else uart_println("");
	
	uart_print("Next in  @ ");
	uart_printnum(ring->next_in, PNUM_DEFAULT, PNUM_SIZE_MIN);
	uart_print("\t| ");
	uart_print("Next out @ ");
	uart_printnum(ring->next_out, PNUM_DEFAULT | PNUM_LN, PNUM_SIZE_MIN);
	
	for(ring_index_t i=0; i < ring->size; i++)
	{
		if((ring->next_in - i + ring->size - 1) % ring->size < ring->hold) uart_print("# ");
		else uart_print("  ");
		uart_print("Ring element ");
		uart_printnum(*(ring->data + i), PNUM_DEFAULT, PNUM_SIZE_MIN);
		uart_print("\t");
		if(i == ring->next_in) uart_print(" <- Next in here ");
		if(i == ring->next_out) uart_print(" <- Next out here ");
		uart_println("");
	}
	
	uart_println(">> END OF RING CONTENT.");
}

void ring_test()
{
	ring_t* test_ring = NULL;
	/* global variable is zero-initialized but local variable is not
	 * zero-initialized pointer is NULL
	 */
	ring_index_t test_ring_size = 5;
	int constructor_return;
	constructor_return = ring_constructor(&test_ring, test_ring_size);
	if(constructor_return != 0)
	{
		uart_println("Ring constructor failed.");
		return;
	}
	ring_uart_expose(test_ring);
	
	// return;
	
	uart_println("\n>> BEGIN TEST SEQUENCE.");
	// For reference:  0b1111000011110000 <- 16 bit
	int16_t test_seq = 0b1110100111000101; // 1 for data in, 0 for data out
	int8_t in_data[] = {23, 15, 94, 47,
						25, 75, 35, 20,
						65, 71, 34, 95,
						40, 59, 37, 96};
	int8_t out_data;
	int ring_in_return;
	int ring_out_return;
	int8_t reset_at[] = {3, 4, 8, 14};
	for(int i = 0; i < 16; i++)
	{
		uart_println("\n----------------------------------------");
		uart_print("[[ SEQUENCE ");
		uart_printnum(i, PNUM_DEFAULT, PNUM_SIZE_MIN);
		uart_print(": ");
		if(test_seq & (0x8000 >> i))
		{
			uart_print("Put into ring (");
			ring_in_return = ring_in(test_ring, in_data[i]);
			if(ring_in_return == 0)
			{
				uart_printnum(in_data[i], PNUM_DEFAULT, PNUM_SIZE_MIN);
			}
			else uart_print("nothing");
		}
		else
		{
			uart_print("Take out of ring (");
			ring_out_return = ring_out(test_ring, &out_data);
			if(ring_out_return == 0)
			{
				uart_printnum(out_data, PNUM_DEFAULT, PNUM_SIZE_MIN);
			}
			else
			{
				uart_print("nothing");
			}
			
		}
		uart_println(") ]]\n");
		ring_uart_expose(test_ring);
		
		for(int j = 0; j < sizeof(reset_at)/sizeof(reset_at[0]); j++)
		{
			if(i == reset_at[j])
			{
				uart_print("\n[[ RESET AFTER SEQUENCE ");
				uart_printnum(i, PNUM_DEFAULT, PNUM_SIZE_MIN);
				uart_println(" ]]\n");
				ring_empty(test_ring);
				ring_uart_expose(test_ring);
			}
		}
	}
}
