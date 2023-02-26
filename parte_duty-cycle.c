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

int duty_cycle = 15;

void Initialize() {
	cli(); //disable global interrupts for setup
	
	DDRD |= (1<<DDD6); //set buzzer to output pin (PD6 = OC0A)
	PORTD |= (1<<PORTD6); // set initially to low
	DDRD |= (1<<DDD5); //set OC0B to output pin
	PORTD |= (1<<PORTD5); // set initially to low
	
	//timer 0 setup, sets pre scaler to 64
	TCCR0B |= (1<<CS00);
	TCCR0B |= (1<<CS01);
	TCCR0B &= ~(1<<CS02);
	
	//set to phase correct PWM with settable TOP
	TCCR0A |= (1<<WGM00);
	TCCR0A &= ~(1<<WGM01);
	TCCR0B |= (1<<WGM02);
	
	//toggle OC0A on compare match
	//OC0A = PD6
	TCCR0A |= (1<<COM0A0);
	TCCR0A &= ~(1<<COM0A1);
	
	//OC0B = 1 on compare match when up counting; OC0B = 0 on comp match when down counting
	//OC0B = PCINT21 - PD5
	TCCR0A |= (1<<COM0B0);
	TCCR0A |= (1<<COM0B1);
  
	OCR0A = 142;//16*10^6 / 64 / (440*2*2) = 142
	OCR0B = OCR0A * (1.0 - (duty_cycle/100.0)); //=OCR0A*(1-duty_cycle) = 120
	sei();//enable global interrupts
}

int main(void) {
	Initialize();
	while (1);
}
