#include "uart_print.h"

void uart_open()
{
	static bool is_uart_open = false;
	if(is_uart_open) return;
	is_uart_open = true;
	DDRD &= ~(1 << DDD0);	// RX - Input
	DDRD |= (1 << DDD1);	// TX - Output
	
	UBRR0H = 0x00;		// Asynchronous normal mode: UBRRn = fosc/(16*BAUD) - 1
	UBRR0L = 0x68;		// UBRR = 104 = 0x68
	
	UCSR0A &= ~(1 << U2X0);
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (0 << UMSEL00) | (00 << UPM00) | (0 << USBS0) | (3 << UCSZ00); // 8-bit data, 1 stop bit, no parity -> arduino_echo compatible
	
	char openMsg[] = "Connection opened with uart_open()";
	uart_println(openMsg);
}

void uart_putc(const char data, const int8_t option)
{
	uart_open();
	if(option & PUTC_NPABLE)
	{
		if(data < 32)
		{
			static char npable_data[][4]
			= {	"NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
				"BS" , "TAB", "LF" , "VT" , "FF" , "CR" , "SO" , "SI" ,
				"DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
				"CAN", "EM" , "SUB", "ESC", "FS" , "GS" , "RS" , "US"	};
			
			uart_print((char*)(npable_data + data));
			return;
		}
	}
	
	while(!(UCSR0A & (1 << UDRE0)));	// wait data register empty
	UDR0 = data;						// transmit back
	while(!(UCSR0A & (1 << TXC0)));		// wait transmit done
}

void uart_print(const char* const data)
{
	for (size_t i=0; ; i++)
	{
		/* null-terminated is guaranteed with string literals */
		if(data[i] == '\0') return;
		uart_putc(data[i], PUTC_DEFAULT);
	}
}

void uart_println(const char* const data)
{
	uart_print(data);
	uart_print("\n\r");
}

void uart_printnum_tpshow(const int8_t option)
{
	// note that 0b10000000 & 0b1100000 = 0b10000000 is still true
	// if option is		0b10000000: 1 option actually used
 	// and PNUM_X is	0b11000000: 2 option combined
	// then option & OPTION_ANY is still true
	if(option & PNUM_TPSHOW)
	{
		if(option & PNUM_BIN) uart_print("0b");
		else if(option & PNUM_HEX) uart_print("0x");
	}
}

void uart_printnum_dec(pnum_int64_t data, const int8_t option, const int8_t min_size)
{
	typedef uint64_t pnum_uint64_t;
	pnum_uint64_t udata;
	if(data < 0)
	{
		udata = (pnum_uint64_t)-data;
		uart_putc('-', PUTC_DEFAULT);
	}
	else
	{
		udata = (pnum_uint64_t)data;
		if(data > 0)
		{
			if(option & PNUM_DECPLUS) uart_putc('+', PUTC_DEFAULT);
		}
	}
	/* data has been converted to non-negative (udata) at this point
	 * only absolute value of data can be further processed
	 */
	
	/* Older implementation
	 * Not used because it needs an array to store digit
	 */
	/* goto alternative_implementation;
	
	static int8_t unit_c[max_digit_represented];
	for(int i = 0; i < max_digit_represented; i++)
	{
		unit_c[max_digit_represented-1-i] = udata % 10;
		udata = (pnum_uint64_t)(udata / (pnum_uint64_t)10);
	}
	static bool begin_print_digit;
	begin_print_digit = option & PNUM_LZERO;
	for(int i = 0; i < max_digit_represented; i++)
	{
		if(!begin_print_digit)
		{
			if(unit_c[i] != 0) begin_print_digit = true;
			else continue;
		}
		if(begin_print_digit)
		{
			uart_putc(unit_c[i] + 0x30,0);
		}
	}
	return;
	
alternative_implementation:
	(void)0; */
	static pnum_uint64_t largest_no_larger_power_of_10;
	static int8_t how_many_digit;
	largest_no_larger_power_of_10 = 1;
	how_many_digit = 1;
	while(udata != 0)
	{
		if(largest_no_larger_power_of_10 > udata)
		{
			largest_no_larger_power_of_10 /= 10;
			how_many_digit--;
			break;
		}
		else if(largest_no_larger_power_of_10 == udata)
		{
			break;
		}
		else if(((largest_no_larger_power_of_10*10)/10) != largest_no_larger_power_of_10)
		/* Multiplication will overflow/wrap around */
		{
			// uart_println("\n\r>> Overflow/wrap around when calculating largest_no_larger_power_of_10");
			break;
		}
		largest_no_larger_power_of_10 *= 10;
		how_many_digit++;
	}
	//uart_print("\n\r>> largest_no_larger_power_of_10 and how_many_digit calculated:");
	//uart_print("\n\r>> how_many_digit: ");
	//uart_putc(how_many_digit/10 + 0x30,0);
	//uart_putc(how_many_digit%10 + 0x30,0);
	//uart_print("\n\r>>");
	for(int i = 1; i <= (min_size - how_many_digit); i++)
	{
		uart_putc('0', PUTC_DEFAULT);
	}
	static pnum_int64_t divisor;
	divisor = largest_no_larger_power_of_10;
	for(int i = 1; i <= how_many_digit; i++)
	{
		uart_putc((char)(udata/divisor) + 0x30, PUTC_DEFAULT);
		udata = udata % divisor;
		divisor /= 10;
	}
}

