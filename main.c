#include <avr/io.h>
#include "buzzer.h"
#include "timer.h"

void init_buzzer(void)
{
	pwm_init();
	adc_init();
}


int main(void)
{
	/* TODO: 
		Read the datasheet again...
		
		Count DOWN instead of UP
		Reimplement the states
		Convert from seconds to mm:ss
		Hard code 8 minutes ( How long it takes to cook a red baron pizza at 350 Fahrenheit :) )
		
		Think of how to connect the buzzer and the timer...
			o Call buzz here on correct state, end the buzz on RESET.
		
		Call adjust buzzer when timer expires
			o Also adjust buzzer when button pressed while in beeping state
		Unimplement potentiometer
		
	*/
	
	init_buzzer();
	Lcd_init();
	interrupt_init();
	sei();  // Forgot what this is??? 

	uint8_t t = 0; // seconds remaining
	uint8_t is_reset = 0;
	char tstr[MAXSTR];
	PushState = NOT_PUSHED;
	state = RESET;
	
	while (1)
	{
		if( ready_to_advance )
		{
			advance_state();
		}
		switch(state)
		{
			case RESET:
				if(!is_reset) /* only reset the LCD once */
				{
					t = 0;
					LcdCommandWrite(0x01);
					LcdDataWrite('0');
					is_reset = 1;
				}
				break;
			case COUNTDOWN:
				is_reset = 0;
				if(hit)
				{
					LcdCommandWrite(0x01);
					itoa(t,tstr,MAXSTR);
					lcd_print(tstr);
					t++;
					hit = 0;
				}
				break;
			default:
				break;
		}
	}
}

