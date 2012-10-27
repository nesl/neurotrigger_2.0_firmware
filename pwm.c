#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "ui.h"
#include "ac.h"
#include "dac.h"
#include "uart.h"
#include "uart_buffer.h"
#include "pwm.h"
#include "neurotrig.h"

//#############################################################
//## Handles PWM Demodulation
//#############################################################
//## Hardware Used
//## -------------
//## 	-Event system (Routing Channel 0)
//##	-OVERRIDES Analog Comparator (PORTA-AC0) -- Reconfigures it and disables access to AC0 access to PA7 output pin
//##	-PORTA.7 (Event Generator)
//##	-Timer PORTE.TCC0 (Event Receiver)
//##	-UART PORTE.UART0 (To transmit detected code) -- Goes to FIRST USB VCP
//#############################################################

extern uint8_t STATE_Autolevel;
uint8_t STATE_Pwm_Counter = 0;
uint8_t STATE_Pwm = PWM_OUTPUT_LOW;
uint8_t STATE_Pwm_Polarity = POSITIVE_PULSE;
uint8_t STATE_Pwm_Decode = LOOK_FOR_START;
uint8_t STATE_Pwm_Timeout = TIMEOUT;

//Initialize the PWM module
void init_pwm(){
	//no init needed for pwm functions. We initialize when the mode is activated.
	//but triggering does
	trigger_target = TRIGGER_TARGET;
}

void pwm_enable(){
	//TIMER (PORTD.TC0)
		TCE0.CTRLA = B8(00000101); //Timer Clock source is 32MHz/64; ~130ms Range @ 2uS resolution
		TCE0.CTRLB = 0x00; //Turn off output pins (for both input capture and waveform generation)
		TCE0.CTRLC = 0x00; //Only for the compare output unit
		TCE0.CTRLD = B8(00000000); //Disable Event Unit
		TCE0.CTRLE = 0x00; //Leave the counter in 16 (rather than 8) bit mode

	//DIGITAL-TO-ANALOG CONVERTER (DAC)
		//PORTB.DAC0 -- Vbackground; value set by calibration routine;
		dac_output0(ENABLE);
		dac_out1(2000); //about mid scale
		
	//ANALOG COMPARATOR
		ACA.AC0MUXCTRL = B8(00000011); //Pos. input = PA0; Neg. Input = PA5 (DAC1); 
		//ACA.CTRLB = 20; //VCC Scaler = VCC/2 = 1.65V
		ac_output(DISABLE); //Turn off PA7 output pin (we'll use it directly to control the external peripheral)
		ACA.AC0CTRL = B8(00111101); //enable AC0; 50mV hysterysis; high priority interrupt on edge toggle; high-speed mode
		PORTA.OUTCLR = B8(10000000); //PA7 output low
		PORTA.DIRSET = B8(10000000); //Set PA7 as output (should be anyway)

	//BUTTON
		STATE_Autolevel = AUTOLEVEL_IDLE;
}

void pwm_disable(){
	//TIMER (PORTD.TC0)
		TCE0.CTRLA = 0x00; //Disable Timer

	//DIGITAL-TO-ANALOG CONVERTER (DAC)
		//PORTB.DAC0 -- Vbackground; Enable to allow DAC operation
		dac_output0(DISABLE);

	//ANALOG COMPARATOR
		init_ac();
}

//Returns 'true' if the user configuration switch is set to Audio/PWM mode
boolean pwm_mode(){
	return sw_on(SW1);
}

//ANALOG COMPARATOR (AC0) OUTPUT TOGGLE INTERRUPT
SIGNAL(ACA_AC0_vect){
	//led_toggle(LED_MID);
	if ((ACA.STATUS & B8(00010000)) > 0){
		led_on(LED_MID);
		//Detected: RISING edge
		if (STATE_Pwm_Polarity == POSITIVE_PULSE) edge_start();
		if (STATE_Pwm_Polarity == NEGATIVE_PULSE) edge_stop();
	}
	else {
		led_off(LED_MID);
		//Detected: FALLING edge
		if (STATE_Pwm_Polarity == POSITIVE_PULSE) edge_stop();
		if (STATE_Pwm_Polarity == NEGATIVE_PULSE) edge_start();
	}
}

