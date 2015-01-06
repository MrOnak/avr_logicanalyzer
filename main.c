/**
 * Logic Analyzer based around an ATTiny2313
 *
 * Constantly pumps out the values of the PINB register via Serial
 * The Serial connection runs at 38400 baud.
 *
 * Higher Baudrates are probably possible when the MUC is clocked higher
 * than the 8MHz internal clock but I have had inconsistency issues
 * when I tried with 8MHz.
 */
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "main.h"
#include "uart/uart.h"

/**
 * Will constantly pump out the value of the PINB register
 *
 */
int main() {
  init();
  uint8_t start = '#';
  uint8_t reg = 0x00;
  char buffer[3] = {0x0D, 0x0A, 0x00};
  
  while (1) {
    // inverting the register values because of the pullups
    reg = ~PINB;
    
    uart_putc(start);
    uart_putc(reg);
    uart_puts(buffer);
    
  }

  return 0;
}

/**
 * Initializes the PB register as pullup-inputs and starts UART
 */
void init() {
    uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
    sei();
    
    // set all PINB pins as inputs with pullups
    DDRD  &= ~(1 << PB0) | ~(1 << PB1) | ~(1 << PB2) | ~(1 << PB3) | ~(1 << PB4) | ~(1 << PB5) | ~(1 << PB6) | ~(1 << PB7);
    PORTB |=  (1 << PB0) |  (1 << PB1) |  (1 << PB2) |  (1 << PB3) |  (1 << PB4) |  (1 << PB5) |  (1 << PB6) |  (1 << PB7);
    
    uart_puts_P("8 channel Logic Analyzer ready");
}
