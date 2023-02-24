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
	
	DDRD |= (1<<DDD6); //set buzzer to output pin (PD6 = OC0A)
	PORTD |= (1<<PORTD6); // set initially to low
	
	//timer 0 setup, sets pre scaler to 256
	TCCR0B |= (1<<CS00);
	TCCR0B |= (1<<CS01);
	TCCR0B &= ~(1<<CS02);
	
	//set to phase correct PWM with settable TOP
	TCCR0A |= (1<<WGM00);
	TCCR0A &= ~(1<<WGM01);
	TCCR0B |= (1<<WGM02);
	
	//toggle OC1A on compare match
	TCCR0A |= (1<<COM0A0);
	TCCR0A &= ~(1<<COM0A1);
  
	OCR0A = 142;16*10^6 / 64 / (440*2*2) = 142
	sei();//enable global interrupts
}

int main(void) {
	Initialize();
	while (1);
}
