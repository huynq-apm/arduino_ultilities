#include "uart_ring.h"

ring_t* uart_ring_rx = NULL;
// *uart_ring_rx = NULL;

void uart_open();
/* Actual implementation in uart_print.c
 *
 * uart_open() prototype is not in uart_print.h
 * because user don't need to use it
 */

void uart_openISR()
{
	uart_open();
	
	static bool is_uart_openISR = false;
	if(is_uart_openISR) return;
	is_uart_openISR = true;
	// Make use of interrupt
	UCSR0B |= (1 << RXCIE0); // Receive complete interrupt
	// UCSR0B |= (1 << TXCIE0); // Transmit complete interrupt
	sei(); // enable global interrupt
	char openMsg[] = "Connection opened with uart_openISR()";
	uart_println(openMsg);
}

int uart_ring_init()
{
	uart_openISR();
	if(uart_ring_rx != NULL) return 0;
	int constructor_return;
	constructor_return = ring_constructor(&uart_ring_rx, (ring_index_t) UART_RING_RX_SIZE);
	return constructor_return;
}

ISR(USART_RX_vect)
{
	ring_data_t data = UDR0;
	while(!(UCSR0A & (1 << UDRE0)));	// wait data register empty
	ring_in(uart_ring_rx, data);
}

bool is_uart_available()
{
	uart_openISR();
	return !is_ring_empty(uart_ring_rx);
}

ring_data_t uart_getc()
{
	uart_openISR();
	while(!is_uart_available());
	ring_data_t data;
	ring_out(uart_ring_rx, &data);
	return data;
}

void uart_flush(const ring_data_t terminate)
{
	ring_data_t data;
	do 
	{
		data = uart_getc();
	} while (data != terminate);
}

void uart_ring_test()
{
	uart_ring_init();
	if(is_ring_empty(uart_ring_rx)) uart_print("UART receive buffer is empty. ");
	else uart_print("UART receive buffer is not empty. ");
	
	if(is_uart_available()) uart_println("UART receive data available.");
	else uart_println("UART receive data unavailable.");
	
	uart_println("");
    uart_println("From now, the MCU will echo your input, character by character.");
	uart_println("");
    
	ring_data_t data;
	const char prompt_input[] = ">> ";
	uart_print(prompt_input);
	uint8_t receiveIndex = 1;
    
	while(1)
	{
		data = uart_getc();
		uart_print("Byte ");
		uart_printnum(receiveIndex, PNUM_DEC, PNUM_SIZE(2));
		receiveIndex++;
		uart_print(" - ");
		uart_printnum(data, PNUM_BIN_0b, PNUM_SIZE_8BIT);
		uart_print(" - ");
		uart_print("<");
		uart_putc(data, PUTC_NPABLE); //uart_print(data);
		uart_print(">");
		uart_println("");
		if(!is_uart_available())
		{
			receiveIndex = 1;
			uart_print(prompt_input);
		}
	}
	
	/* old code, when uart_getc didn't polling for is_uart_available() */
	while(1)
	{
		if(is_uart_available())
		{
			receiveIndex = 1;
			do
			{
				data = uart_getc();
				uart_print("Byte ");
				uart_printnum(receiveIndex, PNUM_DEC, PNUM_SIZE(2));
				receiveIndex++;
				uart_print(" - ");
				uart_printnum(data, PNUM_BIN_0b, PNUM_SIZE_8BIT);
				uart_print(" - ");
				uart_print("<");
				uart_putc(data, PUTC_NPABLE);
				uart_print(">");
				uart_println("");
			} while(is_uart_available());
			uart_print(prompt_input);
		}
	}
}
