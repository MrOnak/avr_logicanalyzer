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
#include <util/delay.h>

#include "main.h"
#include "uart/uart.h"

/**
 * Will constantly pump out the value of the PINB register
 *
 */
int main() {
  init();

  while (1) {
    commandParser();
    
    if (running == 1) {
      if (triggered == 0 && noTrigger == 0) {
        checkTriggers();
        lastData = PINB;
      } else {
        writeToBuffer();
      }
    }
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
    
    // set all flags
    running = 0;
    triggers[0] = 0x00;
    triggers[1] = 0x00;
    noTrigger = 1;
    triggered = 0;
    lastData = 0x00;
    
    //uart_puts_P("8 channel Logic Analyzer ready");
    
}

/**
 * reads commands from UART and calls the appropriate subroutine
 */
void commandParser() {
  if (uart_available() > 0) {
    uint16_t c = uart_getc();
    uint16_t c2;
    
    if ( c & UART_NO_DATA ) {
      // no data available from UART 
    } else {
      // new data available from UART - check for Frame or Overrun error
      if ( c & UART_FRAME_ERROR ) {
        // Framing Error detected, i.e no stop bit detected
        uart_puts_P("UART Frame Error: ");
      }
      
      if ( c & UART_OVERRUN_ERROR ) {
        // Overrun, a character already present in the UART UDR register was not read by the interrupt handler 
        // before the next character arrived, one or more received characters have been dropped
        uart_puts_P("UART Overrun Error: ");
      }
      if ( c & UART_BUFFER_OVERFLOW ) {
        //We are not reading the receive buffer fast enough, one or more received character have been dropped 
        uart_puts_P("Buffer overflow error: ");
      }
      
      switch (c) {
        case CMD_SEND_MAX_BUFFER_SIZE:  // 'm'
          cmdSendMaxBufferSize();
          break;
        case CMD_SEND_BUFFER_SIZE:      // 'b'
          cmdSendBufferSize();
          break;
        case CMD_SET_BUFFER_SIZE:       // 'M'
          while (uart_available() == 0) {}
          c = uart_getc();
          if ((c >> 8) == 0) {
            cmdSetBufferSize(c);
          } else {
            sendNack();
          }
          break;
        case CMD_SET_TRIGGER:           // 'T'
          while (uart_available() < 2) {}
          c = uart_getc();
          c2 = uart_getc();
          if ((c >> 8) == 0 && (c2 >> 8) == 0) {
            cmdSetTrigger(c, c2);
          } else {
            sendNack();
          }
          break;
        case CMD_GET_TRIGGER:           // 't'
          cmdGetTrigger();
          break;
        case CMD_NO_TRIGGER:            // 'N'
          cmdNoTrigger();
          break;
        case CMD_START:                 // 'S'
          cmdStart();
          break;
        case CMD_STOP:                  // 's'
          cmdStop();
          break;
        case CMD_SEND_BUFFER:           // 'B'
          cmdSendBuffer();
          break;
        default:
          sendErr(ERR_INVALID_COMMAND);
      }
    }    
  }
}

/**
 * sends the maximum allowed buffer size to the client
 */
void cmdSendMaxBufferSize() {
  sendAck();
  uart_putc(MAX_BUFFER_SIZE);
}

/**
 * sends the current buffer size to the client
 */
void cmdSendBufferSize() {
  sendAck();
  uart_putc(bufferSize);
}

/**
 * sets the ring buffer to the given size
 */
void cmdSetBufferSize(uint8_t size) {
  if (size <= MAX_BUFFER_SIZE) {
    bufferSize = size;
    sendAck();
  } else {
    sendErr(ERR_BUFFER_TOO_LARGE);
  }    
}

/**
 * configures the input triggers and sets
 * the noTrigger flag to 0.
 */
void cmdSetTrigger(uint8_t rising, uint8_t falling) {
  noTrigger = 0;
  triggers[0] = rising;
  triggers[1] = falling;
  sendAck();
}

 /**
  * erases input triggers and sets the noTrigger flag
  *
  * this will cause data gathering to immediately start
  * after cmdStart() is being issued.
  */
 void cmdNoTrigger() {
   noTrigger = 1;
   triggers[0] = 0x00;
   triggers[1] = 0x00;
   sendAck();
}

/**
 * sends the rising and falling trigger bitmasks (in that order)
 */
void cmdGetTrigger() {
  uart_putc(triggers[0]);
  uart_putc(triggers[1]);  
}
 
 
/**
 * starts input gathering. also resets the buffer position
 * to zero, effectively erasing the buffer as it's being read later
 */
void cmdStart() {
  uint8_t i;
  
  if (running == 0) {
    bufferPos = 0;
    running = 1;
    triggered = 0;
    lastData = PINB;
    
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
      buffer[i] = 0x00;
    }
    
    sendAck();
  } else {
    sendErr(ERR_RUNNING);
  }
}

/**
 * stops the data gathering
 *
 * this is useful in case the triggers don't get triggered.
 *
 * will set running to 0
 */
void cmdStop() {
  running = 0;
  sendAck();
}

/** 
 * sends the recorded data buffer to the client
 */
void cmdSendBuffer() {
  uint8_t i;
  
  if (running == 0) {
    for (i = 0; i < bufferSize; i++) {
      uart_putc(buffer[i]);
    }
  } else {
    sendErr(ERR_RUNNING);
  }
}

/**
 * sends an acknowledgement byte
 */
void sendAck() {
  uart_putc(ACK);
}

/**
 * sends a generic not-acknowledged byte
 */
void sendNack() {
  uart_putc(NACK);
}

/**
 * sends an error
 */
void sendErr(uint16_t err) {
  uart_putc(err >> 8);
  uart_putc(err);
}

/**
 * checks the data register against trigger conditions
 * and resets the triggered flag if they match
 */ 
void checkTriggers() {
  // @todo implement checkTriggers()
}

/**
 * writes the current state of PINB into the ringbuffer
 * and advances the position.
 *
 * If the position reaches the bufferSize then reading is
 * terminated.
 *
 * once reading is finished an ACK will be sent
 */
void writeToBuffer() {
  buffer[bufferPos] = PINB;
  bufferPos++;
  
  // stop reading if buffer size is reached
  if (bufferPos == bufferSize) {
    running   = 0;
    triggered = 0;
    uart_putc(READY);
  }
}
