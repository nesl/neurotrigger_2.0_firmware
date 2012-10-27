#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "uart.h"
#include "uart_buffer.h"
#include "ui.h" //for debugging
#include "string.h"

//XMEGA uart DRIVER
//Configures specified serial port for uart operation using N-8-1 frame
//Assumes 32.000 MHz peripheral clock
//Default baud rate is 115200, unless specified


//==================================
//= INITIALIZATION ROUTINES
//==================================

//Select which uart to initialize from the list of defined constants in the io.h
//Pass in Baud rate from a list of defined constants
void init_uart(USART_t* which, uint8_t baud_rate){
	//Config the PORT
		if (&(*which) == &USARTC0){
			PORTC.DIRSET = B8(00001000); //TX pin as output
			PORTC.OUTSET = B8(00001000); //TX initial output value is high
		}
		else if (&(*which) == &USARTC1) {
			PORTC.DIRSET = B8(10000000); //TX pin as output
			PORTC.OUTSET = B8(10000000); //TX initial output value is high
		}
		else if (&(*which) == &USARTD0) {
			PORTD.DIRSET = B8(00001000); //TX pin as output
			PORTD.OUTSET = B8(00001000); //TX initial output value is high
		}	
		else if (&(*which) == &USARTD1) {
			PORTD.DIRSET = B8(10000000); //TX pin as output
			PORTD.OUTSET = B8(10000000); //TX initial output value is high
		}
		else if (&(*which) == &USARTE0) {
			PORTE.DIRSET = B8(00001000); //TX pin as output
			PORTE.OUTSET = B8(00001000); //TX initial output value is high
		}
		else {
		}
	//Set baud rate
		switch(baud_rate){
		case BAUD_38400:
			//BSEL = 0xCC5; BSCALE = -6; error = 0.01%
			which->BAUDCTRLB = 0xAC;
			which->BAUDCTRLA = 0xC5;
			break;
		case BAUD_57600:
			//BSEL = 0x86E; BSCALE = -6; error = 0.01%
			which->BAUDCTRLB = 0xA8;
			which->BAUDCTRLA = 0x6E;
			break;
		case BAUD_115200:
		default:
			//BSEL = 0x417; BSCALE = -6; error = 0.01%
			which->BAUDCTRLB = 0xA4;
			which->BAUDCTRLA = 0x17;		
		}
	//Config USART Module
		which->CTRLA = 0x00; //Current driver does not enable any interrupts
		which->CTRLB = B8(00011000); //Enable Rx and TX; Do not use double speed mode
		which->CTRLC = B8(00000011); //uart Mode; Use N-8-1 frame configuration
	//
}





//==================================
//= DISPLAY (BLOCKING) FUNCTIONS
//==================================


//Sends one byte; Spin-lock until UART is ready to transmit (BE CAREFUL!)
void uart_send_byte( USART_t* which, unsigned char dataB){
	while((which->STATUS & USART_DREIF_bm) == 0x00); //wait until the transmit buffer is ready to receive new data (DREIF flag is 1 when empty)
	which->DATA = dataB;	
}

//Sends an entire string (blocking)
void uart_send_string(USART_t* which, char* theString){
	uint16_t length = (uint16_t)strlen(theString);
	for (uint16_t i=0; i<length; i++){uart_send_byte(which, theString[i]);}	
}
	
//Most Significant Bit first
void uart_send_BIN4(USART_t* which, uint8_t lowb){
	switch(lowb){
	case(0):
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		break;
	case(1):
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		break;
	case(2):
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		break;
	case(3):
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		break;
	case(4):
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		break;
	case(5):
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		break;
	case(6):
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		break;
	case(7):
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		break;
	case(8):
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		break;
	case(9):
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		break;
	case(10):
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		break;
	case(11):
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		break;
	case(12):
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		uart_send_byte(which,'0');
		break;
	case(13):
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		uart_send_byte(which,'1');
		break;
	case(14):
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		uart_send_byte(which,'0');
		break;
	case(15):
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		uart_send_byte(which,'1');
		break;
	}	
}

//Sends out tosend as ASCII text in 'b01101010' format
void uart_send_BIN8(USART_t* which, uint8_t lowb){
	uart_send_byte(which,'b');
	uart_send_BIN4(which, lowb>>4);
	uart_send_BIN4(which, lowb & 0x0F);
}
	
void uart_send_HEX4(USART_t* which, uint8_t lowb){
	switch(lowb){
	case(0):
		uart_send_byte( which, '0');
		break;
	case(1):
		uart_send_byte( which, '1');
		break;
	case(2):
		uart_send_byte( which, '2');
		break;
	case(3):
		uart_send_byte( which, '3');
		break;
	case(4):
		uart_send_byte( which, '4');
		break;
	case(5):
		uart_send_byte( which, '5');
		break;
	case(6):
		uart_send_byte( which, '6');
		break;
	case(7):
		uart_send_byte( which, '7');
		break;
	case(8):
		uart_send_byte( which, '8');
		break;
	case(9):
		uart_send_byte( which, '9');
		break;
	case(10):
		uart_send_byte( which, 'A');
		break;
	case(11):
		uart_send_byte( which, 'B');
		break;
	case(12):
		uart_send_byte( which, 'C');
		break;
	case(13):
		uart_send_byte( which, 'D');
		break;
	case(14):
		uart_send_byte( which, 'E');
		break;
	case(15):
		uart_send_byte( which, 'F');
		break;
	}	
}

