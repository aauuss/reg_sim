#define F_CPU 9600000UL

#include <avr/io.h>
#include <stdio.h>
//#include <util/delay.h>
#include <avr/interrupt.h>

#define ZC PB1
#define OUT PB0

int sp, lastsp;

void setup(void) {
  DDRB = (1 << OUT);
  PORTB = 0x00;

  ADMUX = 0x22; // опорное напряжение - VCC, левое ориентирование данных, выбран вход ADC2 (на нём висит перем. резистор)
  ADCSRA = 0xE7; // АЦП включен, запуск преобразования, режим автоизмерения, прерывание по окончанию преобразования, частота CLK/128
  ADCSRB = 0x00; // режим автоизмерения: постоянно запущено
  DIDR0 |= (1 << PB4); // запрещаем цифровой вход на ноге аналогового входа

  GIMSK = (1 << INT0);     //включаем внешнее прерывание (на ноге PB1)
  MCUCR = (0 << ISC01) | (1 << ISC00);    //прерывание по заднему фронту

  TCCR0A = 0X00;
  TCCR0B = 0X00;
  TIMSK0 = 0X00;
  TIMSK0 = (1 << OCIE0A) | (0 << OCIE0B); 

  sei();
  
}

ISR(INT0_vect){
cli();
if (PINB & 0x02){
  PORTB &= ~(1 << OUT);
  TCNT0 = 0X00;
  TCCR0B = 0X00;
} else {
  TCNT0 = 0X00;
  TCCR0B = 0X05;
}
sei();
}

ISR(TIM0_COMPA_vect) {
  PORTB |= (1 << OUT); 
}
/*
ISR(TIM0_COMPB_vect) {
  PORTB &= ~(1 << OUT);
  TCCR0B = 0X00;
}

/*
ISR(ADC_vect) {
    OCR0A = ADCH;
    OCR0B = ADCH + 5;    
}
*/

void main(void) {
  setup();
  while (1) {
    sp = (ADCH >> 1) + (lastsp * 31);
    sp = (sp >> 5);
    lastsp = sp;
    if (sp < 1) {      
      PORTB |= (1 << OUT);
      TCNT0 = 0X00;
      GIMSK = (0 << INT0);
    } else if (sp > 75) {
      PORTB &= ~(1 << OUT);
      TCNT0 = 0X00;
      GIMSK = (0 << INT0);
    } else {
      GIMSK = (1 << INT0);
      OCR0A = sp;
      OCR0B = sp + 5;
    }
  }
}


