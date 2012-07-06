#ifndef __uartbuff_h
#define __uartbuff_h

	//DATA BUFFERS
	#define MAX_BUFFER_LEN 250
	#define MAX_IBUFFER_LEN 10

	//FUNCTIONS
	void uart_transmit();
	void init_uart_buffer(USART_t* which);

	void init_uart_obuffer(void);
	uint8_t uart_count(void);
	void uart_enqueue(uint8_t datain);
	uint8_t uart_dequeue(void);

	void init_uart_ibuffer(void);
	uint8_t uart_icount(void);
	void uart_ienqueue(uint8_t datain);
	uint8_t uart_idequeue(void);

	void uart_txbuffer_enable();
	void uart_txbuffer_disable();	
#endif
