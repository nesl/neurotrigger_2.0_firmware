#ifndef __adc_h
#define __adc_h

	//DEFINITIONS
		uint16_t adc_avg;
		uint16_t adc_max;
		uint16_t adc_min;
		uint8_t adc_count;

	//GLOBAL STATE

	//FUNCTIONS
		void init_adc();
		void adc_ch0_stats(uint16_t num_to_avg);
		uint16_t adc_ch0();
		void adc_ch1_stats(uint16_t num_to_avg);
		uint16_t adc_ch1();
		void service_adc();
#endif
