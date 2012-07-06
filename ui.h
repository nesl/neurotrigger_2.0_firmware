#ifndef __ui_h
#define __ui_h

	//DEFINITIONS
		#define LED_LEFT	0
		#define LED_MID		1
		#define SW1		0
		#define SW2		1
		#define SW3		2

	//GLOBAL STATE
		uint8_t STATE_Button;
		#define BUTTON_IDLE		0
		#define BUTTON_PUSHED	1
		#define BUTTON_RELEASED 2

	//MODES
		#define MODE_AUDIO	53
		#define MODE_PHOTO	54
		

	//FUNCTIONS
		void init_ui();
		void led_on(uint8_t which);
		void led_off(uint8_t which);
		void led_toggle(uint8_t which);
		void led_dim(uint8_t which, uint8_t brightness);
		boolean sw_on(uint8_t which);
		void service_switches();
		void service_leds();
		void service_button();
#endif