void uart_printnum_bin(const pnum_int64_t data, const int8_t option, const int8_t size)
{
	pnum_int64_t mask;
	for(int i = 0; i < 64; i++)
	{
		if(64 - i > size)
		{
			continue;
		}
		mask = (pnum_int64_t)1 << (63-i);
		if(i%4 == 0)
		{
			uart_putc(' ', PUTC_DEFAULT);
		}
		if(data & mask)
		{
			uart_putc('1', PUTC_DEFAULT);
		}
		else
		{
			uart_putc('0', PUTC_DEFAULT);
		}
	}
}

void uart_printnum_hex(pnum_int64_t data, const int8_t option, const int8_t size)
{
	pnum_int64_t mask = (pnum_int64_t)0b1111 << 60;
	char hex_digit;
	for(int i = 0; i < 16; i++)
	{
		if(64 - 4*i > size)
		{
			data = data << 4;
			continue;
		}
		if(i%4 == 0)
		{
			uart_putc(' ',0);
		}
		hex_digit = ((data & mask) >> 60) & 0b1111;
		if(hex_digit < 10) uart_putc(hex_digit + 0x30, PUTC_DEFAULT);
		else uart_putc(hex_digit - 10 + 0x41, PUTC_DEFAULT);
		data = data << 4;
	}
}

int uart_printnum_input_err(const int8_t* const option, const int8_t* const size)
{
	if((*option & PNUM_BIN) && (*option & PNUM_HEX)) return 1;
	if((*option & PNUM_BIN) || (*option & PNUM_HEX))
	{
		switch(*size)
		{
			case 8:
			case 16:
			case 32:
			case 64:
			break;
			default:
			return 1;
		}
	}
	else
	{
		if(*size > PNUM_SIZE_MAX)
		{
			return 1;
		}
	}
	
	return 0;
}

int uart_printnum(pnum_int64_t data, const int8_t option, const int8_t size)
/* If a negative integer is input,
 * print as decimal print with true value
 * while print as bin/hex will print the binary representation
 */
/* uart_printnum check for option error (dependency or mutual exclusion)
 * while the individual uart_printnum_<option> just do the job
 */
{	
	if(uart_printnum_input_err(&option, &size)) return 1;
	
	if(option & PNUM_TPSHOW) uart_printnum_tpshow(option);
	
	if(option & PNUM_BIN)
	{
		uart_printnum_bin(data, option, size);
	}
	else if(option & PNUM_HEX)
	{
		uart_printnum_hex(data, option, size);
	}
	else
	{
		uart_printnum_dec(data, option, size);
	}
	
	if(option & PNUM_LN) uart_println("");
	
	return 0;
}

void uart_test()
{
	char msg[] = "This test prints some possible values of int64_t:";
	uart_println(msg);
	uart_print("  Number zero =  ");
	uart_printnum(0, PNUM_DEC, PNUM_SIZE_MAX);
	uart_print(" (= ");
	uart_printnum(0, PNUM_DEC, PNUM_SIZE_MIN);
	uart_println(")");
	uart_print("  INT64_MIN   = ");
	uart_printnum(INT64_MIN, PNUM_DECPLUS | PNUM_LN, PNUM_SIZE_MAX);
	uart_print("  INT64_MAX   = ");
	uart_printnum(INT64_MAX, PNUM_DECPLUS | PNUM_LN, PNUM_SIZE_MAX);
	uart_println("   Decimal\t\t\t   Binary\t\t\t\t\t\t\t\t\t\t\t   Hexadecimal");
	
	pnum_int64_t print_value = INT64_MIN;
	static const pnum_int64_t print_value_step = (pnum_int64_t)1000000000000000000;
	while(1)
	{
		uart_print(">> ");
		uart_printnum(print_value, PNUM_DECPLUS, PNUM_SIZE_MAX);
		uart_print("\t\t>> ");
		uart_printnum(print_value, PNUM_BIN_0b, PNUM_SIZE_64BIT);
		uart_print("\t\t>> ");
		uart_printnum(print_value, PNUM_HEX_0x, PNUM_SIZE_64BIT);
		if (print_value == INT64_MAX) uart_println("  -> INT64_MAX");
		else if(print_value == INT64_MIN) uart_println("  -> INT64_MIN");
		else uart_println("");
		
		if(print_value == INT64_MAX)
		{
			print_value = INT64_MIN;
			break;
		}
		else if(INT64_MAX - print_value_step < print_value)
		{
			print_value = INT64_MAX;
		} 
		else print_value += print_value_step;
		_delay_ms(10);
	}
	
	// This loop contain receive sequence - unused
	/* while(1)
	{
		while(!(UCSR0A & (1 << RXC0)));		// wait to receive
		int8_t result = UDR0;				// receive
		//while(!(UCSR0A & (1 << UDRE0)));	// wait data register empty
		//if(strcmp(result,'\r')) continue;
		//uart_putc('-');
		while(!(UCSR0A & (1 << UDRE0)));	// wait data register empty
		UDR0 = '-';						// transmit back
		while(!(UCSR0A & (1 << TXC0)));		// wait transmit done
		while(!(UCSR0A & (1 << UDRE0)));	// wait data register empty
		UDR0 = result;						// transmit back
		while(!(UCSR0A & (1 << TXC0)));		// wait transmit done
		while(!(UCSR0A & (1 << UDRE0)));	// wait data register empty
		UDR0 = '\n';						// transmit back
		while(!(UCSR0A & (1 << TXC0)));		// wait transmit done
		while(!(UCSR0A & (1 << UDRE0)));	// wait data register empty
		UDR0 = '\r';						// transmit back
		while(!(UCSR0A & (1 << TXC0)));		// wait transmit done
	}*/
}
