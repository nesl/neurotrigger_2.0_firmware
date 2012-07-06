#ifndef __dac_h
#define __dac_h

	//DEFINITIONS

	//GLOBAL STATE

	//FUNCTIONS
		void init_dac();
		void dac_output0(uint8_t config);
		void dac_out0(uint16_t value);
		void dac_out1(uint16_t value);
		void service_dac();
#endif
