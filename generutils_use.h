#ifndef GENERUTILS_USE_H
#define GENERUTILS_USE_H

/* Author: Nguyen Quang Huy
 * Date: From Jan 2022
 *
 * Each of these utility have a test function
 * which helps showing how to use it
 * 
 * The prototype for general_util_use.h is _general_util_use.h
 * Copy the prototype to your project, and remove '_'
 */

/* Define depends on your usage */
#define USE_ALL             (1)
#define USE_RING            (0)
#define USE_SWTMR           (0)
#define USE_UART_PRINT      (0)
#define USE_UART_RING       (0)

/* No modification is neccessary belows this line */
/* -------------------------------------------------- */

/* Include the only needed header,
 * or all of them
 */
#if (USE_ALL || USE_RING)
    #include "ring/ring.h"
#endif /* USE_RING */

#if (USE_ALL || USE_SWTMR)
    #include "swtmr/swtmr.h"
#endif /* USE_SWTMR */

#if (USE_ALL || USE_UART_PRINT)
    #include "uart_print/uart_print.h"
#endif /* USE_UART_PRINT */

#if (USE_ALL || USE_UART_RING)
    #include "uart_ring/uart_ring.h"
#endif /* USE_UART_RING */

#endif /* GENERUTILS_CONFIG_H */