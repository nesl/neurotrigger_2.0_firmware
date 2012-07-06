#ifndef __uart_h
#define __uart_h

	//PCB SPECIFIC DEFINITIONS
	#define uctrl	USARTE0
	#define udata	USARTD1

	//BAUD RATES
	#define BAUD_115200 0
	#define BAUD_57600	1
	#define BAUD_38400	2

	//FUNCTIONS
	void init_uart(USART_t* which, uint8_t baud_rate);
	void uart_send_byte(USART_t* which, unsigned char dataB);
	void uart_send_BIN4(USART_t* which, uint8_t lowb);
	void uart_send_BIN8(USART_t* which, uint8_t lowb);
	void uart_send_HEX8(USART_t* which, uint8_t lowb);
	void uart_send_HEX16b(USART_t* which, uint8_t highb, uint8_t lowb);
	void uart_send_HEX16(USART_t* which, uint16_t highb);
	void uart_enq_BIN4(uint8_t lowb);
	void uart_enq_BIN8(uint8_t lowb);
	void uart_enq_HEX8(uint8_t lowb);
	void uart_enq_HEX16b(uint8_t highb, uint8_t lowb);
	void uart_enq_HEX16(uint16_t highb);

#endif