void inline edge_start(){
	pwm_timer_reset();
}

void inline edge_stop(){
	uint16_t width;
	width = TCE0.CNT;
	if ((TCE0.INTFLAGS & _BV(0)) == 0x00){
		uart_enqueue(13);
		uart_enqueue(10);
		uart_enqueue('-');
		uart_enqueue('-');
		uart_enq_HEX16(width);
		pwm_decode(width);
	}
}

void pwm_polarity(uint8_t polarity){
	STATE_Pwm_Polarity = polarity;
}


//#############################################################
//## TIMER CAPTURE FUNCTIONS
//#############################################################
void pwm_timer_reset(void){
	//Reset the timer effective immediately!
	TCE0.CNTL = 0x00;
	TCE0.CNTH = 0x00;	
	//Clear the overflow flag (actually, clears all flags, but OVIF is the only one we use)
	TCE0.INTFLAGS = 0xFF; 
}

boolean between(uint16_t var, uint16_t low, uint16_t high){
	if ((var > low) && (var < high)) return true;
	else return false;
}

//Converts Pulse Width's into State Machine Changes
	//0x2000 = As.wav
	//0x4000 = Bs.wav
	//0x7000 = Cs.wav
	//0x9000 = Ds.wav
void pwm_decode(uint16_t width){
	
	uart_send_byte(&udata,'-');
	uart_send_byte(&udata,'-');
	uart_send_byte(&udata,'-');
	uart_send_HEX16(&udata, width);
	uart_send_byte(&udata,'-');
	uart_send_byte(&udata, 13);
	
	if (between(width,0x1000,0x2000) == true){
		//Found A code!
		pwm_state('A');		
	}
	if (between(width,0x2000,0x3000) == true){
		//Found B code!
		pwm_state('B');
	}
	if (between(width,0x4000,0x5000) == true){
		//Found C code!
		pwm_state('C');
	}
	if (between(width,0x7000,0x8000) == true){
		//Found D code!
		pwm_state('D');
	}
}

void pwm_out_high(void){
	STATE_Pwm = PWM_OUTPUT_HIGH; //A = Turn On Constantly
	uart_enqueue(' ');
	uart_enqueue('<');
	uart_enqueue('-');
	uart_enqueue('O');
	uart_enqueue('N');
}
void pwm_out_low(void){
	STATE_Pwm = PWM_OUTPUT_LOW; //B = Turn Off Constantly
	uart_enqueue(' ');
	uart_enqueue('<');
	uart_enqueue('-');
	uart_enqueue('O');
	uart_enqueue('F');
	uart_enqueue('F');
}

//Code to initiate an output pulse. Output pulses are only initiated if there is not a current pulse in progress
void pwm_pulse(void){	
	if (STATE_Pwm != PWM_OUTPUT_PULSE){
		STATE_Pwm = PWM_OUTPUT_PULSE; //Update state
		STATE_Pwm_Counter = PWM_PULSE_DURATION; //Load the counter
	}	
	uart_enqueue(' ');
	uart_enqueue('<');
	uart_enqueue('-');
	uart_enqueue('P');
	uart_enqueue('U');
	uart_enqueue('L');
	uart_enqueue('S');
	uart_enqueue('E');
}

void pwm_reserved(void){
	uart_enqueue(' ');
	uart_enqueue('<');
	uart_enqueue('-');
	uart_enqueue('W');
	uart_enqueue('T');
	uart_enqueue('F');
	uart_enqueue('!');
}

void pwm_change_state(uint8_t new_state){
	STATE_Pwm_Decode = new_state;
	STATE_Pwm_Timeout = TIMEOUT;		
}

