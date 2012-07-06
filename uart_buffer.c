#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "uart.h"
#include "uart_buffer.h"
#include "ui.h" //for debugging

//XMEGA uart DATA BUFFER
//--Due to the XMEGA architecture, it is not as easy to write a port-agnostic
//		data buffer, so I implemented in this separate file for now.

//==================================
//= State and Storage Variables
//==================================

//TX Queue (outgoing)
volatile uint8_t uart_buffer[MAX_BUFFER_LEN];
volatile uint8_t uart_head;
volatile uint8_t uart_tail;

//RX Queue (incoming)
volatile uint8_t uart_ibuffer[MAX_IBUFFER_LEN];
volatile uint8_t uart_ihead;
volatile uint8_t uart_itail;

//WHICH PORT?!
volatile USART_t* port;

//************************************************************************
//************************************************************************
//** [PORT SPECIFIC CODE]
//************************************************************************
//************************************************************************

//DATA TRANSMIT COMPLETE
SIGNAL(USARTE0_DRE_vect){
	uart_transmit();	
}

//INCOMING DATA INTERRUPT
//	<<NOT IMPLEMENTED YET!!!>>

//************************************************************************
//************************************************************************
//** [PORT AGNOSTIC CODE]
//************************************************************************
//************************************************************************

//==================================
//= TRANSMISSION ENGINE (ISR BASED)
//==================================

//Starts a transmission out of the UART if the UART is ready to receive data
//and we have data to send. (helper function to the ISR so that we can initiate 
//the first transfer
void inline uart_transmit(){
	//keep loading until data register is full or outgoing queue is empty
	while (((port->STATUS & _BV(5)) == B8(00100000)) && (uart_count() > 0)){
		port->DATA = uart_dequeue();
	}

	//else: wait for the next tx complete to empty out the data register
	if(uart_count()>0) uart_txbuffer_enable();
	else uart_txbuffer_disable();
}

void init_uart_buffer(USART_t* which){
	//Setup UART hardware
		port = which;

	//Setup data buffers
		init_uart_obuffer();
		//init_uart_ibuffer(); //not implemented yet!
}

void uart_txbuffer_enable(){
	port->CTRLA = (port->CTRLA | B8(00000010));	//Set the Data Register Empty Interrupt to Medium Priority (timer needs to be higher!)
}

void uart_txbuffer_disable(){
	port->CTRLA = (port->CTRLA & B8(11111100));	//Disable the Data Register Empty Interrupt
}


//***************************************************
/** @defgroup uart_oq Serial Outgoing Data Queue */
/** @{ */
/** Insert from head. Read from tail. The goal is to be fast (very fast) and light.
	No protection is provided for buffer overflow! Be careful! */
//***************************************************

void init_uart_obuffer(void){
	uart_head = 0;
	uart_tail = 0;
}

inline uint8_t uart_count(void){
	if (uart_head >= uart_tail){	
		return (uart_head - uart_tail);
	}
	else {
		return ((MAX_BUFFER_LEN-uart_tail)+uart_head);
	}
}

///Enqueue date into the outgoing serial queue. This data is sent via USB to the PC's first virtual Comm Port associated with the EEICM. 
/**This is the queue used to send data back to the command and control GUI. The #define UART_DEBUG can be used to disable normal serial activity through this queue
	The blue LED is used in this routine to signal buffer overflow, which, due to the real-time scheduled nature of the EEICM firmware architecture, should not happen.
	This function is inactive when in UART DEBUG mode. Calls to this function have no effect during this period.*/
inline void uart_enqueue(uint8_t datain){
#ifndef UART_DEBUG
	uart_buffer[uart_head] = datain;
	uart_head++;
	if (uart_head >= MAX_BUFFER_LEN){
		uart_head = 0;
	}
	uart_transmit(); //start the transmission process.
#endif
}

inline uint8_t uart_dequeue(void){
	uint8_t oldtail;
	oldtail = uart_tail;
	uart_tail++;
	if (uart_tail >= MAX_BUFFER_LEN){
		uart_tail = 0;
	}
	return uart_buffer[oldtail];
}

/** @} */
//****************************************************

//***************************************************
/** @defgroup uart_iq Serial Incoming Data Queue */
/** @{ */
/** Insert from head. Read from tail. The goal is to be fast (very fast) and light.
	No protection is provided for buffer overflow! Be careful! */
//***************************************************

void init_uart_ibuffer(void){
	uart_ihead = 0;
	uart_itail = 0;
}

inline uint8_t uart_icount(void){
	if (uart_ihead >= uart_itail){	
		return (uart_ihead - uart_itail);
	}
	else {
		return ((MAX_IBUFFER_LEN-uart_itail)+uart_ihead);
	}
}

inline void uart_ienqueue(uint8_t datain){
	uart_ibuffer[uart_ihead] = datain;
	uart_ihead++;
	if (uart_ihead >= MAX_IBUFFER_LEN){
		uart_ihead = 0;
	}
}

inline uint8_t uart_idequeue(void){
	uint8_t oldtail;
	oldtail = uart_itail;
	uart_itail++;
	if (uart_itail >= MAX_IBUFFER_LEN){
		uart_itail = 0;
	}
	return uart_ibuffer[oldtail];
}

/** @} */
//****************************************************



