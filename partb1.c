/*
 * ESE350_lab2_1.c
 *
 * Created: 1/27/2023 11:13:29 AM
 * Author : erica
 */ 

#include <stdlib.h>
#include <stdio.h>

#include <util/delay.h>

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

void Initialize() {
	//cli(); //disable global interrupts for setup
	PORTB &= ~(1<<PORTB5); // set to 0
	
	//PB1 pin 6 - buzzer output
	DDRD |= (1<<DDD6); //set to output pin
	PORTD |= (1<<PORTD6); // set to low
	
	//PB5 - onboard LED
	DDRB |= (1<<DDB5); //set to output pin
	//PORTB &= ~(1<<PORTB5); // set to l
	
	//timer setup
	TCCR0B |= (1<<CS00);
	TCCR0B &= ~(1<<CS01);
	TCCR0B |= (1<<CS02);
	//sets prescaler to 1024
	
	TCCR0A &= ~(1<<WGM00);
	TCCR0A &= ~(1<<WGM01);
	TCCR0B &= ~(1<<WGM02);
	
	//TIMSK0 |= (1<<OCIE0A);//enables output compare
	
	//enables interrupt for timer overflow
	TIMSK0 |= (1<<TOIE0);
	TIFR0 |= (1<<TOV0);//set interrupt flag to 1
	
	sei();//enable global interrupts
	
	
}

ISR(TIMER0_OVF_vect) {
	PORTD ^= (1<<PORTD6);
	//TIFR0 |= (1<<TOV1);//set interrupt flag to 1
}

int main(void)
{
	DDRB |= (1<<DDB5); //set to output pin
	PORTB |= (1<<PORTB5); // set to l
	Initialize();
	
	while (1) {
		PORTB &= ~(1<<PORTB5); //on board
		_delay_ms(10000);
		PORTB |= (1<<PORTB5);
		_delay_ms(10000);
	}
}
