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

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

//music in 1/2 note increments, -1 is change of note
int music[61] = {0, 0, 0, -1, 0, 1, 1, 0, 0, 3, 3, 2, 2, 2, 2, 0, 0, 0, -1, 0, 1, 1, 0, 0, 4, 4, 3, 3, 3, 3, 0, 0, 0, -1, 0, 7, 7, 5, 5, 3, 3, 2, 2, 1, 1, 6, 6, 6, -1, 6, 5, 5, 3, 3, 4, 4, 3, 3, 3, 3, -1};
int notes[8] = {239, 213, 190, 179, 159, 142, 127, 119};//middle C, D, E, F, G, A, B, C - OCR0A values


int mode = 0;
int x = 0;
//mode 0 = nothing
//mode 1 = outputting
//mode 2 = looking for rising edge of input
//mode 3 = looking for falling edge of input
long timeStart = 0;
long timeEnd = 0;
int numOverflows = 0;
char String[25];
int distanceCM = 0;

void print_init() {
	sprintf(String, "print_init \n");
	print(String);
	UBRR0H = (unsigned char)(BAUD_PRESCALER >> 8);
	UBRR0L = (unsigned char)BAUD_PRESCALER;
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
	UCSR0C |= (1<<USBS0);
}
void printChar(unsigned char c) {
	//waif for empty transmit buffer
	while (!(UCSR0A & (1<<UDRE0)));
	//send data through buffer
	UDR0 = c;
}
void print(char* StringToPrint) {
	while (*StringToPrint != 0x00) {
		printChar(*StringToPrint);
		StringToPrint++;
	}
}

void Initialize() {
	sprintf(String, "initialize \n");
	print(String);
	_delay_ms(100);
	cli(); //disable global interrupts for setup
	
	
	//buzzer and PWM stuff below
	DDRD |= (1<<DDD6); //set buzzer to output pin (PD6 = OC0A)
	PORTD |= (1<<PORTD6); // set initially to low
	
	//timer 0 setup, sets pre scaler to 64
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
	
	OCR0A = 30;

	sei();//enable global interrupts
}

int main(void)
{
	print_init();
	Initialize();
	int note = 0;
	while (1) {
		int nextNote = music[note];
		note++;
		if (nextNote < 0) {
			OCR0A = 0;
			_delay_ms(50);
			} else {
			OCR0A = notes[nextNote];
			_delay_ms(2500);
		}
		if (note >= 61) {
			note = 0;
		}
	}
}
