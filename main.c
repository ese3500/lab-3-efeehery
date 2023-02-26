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

int mode = 0;
int x = 0;
long timeStart = 0;
long timeEnd = 0;
int numOverflows = 0;
char String[25];
int distanceCM = 0;

bool discrete = 1;
int discrete_freqs[8] = {30, 32, 35, 40, 45, 47, 53, 60};
	
int adc = -1;

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
	
	//pin 8 is the input - echo - B0  (PB0=ICP1=PCINT0 -input capture for timer 1)
	//pin 7 is the output - trig - D7
	
	DDRD |= (1<<DDD7); //set to output pin
	PORTD &= ~(1<<PORTD7); // set initially to low
	
	DDRB &= ~(1<<DDB0); //set to input pin - pin 8 for reading echo
	PORTB &= ~(1<<PORTB0);
	
	//button setup
	DDRB &= ~(1<<DDB1); //set to input pin - pin 9 for reading button
	PORTB |= (1<<PORTB1); //enable pull up resistor
	PCICR |= (1<<PCIE0); //enable PCINT1 pin change interrupt
	PCMSK0 |= (1<<PCINT1); //enable trigger for PCINT1
	
	//timer 1 setup with pre scaler of 8
	TCCR1B &= ~(1<<CS10);
	TCCR1B |= (1<<CS11);
	TCCR1B &= ~(1<<CS12);
	
	//set timer to normal
	TCCR1A &= ~(1<<WGM10);
	TCCR1A &= ~(1<<WGM11);
	TCCR1B &= ~(1<<WGM12);
	TCCR1B &= ~(1<<WGM13);
	
	TCCR1B |= (1<<ICES1);//looking for rising edge
	TIFR1 |= (1<<ICF1);//clear interrupt flag
	TIMSK1 |= (1<<ICIE1);//enable input capture interrupt
	
	TCCR1A |= (1<<COM1A0);
	TCCR1A &= ~(1<<COM1A1);
	
	//buzzer and PWM stuff below
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
	
	//toggle OC1A on compare match
	TCCR0A |= (1<<COM0A0);
	TCCR0A &= ~(1<<COM0A1);
	
	//OC0B = 1 on compare match when up counting; OC0B = 0 on comp match when down counting
	//OC0B = PCINT21 - PD5
	TCCR0A |= (1<<COM0B0);
	TCCR0A |= (1<<COM0B1);
	
	OCR0A = 30;//initial values
	OCR0B = 25; //=OCR0A*(1-duty_cycle_of_15)
	
	//ADC photoresistor setup
	PRR &= ~(1<<PRADC); //clear power reduction for adc
	//select Vref = AVcc
	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);
	//set the ADC clock div by 128
	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);
	//select channel 0
	ADMUX &= ~(1<<MUX0);
	ADMUX &= ~(1<<MUX1);
	ADMUX &= ~(1<<MUX2);
	ADMUX &= ~(1<<MUX3);
	//set to free running
	ADCSRA |= (1<<ADATE); //autotriggering of ADC
	ADCSRB &= ~(1<<ADTS0); //free running mode ADTS[2:0]==000
	ADCSRB &= ~(1<<ADTS1);
	ADCSRB &= ~(1<<ADTS2);
	//disable digital input buffer on ADC pin
	DIDR0 |= (1<<ADC0D);
	//enable ADC
	ADCSRA |= (1<<ADEN);
	//enable ADC interrupt
	ADCSRA |= (1<<ADIE);
	//start conversion
	ADCSRA |= (1<<ADSC);

	sei();//enable global interrupts
}
ISR(ADC_vect) {
	adc = ADC;
}
ISR(TIMER1_CAPT_vect) {
	if (mode != 0) {
		x ++;
		if (TCCR1B & (1 << ICES1)) {
			TCNT1 = 0;
			//timeStart = TCNT1;
			numOverflows = 0;
			mode = 1;
			_delay_us(10);
			TCCR1B &= ~(1<<ICES1);//looking for falling edge
		} else {
			timeEnd = TCNT1;
			mode = 0;
			TCCR1B |= (1<<ICES1);//rising edge
		}
	} else {
		x--;
	}
	
}
ISR(TIMER1_OVF_vect) {
	numOverflows++;
}
ISR(PCINT0_vect) { // button change
	if (PINB & (1<<PINB1)) { //button is high
		discrete = !discrete; //toggle discrete and continuous modes
		//discrete = 0;
	}
}
int main(void)
{
	print_init();
	Initialize();
	while (1) {
		if (mode == 0) {
			PORTD |= (1<<PORTD7);//output high
			_delay_us(5000);
			PORTD &= ~(1<<PORTD7);//output low
			mode = 1;
		}
		double distance = (64 * (timeEnd) * 8) / 1024 / 58;
		sprintf(String, "%u :distance \n", (int)distance);
		print(String);
		int converted = 0;
		if (discrete == 0) {
			converted = (int)(distance * 0.4285 + 31.71);
		} else {
			int pos_in_range = (int)(distance/70*8);
			if (pos_in_range >= 8) {
				pos_in_range = 7;
			}
			converted = discrete_freqs[pos_in_range];
		}
		//sprintf(String, "%u :OCR0A \n", converted);
		//print(String);
		OCR0A = converted;
		sprintf(String, "%u :discrete \n", (int)discrete);
		print(String);
		
		//(ADC-240)/75
		int index = (adc-240)/75;
		if (index < 0) {
			index = 0;
		}
		if (index > 9) {
			index = 9;
		}
		//(index/9)*45+5
		double i = index;
		int dc = (i / 9.0) * 45.0 + 5;
		OCR0B = OCR0A * (1.0 - (dc/100.0));
		_delay_ms(1000);
	}
}
