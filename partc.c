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

int mode = 0;
//mode 0 = nothing
//mode 1 = outputting
//mode 2 = looking for rising edge of input
//mode 3 = looking for falling edge of input
long timeStart = 0;
int numOverflows = 0;
char String[25];

void Initialize() {
	cli(); //disable global interrupts for setup
	
	//will use this pin for both input/output
	DDRB |= (1<<DDB0); //set to output pin (PB0=ICP1=PCINT0 -input capture for timer 1)
	PORTB |= (1<<PORTB0); // set initially to low
	
	//timer 1 setup with pre scaler of 8
	TCCR1B &= ~(1<<CS10);
	TCCR1B |= (1<<CS11);
	TCCR1B &= ~(1<<CS12);
	//need to measure between 116 and 23200 us
	//prescale by 8 so each tick is 0.5us
	//16 * 10^6 / 8 / 2^16 = 30.5 ticks per overflow
	//16 * 10^6 / 8 / 23200 = 86.2 ticks in 23200us
	
	//PCMSK0 |= (1<<PCINT0);//enable pin change interrupt

	sei();//enable global interrupts
}
void output() {
	cli();
	mode = 1;
	DDRB |= (1<<DDB0); //set to output pin
	PORTB |= (1<<PORTB0);
	_delay_us(10);
	PORTB &= ~(1<<PORTB0);
	mode = 2;
	inputSetup();
	sei();
}
void inputSetup() {
	DDRB &= ~(1<<DDB0); //set to input pin
	TCCR1B |= (1<<ICES1);//looking for rising edge
	TIFR1 |= (1<<ICF1);//clear interrupt flag
	TIMSK1 |= (1<<ICIE1);//enable input capture interrupt
}
ISR(TIMER1_CAPT_vect) {
	if (mode == 2) {
		timeStart = ICR1;
		TCCR1B &= ~(1<<ICES1);//looking for falling edge
	} else {
		TIMSK1 &= ~(1<<ICIE1);//disable input capture interrupt
		long overflowTicks = numOverflows * 65536;
		long tickDiff = ICR1 - timeStart + overflowTicks;
		long timeDiff = tickDiff * 8 / 16000000;
		int distanceCM = timeDiff / 58;
		print(distanceCM);
		_delay_ms(10);
		output();
	}
}
ISR(TIMER1_OVF_vect) {
	numOverflows++;
}
int main(void)
{
	Initialize();
	//output();
	print_init();
	while (1) {
		sprintf(String, "hello \n");
		print(String);
		_delay_ms(1000);
	}
}
void print_init() {
	int baudRate = 9600;
	unsigned long Fcpu = 16000000UL;
	long baudPrescaler = (((Fcpu / (baudRate * 16UL))) - 1)
	UBRR0H = (unsigned char)(baudPrescaler >> 8);
}
void print(char* StringToPrint) {
	while (*StringToPrint != 0x00) {
		printChar(*StringToPrint);
		StringToPrint++;
	}
}
void printChar(unsigned char c) {
	//waif for empty transmit buffer
	while (!(UCSR0A & (1<<UDRE0)));
	//send data through buffer
	UDR0 = c;
}
