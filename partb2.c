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
	cli(); //disable global interrupts for setup
	//PB1 pin 6 - buzzer output
	DDRD |= (1<<DDD6); //set to output pin
	PORTD |= (1<<PORTD6); // set to low
	//timer 0 setup
	TCCR0B &= ~(1<<CS00);
	TCCR0B &= ~(1<<CS01);
	TCCR0B |= (1<<CS02);
	//sets prescaler to 1024
	//16^6 / 1024 / 440 * 2 = 71
	TIMSK0 |= (1<<OCIE0A);//enables output compare
	OCR0A = 71;
	sei();//enable global interrupts
}
ISR(TIMER0_COMPA_vect) {
	PORTD ^= (1<<PORTD6);
	OCR0A = (OCR0A + 71) % 256;
}
int main(void) {
	Initialize();
	while (1);
}
