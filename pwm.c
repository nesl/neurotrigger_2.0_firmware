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

//Initialize the PWM module
void init_pwm(){
	//no init needed. We initialize when the mode is activated.
}

void pwm_enable(){
	//TIMER (PORTD.TC0)
		TCE0.CTRLA = B8(00000101); //Timer Clock source is 32MHz/64; ~130ms Range @ 2uS resolution
		TCE0.CTRLB = 0x00; //Turn off output pins (for both input capture and waveform generation)
		TCE0.CTRLC = 0x00; //Only for the compare output unit
		TCE0.CTRLD = B8(00000000); //Disable Event Unit
		TCE0.CTRLE = 0x00; //Leave the counter in 16 (rather than 8) bit mode

	//DIGITAL-TO-ANALOG CONVERTER (DAC)
		//PORTB.DAC0 -- Vbackground; disable so GPIO can take over and set to ground (prevents us from having to disturb the current DAC output value so we can easily restore it later if need be)
		dac_output0(DISABLE);
		//Output Low on PB2
		PORTB.OUTCLR = B8(00000100);
		PORTB.DIRSET = B8(00000100);

	//ANALOG COMPARATOR
		ACA.AC0MUXCTRL = B8(00000111); //Pos. input = PA0; Neg. Input = VCC Scaler; 
		ACA.CTRLB = 20; //VCC Scaler = VCC/2 = 1.65V
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
		dac_output0(ENABLE); //Overrides PB2 GPIO function

	//ANALOG COMPARATOR
		init_ac();
}

//Returns 'true' if the user configuration switch is set to Audio/PWM mode
boolean pwm_mode(){
	return sw_on(SW1);
}

//ANALOG COMPARATOR (AC0) OUTPUT TOGGLE INTERRUPT
SIGNAL(ACA_AC0_vect){
	led_toggle(LED_MID);
	if ((ACA.STATUS & B8(00010000)) > 0){
		//Detected: RISING edge
		if (STATE_Pwm_Polarity == POSITIVE_PULSE) edge_start();
		if (STATE_Pwm_Polarity == NEGATIVE_PULSE) edge_stop();
	}
	else {
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
	static uint8_t last_code = 0;

	if (between(width,0x1000,0x2000) == true){
		//Found A code!
		STATE_Pwm = PWM_OUTPUT_HIGH; //A = Turn On Constantly
		last_code = 1; //Note that we've seen this code (A=1, B=2, etc)
		uart_enqueue(' ');
		uart_enqueue('<');
		uart_enqueue('-');
		uart_enqueue('A');
	}
	if (between(width,0x2000,0x3000) == true){
		//Found B code!
		STATE_Pwm = PWM_OUTPUT_LOW; //B = Turn Off Constantly
		last_code = 2; //Note that we've seen this code (A=1, B=2, etc)
		uart_enqueue(' ');
		uart_enqueue('<');
		uart_enqueue('-');
		uart_enqueue('B');
	}
	if (between(width,0x4000,0x5000) == true){
		//Found C code!
		if (last_code != 3) pwm_pulse(); //C = Pulse for first C code encountered
		last_code = 3; //Note that we've seen this code (A=1, B=2, etc)
		uart_enqueue(' ');
		uart_enqueue('<');
		uart_enqueue('-');
		uart_enqueue('C');
	}
	if (between(width,0x7000,0x8000) == true){
		//Found D code!
		if (last_code != 4) pwm_pulse(); //D = Pulse for first D code encountered
		last_code = 4; //Note that we've seen this code (A=1, B=2, etc)
		uart_enqueue(' ');
		uart_enqueue('<');
		uart_enqueue('-');
		uart_enqueue('D');
	}
}

//Code to initiate an output pulse. Output pulses are only initiated if there is not a current pulse in progress
void pwm_pulse(void){
	if (STATE_Pwm != PWM_OUTPUT_PULSE){
		STATE_Pwm = PWM_OUTPUT_PULSE; //Update state
		STATE_Pwm_Counter = PWM_PULSE_DURATION; //Load the counter
	}
}

//Mainline Loop PWM Service Routine -- use to manage output pulse
//...and trigger states
//---must run only once per 2ms looptime
void service_pwm(void){
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
