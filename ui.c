#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "ui.h"
#include "ac.h"
#include "pwm.h"

//Handles all UI hardware (buttons, switches, and lights)

void init_ui(){
	//LED's are located at PD0 (Middle) and PD4 (Left)
	PORTD.DIRSET = 0x11; //pins 0 and 4 to output
	PORTD.OUTSET = 0x11; //pins 0 and 4 to high (off)
	PORTD.PIN0CTRL = B8(01000000); //Invert the pin (needed to achieve correct PWM output polarity)
	PORTD.PIN4CTRL = B8(01000000); //Invert the pin (needed to achieve correct PWM output polarity)
	TCD0.CTRLA = 0x07; //enable; div1024
	TCD0.CTRLB = 0x13; //Output Channel A enable; Single-slope PWM
	TCD0.PER = 0x00FF; //Set the top of the counter to basically force 8 bit operation; we do this for speed when calling dimming functions in the future
	TCD0.CCA = 0x0080; //Default to off-level brightness
	TCD1.CTRLA = 0x07; //enable; div1024
	TCD1.CTRLB = 0x13; //Output Channel A enable; Single-slope PWM
	TCD1.PER = 0x00FF; //Set the top of the counter to basically force 8 bit operation; we do this for speed when calling dimming functions in the future
	TCD1.CCA = 0x0010; //Default to off-level brightness

	//Switches need pull-up resistance
	//	Switches: PC5 = SW1; PC4 = SW2; PC3 = SW3
	PORTC.DIRCLR = B8(00111100); //This is the default condition, but just to be safe
	PORTC.PIN5CTRL = B8(10011000); //Slew rate limiter on; Internal pull-up on; Sense on both input edges
	PORTC.PIN4CTRL = B8(10011000); //Slew rate limiter on; Internal pull-up on; Sense on both input edges
	PORTC.PIN3CTRL = B8(10011000); //Slew rate limiter on; Internal pull-up on; Sense on both input edges	
	//Button requirements
	//	Button Interrupt Controls
	//	Button: PC2 = Button0
	#define RISING_EDGE 	B8(10011001) //Slew rate limiter on; Internal pull-up on; Sense on rising input edges
	#define FALLING_EDGE 	B8(10011010) //Slew rate limiter on; Internal pull-up on; Sense on falling input edges
	PORTC.INT0MASK = B8(00000100); //Enable interrupt0 channel for PC2 
	PORTC.INTCTRL = B8(00000011); //interrupt0 channel set to high priority
	PORTC.PIN2CTRL = FALLING_EDGE; 
	STATE_Button = BUTTON_IDLE;
}

//#############################################################
//## LEDs
//#############################################################

void led_on(uint8_t which){led_dim(which, 0xff);}

void led_off(uint8_t which){led_dim(which, 0x00);}

void led_dim(uint8_t which, uint8_t brightness){
	switch(which){
	case LED_LEFT:
		TCD1.CCABUF = (uint16_t)brightness;		
		break;
	case LED_MID:
		TCD0.CCABUF = (uint16_t)brightness;		
		break;
	}
}

void led_toggle(uint8_t which){
	switch(which){
	case LED_LEFT:
		TCD1.CCAL = ~TCD1.CCAL;		
		break;
	case LED_MID:
		TCD0.CCAL = ~TCD0.CCAL;		
		break;
	}
}

void service_leds(){

}

//#############################################################
//## BUTTON -- Interrupt driven
//#############################################################

//Caution with programatic use as I do not implement switch debouncing
SIGNAL(PORTC_INT0_vect){
	if (PORTC.PIN2CTRL == FALLING_EDGE){
		//Just detected a falling edge (button has been pressed in)
		PORTC.PIN2CTRL = RISING_EDGE;
		STATE_Button = BUTTON_PUSHED;
	}
	else {
		//Just detected a rising edge (button has been released)
		PORTC.PIN2CTRL = FALLING_EDGE;
		STATE_Button = BUTTON_RELEASED;
	}
}

void service_button(){
	if (STATE_Button == BUTTON_PUSHED){
		//Actions when the button is pushed and held down
	}
	else {
		//Actions when the button is not pressed or held
	}
}

//#############################################################
//## SWITCHESs -- Polling only
//#############################################################

//	Switches: PC5 = SW1; PC4 = SW2; PC3 = SW3
boolean sw_on(uint8_t which){
	switch(which){
	case SW1:
		if ((PORTC.IN & _BV(5)) == 0x00){
			//Switch is closed (on)
			return true;
		}
		else {
			//Switch is open (off)
			return false;
		}
		break;
	case SW2:
		if ((PORTC.IN & _BV(4)) == 0x00){
			//Switch is closed (on)
			return true;
		}
		else {
			//Switch is open (off)
			return false;
		}
		break;
	case SW3:
		if ((PORTC.IN & _BV(3)) == 0x00){
			//Switch is closed (on)
			return true;
		}
		else {
			//Switch is open (off)
			return false;
		}
		break;
	default:
		return false;
	}
}

void service_switches(){
	static uint8_t mode = 0; //Store prior MODE state

	//[Switch 1] Audio vs. Photodiode Mode -- use pwm.c / pwm_mode() to test condition
	if (sw_on(SW1) == true){
		if (mode != MODE_AUDIO){
			//Just changed modes into AUDIO mode!
			mode = MODE_AUDIO; //update state
			pwm_enable(); //turn on AUDIO mode interrupts and configure!
		}
	}
	else {
		if (mode != MODE_PHOTO){
			//Just changed modes into PHOTO mode!
			mode = MODE_PHOTO; //update state
			pwm_disable();
		}
	}
	//[Switch 2] Output polarity
	if (sw_on(SW2) == true){
		if (pwm_mode() == false) ac_polarity(IDLE_HIGH);
		else pwm_polarity(NEGATIVE_PULSE);
	}
	else {
		if (pwm_mode() == false) ac_polarity(IDLE_LOW);
		else pwm_polarity(POSITIVE_PULSE);
	}
	//[Switch 3] Hysteresis enable
	if (sw_on(SW3) == true){
		if (pwm_mode() == false) ac_hysteresis(LARGE);
	}
	else {
		if (pwm_mode() == false) ac_hysteresis(NONE);
	}
}
