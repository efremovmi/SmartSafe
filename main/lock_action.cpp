#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include "lock_action.h"
#include "variables.h"

void lock(){
  cli(); // Отключение прерываний
  digitalWrite(LOCK_PIN, HIGH);
  Serial.println(F("Start"));
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;
  TCNT1 = 0;
  OCR1A = 0; // установка значения регистра в 0

  // устанавливаем значение OCR1A для генерации прерывания через 2 секунды
  OCR1A = 31250; // 2 секунды * 15625 Гц (частота таймера с предделителем 1024) = 31250
  TCCR1B |= (1 << WGM12);  // включить CTC режим 
  TCCR1B |= (1 << CS10); // Установить биты на коэффициент деления 1024
  TCCR1B |= (1 << CS12);

  TIMSK1 |= (1 << OCIE1A);
  sei(); // Включение прерываний
}


void set_lock(){
  cli(); // Отключение прерываний
  digitalWrite(LOCK_PIN, LOW);
  Serial.println(F("Start"));
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;
  TCNT1 = 0;
  OCR1A = 0; // установка значения регистра в 0

  // устанавливаем значение OCR1A для генерации прерывания через 2 секунды
  OCR1A = 31250; // 2 секунды * 15625 Гц (частота таймера с предделителем 1024) = 31250
  TCCR1B |= (1 << WGM12);  // включить CTC режим 
  TCCR1B |= (1 << CS10); // Установить биты на коэффициент деления 1024
  TCCR1B |= (1 << CS12);

  TIMSK1 |= (1 << OCIE1A);
  sei(); // Включение прерываний
}

ISR(TIMER1_COMPA_vect)
{
  cli();
  // Serial.println(F("Stop"));
  digitalWrite(LOCK_PIN, LOW);
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;
  TCNT1 = 0;
  OCR1A = 0;
}