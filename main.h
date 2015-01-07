#ifndef LOGIC_ANALYZER_H
#define LOGIC_ANALYZER_H

#define UART_BAUD_RATE      38400   
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#define NACK 0x30                         // '0'
#define ACK 0x31                          // '1'
#define READY 0x32                        // '2' - send after data gathering is completed

#define ERR_BUFFER_TOO_LARGE 0x4531       // 'E1'
#define ERR_INVALID_COMMAND 0x4532        // 'E2'
#define ERR_RUNNING 0x4533                // 'E3' - send when commands are sent while gathering is running

#define CMD_SEND_MAX_BUFFER_SIZE 0x6D     // 'm'
#define CMD_SEND_BUFFER_SIZE 0x62         // 'b'
#define CMD_SET_BUFFER_SIZE 0x4D          // 'M'
#define CMD_SET_TRIGGER 0x54              // 'T'
#define CMD_GET_TRIGGER 0x74              // 't'
#define CMD_NO_TRIGGER 0x4E               // 'N'
#define CMD_START 0x53                    // 'S'
#define CMD_STOP 0x73                     // 's'
#define CMD_SEND_BUFFER 0x42              // 'B'

#define MAX_BUFFER_SIZE 40

/**
 * contains the register value from last iteration
 *
 * is used by checkTriggers()
 */
uint8_t lastData;

/**
 * configured buffer size.
 *
 * note that the actual buffer is always of MAX_BUFFER_SIZE
 * size but reading data will stop once bufferPos matches
 * bufferSize
 */
uint8_t bufferSize = MAX_BUFFER_SIZE;

/**
 * the data buffer
 */
uint8_t buffer[MAX_BUFFER_SIZE];
/**
 * current position in the buffer
 */
uint8_t bufferPos = 0;

/** 
 * rising and falling triggers (in that order)
 */
uint8_t triggers[2] = {0x00, 0x00};

/**
 * flag that indicates that data gathering is currently in progress
 * when set to 1.
 */
uint8_t running = 0;
 
/**
 * set 1 after the trigger conditions are met. 
 * writeToBuffer() will not fill the buffer until
 * this is 1 OR noTrigger is 1
 */
uint8_t triggered = 0;

/**
 * if set 1, then writeToBuffer() will immediately
 * start filling the buffer after cmdStart() has been
 * called.
 *
 * if it is 0 then writeToBuffer() requires triggered to 
 * be 1.
 */
uint8_t noTrigger = 1;

/**
 * Initializes the PB register as pullup-inputs and starts UART
 */
void init();

/**
 * reads commands from UART and calls the appropriate subroutine
 */
void commandParser();

/**
 * sends the maximum allowed buffer size to the client
 */
void cmdSendMaxBufferSize();

/**
 * sends the current buffer size to the client
 */
void cmdSendBufferSize();

/**
 * sets the ring buffer to the given size
 */
void cmdSetBufferSize(uint8_t size);

/**
 * configures the input triggers and sets
 * the noTrigger flag to 0.
 */
void cmdSetTrigger(uint8_t rising, uint8_t falling);

 /**
  * erases input triggers and sets the noTrigger flag
  *
  * this will cause data gathering to immediately start
  * after cmdStart() is being issued.
  */
void cmdNoTrigger();

/**
 * sends the rising and falling trigger bitmasks (in that order)
 */
void cmdGetTrigger();
 
/**
 * starts input gathering. 
 *
 * also resets the buffer position
 * to zero, effectively erasing the buffer as it's been read
 */
void cmdStart();

/**
 * stops the data gathering
 *
 * this is useful in case the triggers don't get triggered.
 *
 * will set running to 0
 */
void cmdStop();

/** 
 * sends the recorded data buffer to the client
 */
void cmdSendBuffer();

/**
 * sends an acknowledgement byte
 */
void sendAck();

/**
 * sends a generic not-acknowledged byte
 */
void sendNack();
 
/**
 * sends an error
 */
void sendErr(uint16_t err);

/**
 * checks the data register against trigger conditions
 * and resets the triggered flag if they match
 */
void checkTriggers();

/**
 * writes the current state of PINB into the ringbuffer
 * and advances the position.
 *
 * If the position reaches the bufferSize then reading is
 * terminated.
 *
 * once reading is finished an ACK will be sent
 */
void writeToBuffer();

#endif // LOGIC_ANALYZER_H