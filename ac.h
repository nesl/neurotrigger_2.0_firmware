#ifndef __ac_h
#define __ac_h

	//DEFINITIONS
		//Hysteresis
		#define NONE	0 //for XMEGA A4 series parts = 0mV
		#define SMALL	1 //for XMEGA A4 series parts = 20mV
		#define LARGE	2 //for XMEGA A4 series parts = 50mV

		//Output Polarity
		#define IDLE_HIGH	1
		#define IDLE_LOW	2

	//GLOBAL STATE

	//FUNCTIONS
		void init_ac();
		void ac_output(uint8_t config);
		void ac_hysteresis(uint8_t amount);
		void ac_polarity(uint8_t polarity);
		void service_ac();
#endif
