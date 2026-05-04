/*
 * Proyecto
 *
 * Author: Sebastian Da Silva 
 */
// ====================================================================
// Libraries
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
// ====================================================================
// Variables
uint8_t Manual = 0;  // Variable para usar terminal o pots
char buffer[10];  //
uint16_t Terminal = 0; //para controlar cuando leer el terminal 
// ====================================================================
// Function prototypes
void setup(void);
uint16_t leerADC(uint8_t canal);
void enviarChar(char c);
void enviarString(char* str);
void limpiarTerminal(void);
// ====================================================================
// Main Function
int main(void) {
	setup();
	while(1) {
				if (Manual == 0) {
					// Loop para modo Manual (Manual = 0)
					PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5);
					uint16_t pot1 = (leerADC(4)/10);  
					// Leer pot 2
					uint16_t pot2 = (leerADC(5)/10);  
					// Leer pot 3
					uint16_t pot3 = (leerADC(6)/10); 
					// Leer pot 4
					uint16_t pot4 = (leerADC(7)/10);  
					
					_delay_ms(10);
					
					if (Terminal == 0) {
						limpiarTerminal();
						enviarString("\r\n======================\r\n");
						enviarString("\r\nModo Manual\r\n");
						enviarString("\r\n======================\r\n");
						enviarString("Pot 1: ");
						enviarString(itoa(pot1, buffer, 10));
						enviarString("\r\nPot 2: ");
						enviarString(itoa(pot2, buffer, 10));
						enviarString("\r\nPot 3: ");
						enviarString(itoa(pot3, buffer, 10));
						enviarString("\r\nPot 4: ");
						enviarString(itoa(pot4, buffer, 10));
						enviarString("\r\n======================\r\n");
						Terminal = 100;
					}
					else { Terminal--;}
					
				}
				else {
					// Loop para modo Automatico (Manual = 1)

				}
		//aqui termina el while(1)		
		}
	}
// ====================================================================
// NON-Interrupt subroutines
//SETUP
void setup(void) {
	// Configurar todo el Puerto D como salida
	DDRD = 0xFF;  // Todos los pines del Puerto D como salidas
	
	// Configurar ADC para leer ADC6 y ADC7
	// Habilitar ADC y seleccionar prescaler de 128 (para 16MHz -> 125kHz)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	// Configurar referencia: A
	ADMUX = (1 << REFS0);  
	
	// Configurar USART a 9600 baudios (para 16MHz)
	UBRR0H = 0;
	UBRR0L = 103;  // 16MHz / (16 * 9600) - 1 = 103

	// Habilitar transmisor y receptor
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);

	// Configurar formato: 8 bits, 1 stop bit, sin paridad
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
}
//LEER ADC
uint16_t leerADC(uint8_t canal) {
	// Limpiar los bits de selección de canal (MUX[2:0]) y seleccionar el nuevo canal
	ADMUX = (ADMUX & 0xF8) | (canal & 0x07);
	
	// Iniciar conversión
	ADCSRA |= (1 << ADSC);
	
	// Esperar a que termine la conversión (ADSC se limpia automáticamente)
	while (ADCSRA & (1 << ADSC));
	
	// Retornar el valor de 10 bits
	return ADC;
} 

void enviarChar(char c) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}

void enviarString(char* str) {
	while (*str) {
		enviarChar(*str++);
	}
}
void limpiarTerminal(void) {
	// Enviar saltos de línea para "limpiar la" pantalla
	for (int i = 0; i < 10; i++) {
		enviarString("\r\n");
	}
}
// ====================================================================
// Interrupt subroutines