//State machine to decode more advanced command format
void pwm_state(uint8_t next_code){
	switch (STATE_Pwm_Decode){
		case LOOK_FOR_START:
			if (next_code == 'A') pwm_change_state(SAW_A);
			break;
		case SAW_A:
			switch (next_code){
				case 'A':
				break;
				case 'B':
				pwm_change_state(SAW_AB); break;
				case 'C':
				pwm_change_state(SAW_AC); break;
				default:
				pwm_change_state(LOOK_FOR_START);
			}		
			break;	
		case SAW_AB:
			switch (next_code){
				case 'B':
					pwm_change_state(SAW_ABB); break;
				case 'C':
					pwm_change_state(SAW_ABC); break;
				default:
					pwm_change_state(LOOK_FOR_START);
			}
			break;
		case SAW_AC:
			switch (next_code){
				case 'B':
					pwm_change_state(SAW_ACB); break;
				case 'C':
					pwm_change_state(SAW_ACC); break;
				default:
					pwm_change_state(LOOK_FOR_START);
			}
			break;
		case SAW_ABB:
			if (STATE_Pwm_Decode == 'D') pwm_out_high();
			STATE_Pwm_Decode = LOOK_FOR_START;				
			break;
		case SAW_ABC:
			if (STATE_Pwm_Decode == 'D') pwm_out_low();
			STATE_Pwm_Decode = LOOK_FOR_START;
			break;		
		case SAW_ACB:
			if (STATE_Pwm_Decode == 'D') pwm_pulse();
			STATE_Pwm_Decode = LOOK_FOR_START;
			break;
		case SAW_ACC:
			if (STATE_Pwm_Decode == 'D') pwm_reserved();
			STATE_Pwm_Decode = LOOK_FOR_START;
			break;
		default:
			STATE_Pwm_Decode = LOOK_FOR_START;
	}	
}

//Mainline Loop PWM Service Routine -- use to manage output pulse
//...and trigger states
//---must run only once per 2ms looptime
void service_pwm(void){
	//Process Decoding Timeout
	if (STATE_Pwm_Timeout > 0) STATE_Pwm_Timeout--;
	if (STATE_Pwm_Timeout == 0) STATE_Pwm_Decode = LOOK_FOR_START;
	
	//Process component pulse durations
	switch(STATE_Pwm){
	case PWM_OUTPUT_HIGH:
		PORTA.OUTSET = B8(10000000); //PA7 output high
		break;
	case PWM_OUTPUT_PULSE:
		if (STATE_Pwm_Counter == 0)	{
			//Done with pulse!
			PORTA.OUTCLR = B8(10000000); //PA7 output low
			STATE_Pwm = PWM_OUTPUT_LOW;	//Update State (Done with pulse)
		}
		else {
			STATE_Pwm_Counter--; //Decrement Counter
			PORTA.OUTSET = B8(10000000); //PA7 output high
		}
		break;
	case PWM_OUTPUT_LOW:
	default:
		PORTA.OUTCLR = B8(10000000); //PA7 output low
	}//switch
}



//#############################################################
//## SERIAL TRIGGERING FUNCTIONS
//#############################################################

inline void build_dut(uint8_t start_index){
	for(uint8_t i=0; i<TARGET_LENGTH; i++){
	if (start_index+i >= MAX_IBUFFER_LEN){trigger_compare[i] = uart_ibuffer[start_index+i-MAX_IBUFFER_LEN];}
else {trigger_compare[i] = uart_ibuffer[start_index+i];}
	}	
}

inline uint8_t array_compare(uint8_t* arr1, uint8_t* arr2){
	uint8_t equal = true;
	for(uint8_t i=0; i<TARGET_LENGTH; i++) { if (arr1[i] != arr2[i]){equal = false;} }
	return equal;
}

inline void pwm_hunt_target(){
	uint8_t how_many = uart_icount();
	uint8_t current_index = uart_itail;
	if (how_many < TARGET_LENGTH){return;} //abort if not enough data received
	for (uint8_t i=0; i<=(how_many-TARGET_LENGTH); i++){
		if (current_index >= MAX_IBUFFER_LEN){current_index = 0;}
		build_dut(current_index);
		if (array_compare(trigger_target, trigger_compare) == true){
			pwm_pulse(); //fire off the solenoid
			init_uart_ibuffer(); //flush buffer to prevent retriggering
		}
		current_index++;
	}
}