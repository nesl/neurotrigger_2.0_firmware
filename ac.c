#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "ui.h"
#include "ac.h"

//Handles the Analog Comparator

void init_ac(){
	//hysterysis options: 0mV, 20mV, 50mV
	ACA.AC0CTRL = B8(00001101); //enable AC0; 50mV hysterysis; no interrupts; high-speed mode
	ACA.AC0MUXCTRL = B8(00000011); //Pos. input = PA0; Neg. Input = PA5; 
	ac_output(ENABLE); //AC output to pin PA7;
}

void ac_output(uint8_t config){
	switch(config){
	case DISABLE:
		ACA.CTRLA = 0x00; //NO AC output to pin PA7;
		break;
	case ENABLE:
	default:
		ACA.CTRLA = 0x01; //AC output to pin PA7;	
	}
}

//#############################################################
//## ACs
//#############################################################

void ac_hysteresis(uint8_t amount){
	switch(amount){
	case LARGE:
		ACA.AC0CTRL = B8(00001101); //enable AC0; 50mV hysterysis; no interrupts; high-speed mode
		break;
	case SMALL:
		ACA.AC0CTRL = B8(00001011); //enable AC0; 20mV hysterysis; no interrupts; high-speed mode
		break;
	case NONE:
	default:
		ACA.AC0CTRL = B8(00001001); //enable AC0; no hysterysis; no interrupts; high-speed mode		
	}
}

//Sets output polarity (e.g. active high or active low output)
void ac_polarity(uint8_t polarity){
	//reverse the comparator inputs to reverse the output polarity
	//pin I/O functions are overridden by the AC unit so I/O inversion is ineffective
	if (polarity == IDLE_HIGH){
		ACA.AC0MUXCTRL = B8(00101000); //Pos. input = PA5; Neg. Input = PA0; 
	}
	else {
		ACA.AC0MUXCTRL = B8(00000011); //Pos. input = PA0; Neg. Input = PA5; 
	}
}

void service_ac(){	
}
