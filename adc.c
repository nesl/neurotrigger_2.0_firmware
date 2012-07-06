#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "ui.h"
#include "adc.h"

//Handles the ADC

void init_adc(){
	ADCA.REFCTRL = B8(00010000); //Use AVCC/1.6 = 2.063V as reference (highest allowed)
	ADCA.CTRLB = B8(00000000); //12bit-right adjusted; One-shot conversion
	ADCA.CTRLA = B8(00000001); //Enable ADC;
	ADCA.CH0.MUXCTRL = B8(00010000); //CH0 converts from PA2
	ADCA.CH1.MUXCTRL = B8(00011000); //CH1 converts from PA3
}

//#############################################################
//## ADCs
//#############################################################

//Performs a single 12-bit conversion on ADC CH0
//BLOCKS UNTIL CONVERSION COMPLETE!
uint16_t adc_ch0(){
	ADCA.CH0.CTRL = B8(10000001); //Single ended input mode; No gain
	while(ADCA.CH0.INTFLAGS == 0x00); //wait for conversion to complete
	ADCA.CH0.INTFLAGS = 0x01; //clear the conversion complete flag
	return ADCA.CH0.RES;
}

//Performs a single 12-bit conversion on ADC CH1
//BLOCKS UNTIL CONVERSION COMPLETE!
uint16_t adc_ch1(){
	ADCA.CH1.CTRL = B8(10000001); //Single ended input mode; No gain
	while(ADCA.CH1.INTFLAGS == 0x00); //wait for conversion to complete
	ADCA.CH1.INTFLAGS = 0x01; //clear the conversion complete flag
	return ADCA.CH1.RES;
}

//Average num_to_avg samples and return the result
//Pass in only power-of-2 integers; Max is 2^20;
//BLOCKS UNTIL ALL CONVERSIONS COMPLETE! USE WITH CAUTION!
//Updates the global adc_stats variables (adc_avg, adc_max, etc)
void adc_ch0_stats(uint16_t num_to_avg){
	uint16_t intermediate;
	uint32_t sum = 0;
	uint16_t i = 0;
	adc_max = 0;
	adc_min = 0xFFFF;
	while(i < num_to_avg){
		intermediate = adc_ch0();
		sum += intermediate; //for averaging
		if (intermediate < adc_min) adc_min = intermediate; //check min
		if (intermediate > adc_max) adc_max = intermediate; //check max
		i++;
	}
	adc_count = num_to_avg;
	adc_avg = sum / num_to_avg;
}

//Average num_to_avg samples and return the result
//Pass in only power-of-2 integers; Max is 2^20?;
//BLOCKS UNTIL ALL CONVERSIONS COMPLETE! USE WITH CAUTION!
//Updates the global adc_stats variables (adc_avg, adc_max, etc)
void adc_ch1_stats(uint16_t num_to_avg){
	uint16_t intermediate;
	uint32_t sum = 0;
	uint16_t i = 0;
	adc_max = 0;
	adc_min = 0xFFFF;
	while(i < num_to_avg){
		intermediate = adc_ch1();
		sum += intermediate; //for averaging
		if (intermediate < adc_min) adc_min = intermediate; //check min
		if (intermediate > adc_max) adc_max = intermediate; //check max
		i++;
	}
	adc_count = num_to_avg;
	adc_avg = sum / num_to_avg;
}

void service_adc(){	
}
