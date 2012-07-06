#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "ui.h"

//Handles the DAC for PORTB

void init_dac(){
	DACB.CTRLA = B8(00001101); //enable DAC and both output pins
	DACB.CTRLB = B8(01000000); //enable sample-and-hold to allow for independent output operation
	DACB.CTRLC = B8(00001000); //use AVCC as the reference
}

void dac_output0(uint8_t config){
	switch(config){
	case DISABLE:
		DACB.CTRLA = DACB.CTRLA & B8(11111011); //NO DAC output to pin PB2
		break;
	case ENABLE:
	default:
		DACB.CTRLA = DACB.CTRLA | B8(00000100); //DAC output to pin PB2
	}
}

//#############################################################
//## DACs
//#############################################################

//DAC's are 12 bit valued -- numbers outside this range will overflow silently
void dac_out0(uint16_t value){DACB.CH0DATA = value;}
void dac_out1(uint16_t value){DACB.CH1DATA = value;}

void service_dac(){	
}
