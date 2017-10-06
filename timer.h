#include <avr/io.h>
#include <avr/interrupt.h>


#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define TIMER1_CNT 15000
#define TCNT_INIT 49910
#define MAXSTR 8

#include <util/delay.h>
#include <stdlib.h>

enum { NOT_PUSHED, NEW_MAYBE, JUST_PUSHED, OLD_MAYBE, OLD_PUSHED }; /* debouncing */
enum { RESET, COUNTDOWN };  /* timer states */

volatile int hit = 0;
volatile unsigned char ready_to_advance;
volatile unsigned char state;
volatile unsigned char PushState;

void interrupt_init()
{	
	// TODO comment what these lines do so I can actually remember 
	
	/* Initialize timer interrupt registers */
	TCCR1A = 0;
	TCCR1B = (1<<CS10) | (1<<CS12);
	TIMSK1 = _BV(TOIE1);
	TCNT1 = TCNT_INIT;

	/* Initialize pushbutton interrupt registers */
	TCCR0A = 0;
	TCCR0B = (1<<CS00) | (1<<CS02);
	TIMSK0 = _BV(OCIE0A);
	TCNT0 = 0;
	OCR1A = 15;
}



void LcdCommandWrite_UpperNibble( unsigned char cm )
{
	PORTC = (PORTC & 0xf0) | (cm >> 4); // DB4-DB7
	PORTC &= ~(1<<4); // RS = 0
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
}

void LcdCommandWrite_LowerNibble( unsigned char cm )
{
	PORTC = (PORTC & 0xf0) | (cm & 0x0f); // DB4-DB7
	PORTC &= ~(1<<4); // RS = 0
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
}

void LcdCommandWrite( unsigned char cm )
{
	// TODO: split this into upper and lower nibble function calls
	
	//LcdCommandWrite_UpperNibble(cm);
	//LcdCommandWrite_LowerNibble(cm);
	PORTC = (PORTC & 0xf0) | (cm >> 4);
	PORTC &= ~(1<<4); // RS = 0
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
	// Write lower nibble
	PORTC = (PORTC & 0xf0) | (cm & 0x0f);
	PORTC &= ~(1<<4); // RS = 0
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
}

void LcdDataWrite( unsigned char cm )
{
	// TODO: split this into upper and lower nibble function calls
	
	// Write upper nibble
	PORTC = (PORTC & 0xf0) | (cm >> 4);
	PORTC |= 1<<4; // RS = 1
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
	// Write lower nibble
	PORTC = (PORTC & 0xf0) | (cm & 0x0f);
	PORTC |= 1<<4; // RS = 1
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
}

int lcd_print(char *word)
{
	int i = 0;
	for( ; word[i] != '\0'; ++i)
		LcdDataWrite(word[i]);
	return i;
}

void Lcd_init()
{
	DDRC = 0x3f;
	LcdCommandWrite_UpperNibble(0x30);
	_delay_ms(4.1);
	LcdCommandWrite_UpperNibble(0x30);
	_delay_us(100);
	LcdCommandWrite_UpperNibble(0x30);
	LcdCommandWrite_UpperNibble(0x20);
	
	LcdCommandWrite(0x28);
	// function set: 0x28 means, 4-bit interface, 2 lines, 5x8 font
	LcdCommandWrite(0x08);
	// display control: turn display off, cursor off, no blinking
	LcdCommandWrite(0x01);
	// clear display, set address counter to zero
	LcdCommandWrite(0x06); // entry mode set:
	LcdCommandWrite(0x0c); // display on, cursor on, cursor blinking
	_delay_ms(120);
}


void advance_state()
{
	// TODO allow change state to any state...
	if( ++state > COUNTDOWN )
		state = RESET;
		
	ready_to_advance = 0;
}

/*

int main(void)
{
    Lcd_init();
	interrupt_init();
	sei();

	uint8_t t = 0;
	uint8_t neonate = 1;
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
				if(neonate)
				{
					t = 0;
					LcdCommandWrite(0x01);
					LcdDataWrite('0');
					neonate = 0;
				}
				break;
			case COUNTDOWN:
				neonate = 1;
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

*/


////////////////////////////////
// Interrupt service routines //
////////////////////////////////

/* Interrupt that triggers twice per second */
ISR(TIMER1_OVF_vect)
{
	// TODO: change TCNT1 to be only half of a second.
	
	hit = 1;
	TCNT1 = 49910;
}


/* Button pressed */
ISR(TIMER0_COMPA_vect)
{
	// TODO: properly note which state to go to next...
	switch(PushState)
	{
		case NOT_PUSHED:
			if(PIND & 1<<PIND3)
				PushState = NEW_MAYBE;
			else
				PushState = NOT_PUSHED;
			break;
		case NEW_MAYBE:
			if(PIND & 1<<PIND3)
				PushState = JUST_PUSHED;
			else
				PushState = NOT_PUSHED;
			break;
		case JUST_PUSHED:
			ready_to_advance = 1;
			if(PIND & 1<<PIND3)
				PushState = OLD_PUSHED;
			else
				PushState = OLD_MAYBE;
			break;
		case OLD_PUSHED:
			if(PIND & 1<<PIND3)
				PushState = OLD_PUSHED;
			else
				PushState = OLD_MAYBE;
			break;
		case OLD_MAYBE:
			if(PIND & 1<<PIND3)
				PushState = OLD_PUSHED;
			else
				PushState = NOT_PUSHED;
	}
}

