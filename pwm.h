#ifndef __pwm_h
#define __pwm_h

	//STATES
		#define POSITIVE_PULSE	87
		#define NEGATIVE_PULSE	88

	//DEFINITIONS
		#define PWM_PULSE_DURATION 100
	
	//GLOBAL STATE
		#define PWM_OUTPUT_LOW 78
		#define PWM_OUTPUT_HIGH 79
		#define PWM_OUTPUT_PULSE 80

	//FUNCTIONS
		void init_pwm();
		void pwm_timer_reset(void);
		boolean pwm_mode();
		void pwm_enable(void);
		void pwm_disable(void);
		void pwm_decode(uint16_t width);
		void pwm_pulse(void);
		void service_pwm(void);
		void pwm_polarity(uint8_t polarity);
		void edge_start();
		void edge_stop();
#endif
