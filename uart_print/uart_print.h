#ifndef UART_PRINT_H
#define UART_PRINT_H

#include <generutils_f_cpu.h>

/* #define F_CPU 16000000UL for correct BAUD setting */
#ifndef F_CPU
	#error >> uart_print requires F_CPU defined
#elif (F_CPU != 16000000UL)
    #error >> uart_print only works with F_CPU = 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint-gcc.h>

// void uart_open();
// void uart_openISR();

#define PUTC_ASCII		(0)				
#define PUTC_NPABLE		(1<<0)			// print non-printable characters as symbols
#define PUTC_OPTION2	(1<<1)			// option 2
#define PUTC_OPTION3	(1<<2)			// option 3
#define PUTC_OPTION4	(1<<3)			// option 4
#define PUTC_OPTION5	(1<<4)			// option 5
#define PUTC_OPTION6	(1<<5)			// option 6
#define PUTC_OPTION7	(1<<6)			// option 7
#define PUTC_OPTION8	(1<<7)			// option 8
#define PUTC_DEFAULT	(PUTC_ASCII)	

void uart_putc(char data, const int8_t option);
void uart_print(const char* const data);
void uart_println(const char* const data);

typedef int64_t pnum_int64_t;

#define PNUM_DEC		(0)
#define PNUM_OPTION1	(1<<0)			// with leading zero -> removed, as size is now passed
#define PNUM_BIN		(1<<1)			// in binary
#define PNUM_HEX		(1<<2)			// in hexadecimal
#define PNUM_OPTION4	(1<<3)			// option 4
#define PNUM_OPTION5	(1<<4)			// option 5
#define PNUM_LN			(1<<5)			// newline after number
#define PNUM_DECPLUS	(1<<6)			// show plus sign of decimal
#define PNUM_TPSHOW		(1<<7)			// show 0b if bin / 0x if hex representation
#define PNUM_BIN_0b		(PNUM_BIN | PNUM_TPSHOW)
#define PNUM_HEX_0x		(PNUM_HEX | PNUM_TPSHOW)
#define PNUM_DEFAULT	(PNUM_DEC)		// no option, just default

#define PNUM_SIZE(X)	(X)				// specific size of decimal (with padding 0 if needed)
#define PNUM_SIZE_MIN	(0)				// minimum size of decimal
#define PNUM_SIZE_MAX	(19)			// because 64 bit
#define PNUM_SIZE_8BIT	(8)				// for bin / hex, print lowest 08 bit only
#define PNUM_SIZE_16BIT	(16)			// for bin / hex, print lowest 16 bit only
#define PNUM_SIZE_32BIT	(32)			// for bin / hex, print lowest 32 bit only
#define PNUM_SIZE_64BIT	(64)			// for bin / hex, print lowest 64 bit only

int uart_printnum(pnum_int64_t data, const int8_t option, const int8_t size);
void uart_test();

// #define uart_println(); uart_println(""); // not work - hence the uart_println("");

#endif /* UART_PRINT_H */