void uart_send_HEX8(USART_t* which, uint8_t lowb){
	uart_send_HEX4(which, lowb>>4);
	uart_send_HEX4(which, lowb & 0x0F);
}

void uart_send_HEX16b(USART_t* which, uint8_t highb, uint8_t lowb){
	uart_send_HEX8(which, highb);
	uart_send_HEX8(which, lowb);
}

void uart_send_HEX16(USART_t* which, uint16_t highb){
	uint8_t blah;
	blah = (uint8_t)(highb>>8);
	uart_send_HEX8(which, blah);
	blah = (uint8_t)(highb & 0x00FF);
	uart_send_HEX8(which, blah);
}


//======================================================================
//== The Queue-based versions (NON-BLOCKING) of the display functions ==
//======================================================================


//Most Significant Bit first
void uart_enq_BIN4(uint8_t lowb){
	switch(lowb){
	case(0):
		uart_enqueue('0');
		uart_enqueue('0');
		uart_enqueue('0');
		uart_enqueue('0');
		break;
	case(1):
		uart_enqueue('0');
		uart_enqueue('0');
		uart_enqueue('0');
		uart_enqueue('1');
		break;
	case(2):
		uart_enqueue('0');
		uart_enqueue('0');
		uart_enqueue('1');
		uart_enqueue('0');
		break;
	case(3):
		uart_enqueue('0');
		uart_enqueue('0');
		uart_enqueue('1');
		uart_enqueue('1');
		break;
	case(4):
		uart_enqueue('0');
		uart_enqueue('1');
		uart_enqueue('0');
		uart_enqueue('0');
		break;
	case(5):
		uart_enqueue('0');
		uart_enqueue('1');
		uart_enqueue('0');
		uart_enqueue('1');
		break;
	case(6):
		uart_enqueue('0');
		uart_enqueue('1');
		uart_enqueue('1');
		uart_enqueue('0');
		break;
	case(7):
		uart_enqueue('0');
		uart_enqueue('1');
		uart_enqueue('1');
		uart_enqueue('1');
		break;
	case(8):
		uart_enqueue('1');
		uart_enqueue('0');
		uart_enqueue('0');
		uart_enqueue('0');
		break;
	case(9):
		uart_enqueue('1');
		uart_enqueue('0');
		uart_enqueue('0');
		uart_enqueue('1');
		break;
	case(10):
		uart_enqueue('1');
		uart_enqueue('0');
		uart_enqueue('1');
		uart_enqueue('0');
		break;
	case(11):
		uart_enqueue('1');
		uart_enqueue('0');
		uart_enqueue('1');
		uart_enqueue('1');
		break;
	case(12):
		uart_enqueue('1');
		uart_enqueue('1');
		uart_enqueue('0');
		uart_enqueue('0');
		break;
	case(13):
		uart_enqueue('1');
		uart_enqueue('1');
		uart_enqueue('0');
		uart_enqueue('1');
		break;
	case(14):
		uart_enqueue('1');
		uart_enqueue('1');
		uart_enqueue('1');
		uart_enqueue('0');
		break;
	case(15):
		uart_enqueue('1');
		uart_enqueue('1');
		uart_enqueue('1');
		uart_enqueue('1');
		break;
	}	
}

//enqs out toenq as ASCII text in 'b01101010' format
void uart_enq_BIN8(uint8_t lowb){
	uart_enqueue('b');
	uart_enq_BIN4(lowb>>4);
	uart_enq_BIN4(lowb & 0x0F);
}
	

void uart_enq_HEX4(uint8_t lowb){
	switch(lowb){
	case(0):
		uart_enqueue( '0');
		break;
	case(1):
		uart_enqueue( '1');
		break;
	case(2):
		uart_enqueue( '2');
		break;
	case(3):
		uart_enqueue( '3');
		break;
	case(4):
		uart_enqueue( '4');
		break;
	case(5):
		uart_enqueue( '5');
		break;
	case(6):
		uart_enqueue( '6');
		break;
	case(7):
		uart_enqueue( '7');
		break;
	case(8):
		uart_enqueue( '8');
		break;
	case(9):
		uart_enqueue( '9');
		break;
	case(10):
		uart_enqueue( 'A');
		break;
	case(11):
		uart_enqueue( 'B');
		break;
	case(12):
		uart_enqueue( 'C');
		break;
	case(13):
		uart_enqueue( 'D');
		break;
	case(14):
		uart_enqueue( 'E');
		break;
	case(15):
		uart_enqueue( 'F');
		break;
	}	
}

void uart_enq_HEX8(uint8_t lowb){
	uart_enq_HEX4(lowb>>4);
	uart_enq_HEX4(lowb & 0x0F);
}

void uart_enq_HEX16b(uint8_t highb, uint8_t lowb){
	uart_enq_HEX8(highb);
	uart_enq_HEX8(lowb);
}

void uart_enq_HEX16(uint16_t highb){
	uint8_t blah;
	blah = (uint8_t)(highb>>8);
	uart_enq_HEX8(blah);
	blah = (uint8_t)(highb & 0x00FF);
	uart_enq_HEX8(blah);
}


