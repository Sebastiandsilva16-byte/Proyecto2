/*
 * Created: 16/05/2026 02:52:35 p. m.
 * Author : Dasil
 */ 


// ====================================================================
// Libraries
#include <avr/io.h>
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>



// ====================================================================
// Variables
uint8_t modo = 0;
uint16_t pot1, pot2, pot3, pot4;
uint16_t mot1, mot2, mot3, mot4;
uint16_t pulso1, pulso2, pulso3, pulso4;
volatile uint8_t contador_segundos = 0;
volatile uint8_t flag_enviar = 0;
char buffer[32];
uint8_t indice_buffer = 0;
uint8_t datos_recibidos[5];
uint8_t datos_completos = 0;
// ====================================================================
// Function prototypes
void setup(void);
uint16_t leerADC(uint8_t canal);
void enviar_por_usart(char dato);
void enviar_string(const char* str);
void enviar_pots(uint16_t pot1, uint16_t pot2, uint16_t pot3, uint16_t pot4);
// ====================================================================
// MAIN
int main(void)
{
	pot4 = 0;
	setup();
    while (1) 
    {
		
		if (modo == 0) {
			//led que indica el modo
			PORTD |= (1 << PD6);
			PORTD &= ~(1 << PD7);
			// Modo potenciometros
			pot1 = leerADC(4);
			pot2 = leerADC(5);
			pot3 = leerADC(6);
			pot4 = leerADC(7);
// 			Para que funcione igual que el de adafruit
						mot1 = pot1/10;
						mot2 = pot2/10;
						mot3 = pot3/10;
						mot4 = pot4/10;

			//FINAL MODO POT
		}
		else if (modo == 1) {
			//led que indica el modo
			PORTD &= ~(1 << PD6);
			PORTD |= (1 << PD7);
			// Modo adafruit	
			//FINAL MODO AFAFRUIT
		}
		
		if (flag_enviar) {
			flag_enviar = 0;
			enviar_pots(mot1, mot2, mot3, mot4);
		}
		
		pulso1 = (mot1 * 15); 
		pulso2 = (mot2 * 15);
		pulso3 = (mot3 * 15);
		pulso4 = (mot4 * 15);
		
		// Controlar servos en PORTC0, PORTC1, PORTC2, PORTC3
		PORTC |= (1 << PC0);
		for (uint16_t i = 0; i < pulso1; i++) { _delay_us(1); }
		PORTC &= ~(1 << PC0);

		PORTC |= (1 << PC1);
		for (uint16_t i = 0; i < pulso2; i++) { _delay_us(1); }
		PORTC &= ~(1 << PC1);

		PORTC |= (1 << PC2);
		for (uint16_t i = 0; i < pulso3; i++) { _delay_us(1); }
		PORTC &= ~(1 << PC2);

		PORTC |= (1 << PC3);
		for (uint16_t i = 0; i < pulso4; i++) { _delay_us(1); }
		PORTC &= ~(1 << PC3);
		
	//Fin del While
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
	_delay_ms(10);
	
	// Configurar referencia: A
	ADMUX = (1 << REFS0);
	
	// Configurar USART a 9600 baudios (para 16MHz)
	UBRR0H = 0;
	UBRR0L = 103;  // 16MHz / (16 * 9600) - 1 = 103

	// Habilitar transmisor y receptor
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);

	// Configurar formato: 8 bits, 1 stop bit, sin paridad
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
	//Control de los servos
	DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3);
	PORTC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3));
	
	// Configurar Timer1 para interrupción cada ~10 segundos
	TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);  // CTC, prescaler 1024
	OCR1A = 15624;  // 1 segundo
	TIMSK1 = (1 << OCIE1A);  // Habilitar interrupción
	sei();  // Habilitar interrupciones globales
	
	// Habilitar transmisor y receptor Y la interrupción de recepción
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);  // <-- Añadir RXCIE0
	
	
	
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

void enviar_por_usart(char dato) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = dato;
}

void enviar_string(const char* str) {
	while (*str) {
		enviar_por_usart(*str++);
	}
}

void enviar_pots(uint16_t pot1, uint16_t pot2, uint16_t pot3, uint16_t pot4) {
	char buffer[16];
	
	itoa(pot1, buffer, 10);
	enviar_string(buffer);
	enviar_por_usart('\n');
	
	itoa(pot2, buffer, 10);
	enviar_string(buffer);
	enviar_por_usart('\n');
	
	itoa(pot3, buffer, 10);
	enviar_string(buffer);
	enviar_por_usart('\n');
	
	itoa(pot4, buffer, 10);
	enviar_string(buffer);
	enviar_por_usart('\n');
	
}
// ====================================================================
// NON-Interrupt subroutines

ISR(TIMER1_COMPA_vect) {
	contador_segundos++;
	if (contador_segundos >= 10) {
		contador_segundos = 0;
		flag_enviar = 1;
	}
}

//(para adafruit)
ISR(USART_RX_vect) {
	char c = UDR0;
	
	if (c == ' ') {  // Detect enter (newline)
		if (indice_buffer > 0) {
			buffer[indice_buffer] = '\0';
			datos_recibidos[datos_completos] = atoi(buffer);
			datos_completos++;
			indice_buffer = 0;
			
			if (datos_completos >= 5) {
				    
					modo = datos_recibidos[0];
				    
				    if (modo == 0) {
					    // En modo 0, solo actualiza el modo
					    // No cambia motores
				    }
				    else if (modo == 1) {
					    // En modo 1, actualiza modo y motores
					    mot1 = datos_recibidos[1];
					    mot2 = datos_recibidos[2];
					    mot3 = datos_recibidos[3];
					    mot4 = datos_recibidos[4];
				    }
				datos_completos = 0;
			}
		}

	}
	else if (c >= '0' && c <= '9' && indice_buffer < 31) {
		buffer[indice_buffer] = c;
		indice_buffer++;
	}
}	