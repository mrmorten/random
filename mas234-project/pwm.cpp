/*
 * pwm.cpp
 *
 * Created: 19.11.2020 10.01.21
 * Author : Morten
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
double dutyCycle = 0;


int main(void)
{
	DDRD |= (1<<PD6); // set port D6, the pin for OC0A
	TCCR0A |= (1<<COM0A1)|(1<<WGM00)|(1<<WGM01); //COM0A1 to clear OC0A on compare match, set OC0A at bottom
	// WGM00 and WGM01 fast pwm mode, TOV flag set on max 0xFF 255
	TIMSK0 |= (1<<TOIE0); // choose timer overflow
	
	OCR0A = (dutyCycle/100)*255;
	sei();
	TCCR0B |= (1<<CS00); //| (1<<CS01);  //set prescalers, now 1

	while(1)
	{
		for (int i = 0; i <=100; i+=2)
		{
			dutyCycle ++;
			_delay_ms(0.005);
		}
		for (int i = 100; i>= 0; i-=2)
		{
			dutyCycle--;
			_delay_ms(0.005);
		}
	}
}
ISR(TIMER0_OVF_vect){
	OCR0A = (dutyCycle/100)*255;
}

