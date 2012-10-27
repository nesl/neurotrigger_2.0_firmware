#ifndef __uartbuff_h
#define __uartbuff_h

	//DATA BUFFERS
	#define MAX_BUFFER_LEN 250
	#define MAX_IBUFFER_LEN 250

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

	//FUNCTIONS
	
	void service_uart_buffer();
	
	void uart_transmit();
	void uart_receive();
	void init_uart_buffer(USART_t* which);

	void init_uart_obuffer(void);
	uint8_t uart_count(void);
	void uart_enqueue(uint8_t datain);
	void uart_enqueue_string(char* string_in);
	uint8_t uart_dequeue(void);

	void init_uart_ibuffer(void);
	uint8_t uart_icount(void);
	void uart_ienqueue(uint8_t datain);
	uint8_t uart_idequeue(void);
	
	inline void uart_print_ibuffer();

	void uart_txbuffer_enable();
	void uart_txbuffer_disable();	
	void uart_rxbuffer_enable();
	void uart_rxbuffer_disable();
#endif
