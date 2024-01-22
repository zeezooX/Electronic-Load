#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define V_CALIB 1.0
#define A_CALIB 0.75

#define RS PD0
#define EN PD2

#define BTN1 PC2
#define BTN2 PC3
#define BTN3 PC4
#define BTN4 PC5

#define DEBOUNCE_TIME 25
#define LOCK_INPUT_TIME 150

#define KP 1
#define KI 1
#define KD 1

#define REFRESH 3

unsigned int refresh_counter;

long prev_error, total_error;

unsigned int voltage_counter, current_counter;
unsigned long voltage, current, power;
volatile unsigned long voltage_buffer, current_buffer;

unsigned int setpoint;

ISR(ADC_vect) {
  if (ADMUX & 0x0F) {
    voltage_buffer += ADC;
    ADMUX = (ADMUX & 0xF0) | 0;
    voltage_counter++;
  } else {
    current_buffer += ADC;
    ADMUX = (ADMUX & 0xF0) | 1;
    current_counter++;
  }
}

void lcd_com(unsigned char p) {
  PORTD &= ~(1 << RS);
  PORTD |= (1 << EN);
  PORTD &= 0x0F;
  PORTD |= (p & 0xF0);
  _delay_us(100);
  PORTD &= ~(1 << EN);
  _delay_us(100);
  PORTD |= (1 << EN);
  PORTD &= 0x0F;
  PORTD |= (p << 4);
  _delay_us(100);
  PORTD &= ~(1 << EN);
  _delay_us(100);
}

void lcd_data(unsigned char p) {
  PORTD |= (1 << RS) | (1 << EN);
  PORTD &= 0x0F;
  PORTD |= (p & 0xF0);
  _delay_us(100);
  PORTD &= ~(1 << EN);
  _delay_us(100);
  PORTD |= (1 << EN);
  PORTD &= 0x0F;
  PORTD |= (p << 4);
  _delay_us(100);
  PORTD &= ~(1 << EN);
  _delay_us(100);
}

void lcd_string(unsigned char command, char *string) {
  lcd_com(0x0C);
  lcd_com(command);
  while (*string != '\0') {
    lcd_data(*string);
    string++;
  }
}

void lcd_init(void) {
  DDRD = 0xFF;
  PORTD = 0x00;
  _delay_ms(50);
  PORTD |= (1 << PD5);
  PORTD &= ~(1 << PD4);
  PORTD |= (1 << EN);
  PORTD &= ~(1 << EN);
  _delay_ms(5);
  lcd_com(0x28);
  lcd_com(0x08);
  lcd_com(0x01);
  _delay_us(100);
  lcd_com(0x06);
  lcd_com(0x0C);
}

void run_pid(void) {
  if (current < 200) {
    total_error = 0;
    prev_error = 0;
    OCR1A = 0;
    return;
  }
  int error = setpoint - current;
  int proportional = error * KP;
  int integral = total_error * KI;
  int derivative = (error - prev_error) * KD;
  int output = proportional + integral + derivative;
  if (output < 0) {
    output = 0;
  }
  prev_error = error;
  total_error += error;

  OCR1A = output / 10;
}

unsigned char get_button(void) {
  if (!(PINC & (1 << BTN1))) {
    _delay_ms(DEBOUNCE_TIME);
    if (!(PINC & (1 << BTN1))) {
      return 1;
    }
  }
  if (!(PINC & (1 << BTN2))) {
    _delay_ms(DEBOUNCE_TIME);
    if (!(PINC & (1 << BTN2))) {
      return 2;
    }
  }
  if (!(PINC & (1 << BTN3))) {
    _delay_ms(DEBOUNCE_TIME);
    if (!(PINC & (1 << BTN3))) {
      return 3;
    }
  }
  if (!(PINC & (1 << BTN4))) {
    _delay_ms(DEBOUNCE_TIME);
    if (!(PINC & (1 << BTN4))) {
      return 4;
    }
  }
  return 0;
}

void update_display(unsigned char c) {
  switch (c) {
    case 'v':
      lcd_com(0x8A);
      lcd_data((voltage / 10000 % 10) + '0');
      lcd_data((voltage / 1000 % 10) + '0');
      lcd_data((voltage / 100 % 10) + '0');
      lcd_com(0x8E);
      lcd_data((voltage / 10 % 10) + '0');
      break;
    case 'p':
      lcd_com(0xCA);
      lcd_data((power / 10000 % 10) + '0');
      lcd_data((power / 1000 % 10) + '0');
      lcd_data((power / 100 % 10) + '0');
      lcd_com(0xCE);
      lcd_data((power / 10 % 10) + '0');
      break;
    case 'c':
      lcd_com(0x83);
      lcd_data((current / 1000 % 10) + '0');
      lcd_com(0x85);
      lcd_data((current / 100 % 10) + '0');
      lcd_data((current / 10 % 10) + '0');
      break;
    case 's':
      lcd_com(0xC3);
      lcd_data((setpoint / 1000 % 10) + '0');
      lcd_com(0xC5);
      lcd_data((setpoint / 100 % 10) + '0');
      lcd_data((setpoint / 10 % 10) + '0');
      break;
  }
}

int main(void) {
  TCCR1A = (1 << COM1A1) + (1 << WGM11);
  TCCR1B = (1 << WGM13) + (1 << WGM12) + (1 << CS10);
  ICR1 = 833;
  OCR1A = 0;
  DDRB |= (1 << PB1);

  PORTC |= (1 << BTN1) | (1 << BTN2) | (1 << BTN3) | (1 << BTN4);

  ADMUX |= 1 << REFS0;
  ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADFR) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADIE);

  sei();

  lcd_init();

  _delay_ms(25);
  lcd_string(0x80, "PV:0.00A  000.0V");
  lcd_string(0xC0, "SP:1.00A  000.0W");

  setpoint = 1000;

  while (1) {
    switch (get_button()) {
      case 1:
        setpoint -= (setpoint < 1100) ? setpoint - 1000 : 100;
        update_display('s');
        _delay_ms(LOCK_INPUT_TIME);
        break;
      case 2:
        setpoint += (setpoint > 4900) ? 5000 - setpoint : 100;
        update_display('s');
        _delay_ms(LOCK_INPUT_TIME);
        break;
      case 3:
        setpoint -= (setpoint < 2000) ? setpoint - 1000 : 1000;
        update_display('s');
        _delay_ms(LOCK_INPUT_TIME);
        break;
      case 4:
        setpoint += (setpoint > 4000) ? 5000 - setpoint : 1000;
        update_display('s');
        _delay_ms(LOCK_INPUT_TIME);
        break;
    }

    if (voltage_counter >= 250) {
      voltage = ((voltage_buffer * 5500.0) / 1024) / voltage_counter;
      voltage = (int)((double)voltage * V_CALIB);
      if (voltage % 10 >= 5) {
        voltage += 5;
      }
      voltage_counter = 0;
      voltage_buffer = 0;
      power = (voltage * current) / 1000;

      refresh_counter++;
      if (refresh_counter > REFRESH * 2) {
        refresh_counter = 0;
        update_display('v');
        update_display('p');
      }
    }

    if (current_counter >= 250) {
      current = ((current_buffer * 100000.0) / 11264) / current_counter;
      current = (int)((double)current * A_CALIB);
      run_pid();
      if (current % 10 >= 5) {
        current += 5;
      }
      current_counter = 0;
      current_buffer = 0;

      refresh_counter++;
      if (refresh_counter > REFRESH * 2) {
        refresh_counter = 0;
        update_display('c');
        update_display('p');
      }
    }
  }
}