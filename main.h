#ifndef LOGIC_ANALYZER_H
#define LOGIC_ANALYZER_H

#define UART_BAUD_RATE      38400   
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

void init();

#endif // LOGIC_ANALYZER_H