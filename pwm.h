#ifndef __pwm_h
#define __pwm_h

	//SERIAL TRIGGERING
		#define TRIGGER_TARGET "level_restart"
		#define TARGET_LENGTH 13
		uint8_t* trigger_target;
		uint8_t trigger_compare[TARGET_LENGTH];
		void pwm_hunt_target();

	//STATES
		#define POSITIVE_PULSE	87
		#define NEGATIVE_PULSE	88
	
	//DECODING	
		#define TIMEOUT 50 //Number of 2ms periods to wait before reseting state machine
		
	//DECODING STATES
		#define LOOK_FOR_START 130
		#define SAW_A 131
		#define SAW_AB 132
		#define SAW_AC 133
		#define SAW_ABB 134
		#define SAW_ABC 135
		#define SAW_ACB 136
		#define SAW_ACC 137

	//DEFINITIONS
		//Dialed for 0.3uL per pulse delivery
		//2 = 0.3uL (via x10 measurement)
		//25 = 7uL
		#define PWM_PULSE_DURATION 20
	
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
		void pwm_out_high(void);
		void pwm_out_low(void);
		void pwm_pulse(void);
		void pwm_reserved(void);
		void pwm_change_state(uint8_t new_state);
		void pwm_state(uint8_t next_code);
		void service_pwm(void);
		void pwm_polarity(uint8_t polarity);
		void edge_start();
		void edge_stop();
#endif
