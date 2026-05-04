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
uint16_t pot1 = 0;
uint16_t pot2 = 0;
uint16_t pot3 = 0;
uint16_t pot4 = 0;
uint8_t Manual = 0;  // Variable para usar terminal o pots
char buffer[10];  //
uint16_t Terminal = 0; //para controlar cuando leer el terminal 
uint8_t editando = 0;  // Flag para saber si estamos en modo edición
uint8_t potSeleccionado = 0;  // Qué pot estamos editando (1-4)
uint8_t pasoEdicion = 0;  // 0: esperando comando, 1: esperando número
char numeroBuffer[4];  // Buffer para guardar el número ingresado
uint8_t indiceNumero = 0;  // Índice para el buffer del número
// ====================================================================
// Function prototypes
void setup(void);
uint16_t leerADC(uint8_t canal);
void enviarChar(char c);
void enviarString(char* str);
void limpiarTerminal(void);
void manejarComandosSerial(void);
void mostrarMenuAuto(void);
// ====================================================================
// Main Function
int main(void) {
	setup();
	while(1) {
		// Verificar comandos por serial
		manejarComandosSerial();
				if (Manual == 0) {
					// Loop para modo Manual (Manual = 0)
					//led que indica modo manual
					PORTD |= (1 << PD6);
					PORTD &= ~(1 << PD7); 
					//para leer los pots
					PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5);
					pot1 = (leerADC(4)/10);  
					// Leer pot 2
					pot2 = (leerADC(5)/10);  
					// Leer pot 3
					pot3 = (leerADC(6)/10); 
					// Leer pot 4
					pot4 = (leerADC(7)/10);  
					
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
						enviarString("\r\nPress 2 para cambiar de modo\r\n");
						Terminal = 100;
					}
					else { Terminal--;}
				}
				else {
					// Loop para modo Automatico (Manual = 1)
					//indicar el modo automatico
					PORTD &= ~(1 << PD6);
					PORTD |= (1 << PD7);
					if (Terminal == 0) {
						limpiarTerminal();
						 mostrarMenuAuto(); 
						Terminal = 1;
					}
					else { //nada por ahora
					}
					
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
void manejarComandosSerial(void) {
	if (UCSR0A & (1 << RXC0)) {
		char comando = UDR0;
		
		// Si estamos en modo edición
		if (editando == 1) {
			if (pasoEdicion == 0) {
				// Esperando selección del pot (1-4)
				if (comando >= '1' && comando <= '4') {
					potSeleccionado = comando - '0';
					enviarString("\r\nIngrese el nuevo valor (0-100): ");
					pasoEdicion = 1;
					indiceNumero = 0;
					memset(numeroBuffer, 0, sizeof(numeroBuffer));
					} else {
					enviarString("\r\nOpcion invalida. Seleccione un pot (1-4): ");
				}
			}
			else if (pasoEdicion == 1) {
				// Esperando el valor numérico
				if (comando >= '0' && comando <= '9') {
					if (indiceNumero < 3) {
						numeroBuffer[indiceNumero++] = comando;
						enviarChar(comando);
					}
					// Si ya tenemos 3 dígitos, validamos automáticamente
					if (indiceNumero == 3) {
						uint8_t nuevoValor = atoi(numeroBuffer);
						if (nuevoValor >= 0 && nuevoValor <= 100) {
							switch(potSeleccionado) {
								case 1: pot1 = nuevoValor; break;
								case 2: pot2 = nuevoValor; break;
								case 3: pot3 = nuevoValor; break;
								case 4: pot4 = nuevoValor; break;
							}
							enviarString("\r\nPot ");
							enviarString(itoa(potSeleccionado, buffer, 10));
							enviarString(" actualizado a: ");
							enviarString(itoa(nuevoValor, buffer, 10));
							enviarString("\r\n");
							
							editando = 0;
							pasoEdicion = 0;
							Terminal = 0;
							} else {
							// Valor mayor a 100
							enviarString("\r\nERROR: Valor debe ser entre 0 y 100\r\n");
							enviarString("Presione cualquier tecla para continuar...\r\n");
							editando = 0;
							pasoEdicion = 0;
							Terminal = 0;
						}
					}
				}
				else if (comando == '\r') {  // Enter
					if (indiceNumero > 0) {
						uint8_t nuevoValor = atoi(numeroBuffer);
						if (nuevoValor >= 0 && nuevoValor <= 100) {
							switch(potSeleccionado) {
								case 1: pot1 = nuevoValor; break;
								case 2: pot2 = nuevoValor; break;
								case 3: pot3 = nuevoValor; break;
								case 4: pot4 = nuevoValor; break;
							}
							enviarString("\r\nPot ");
							enviarString(itoa(potSeleccionado, buffer, 10));
							enviarString(" actualizado a: ");
							enviarString(itoa(nuevoValor, buffer, 10));
							enviarString("\r\n");
							
							editando = 0;
							pasoEdicion = 0;
							Terminal = 0;
							} else {
							enviarString("\r\nERROR: Valor debe ser entre 0 y 100\r\n");
							enviarString("Presione cualquier tecla para continuar...\r\n");
							editando = 0;
							pasoEdicion = 0;
							Terminal = 0;
						}
						} else {
						enviarString("\r\nNo ingreso ningun valor\r\n");
						enviarString("Presione cualquier tecla para continuar...\r\n");
						editando = 0;
						pasoEdicion = 0;
						Terminal = 0;
					}
				}
				// Si se presiona cualquier otra tecla (no número, no Enter) salir
				else {
					enviarString("\r\nComando cancelado\r\n");
					enviarString("Presione cualquier tecla para continuar...\r\n");
					editando = 0;
					pasoEdicion = 0;
					Terminal = 0;
				}
			}
			return;
		}
		
		// Comandos normales (no en edición)
		if (comando == '1') {
			Manual = 0;
			enviarString("\r\nCambiando a MODO MANUAL\r\n");
			Terminal = 0;
		}
		else if (comando == '2') {
			Manual = 1;
			enviarString("\r\nCambiando a MODO AUTOMATICO\r\n");
			Terminal = 0;
		}
		else if (comando == '3' && Manual == 1) {
			editando = 1;
			pasoEdicion = 0;
			limpiarTerminal();
			enviarString("\r\n===== EDITAR VALORES =====\r\n");
			enviarString("Seleccione el pot a editar:\r\n");
			enviarString("1 - Pot 1 (actual: ");
			enviarString(itoa(pot1, buffer, 10));
			enviarString(")\r\n");
			enviarString("2 - Pot 2 (actual: ");
			enviarString(itoa(pot2, buffer, 10));
			enviarString(")\r\n");
			enviarString("3 - Pot 3 (actual: ");
			enviarString(itoa(pot3, buffer, 10));
			enviarString(")\r\n");
			enviarString("4 - Pot 4 (actual: ");
			enviarString(itoa(pot4, buffer, 10));
			enviarString(")\r\n");
			enviarString("\r\nOpcion: ");
		}
	}
}

void mostrarMenuAuto(void) {
	enviarString("\r\n======================\r\n");
	enviarString("\r\nMODO AUTOMATICO\r\n");
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
	enviarString("\r\nComandos:\r\n");
	enviarString("1 - Volver a Modo Manual\r\n");
	enviarString("3 - Editar valor de Pot\r\n");
	enviarString("\r\n");
}

// ====================================================================
// Interrupt subroutines