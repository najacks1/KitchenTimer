#include <avr/io.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <util/delay.h>
#include <stdlib.h>


// Initialize pulse width modulation.
void pwm_init()
{	
	TCCR0A = (1<<WGM01) | (1<<WGM00) | (1<<COM0B1);
	TCCR0B = (1<<CS00)  | (1<<CS01)  | (1<<WGM02);
	TIMSK0 = (1<<OCIE0A) | (1<<OCIE0B);
	TCNT0 = 0;
	OCR0A = 255;
	OCR0B = 25;
}




void adc_init()
{
	DDRC &= 0x00; // PC1 = ADC 1 as input
	DDRD = (1<<DDD5);
	
	// ADLAR set to 1 -> left adjusted, will ignore 2 least sig bits
	// MUX3:0 set to 0001 -> input voltage at ADC1
	ADMUX = (1<<ADLAR) | (1<<MUX0) | (1<<REFS0);
		
	// ADEN set to 1 -> enable ADC
	// ADPS2:0 set to 111 -> 128 prescalar
	ADCSRA = (1<<ADEN) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
}

unsigned char GetADC()
{
	// start conversion
	ADCSRA |= (1<<ADSC);
	
	while( ADCSRA & (1<<ADSC));
	
	return ADCH;
	
}


unsigned char adc_to_hz(unsigned char c)
{
	// 120 = 1048 Hz
	// ...
	// 240 = 2096 Hz
	return 120 + (c*120)/(255);
}

void adjust_buzzer()
{
	unsigned char c = GetADC();
	// change frequency
	OCR0A = adc_to_hz(c);
}


/*
int main(void)
{
	pwm_init();
	adc_init();
	
    while (1) 
    {
		adjust_buzzer();
    }
}
*/