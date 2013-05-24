/**
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/nvic.h>

#include "serial.h"
#include "events.h"

/* Incoming buffer */
volatile static char line[80];

char* itoa(u32 val, int base)
{
	static char buf[32] = {0};
	int i = 30;
	if (val == 0) return "0";
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];
}

void putchar(char c)
{
	usart_send_blocking(USART1, c);
}

void writeln(char *string)
{
	int i = 0;
	while (string[i] != '\0') {
		usart_send_blocking(USART1, string[i]);
		++i;
	}
	usart_send_blocking(USART1, '\r');
	usart_send_blocking(USART1, '\n');
}

void write(char *string)
{
	int i = 0;
	while (string[i] != '\0') {
		usart_send_blocking(USART1, string[i]);
		++i;
	}
}

void proccess_command(char *line, char *command, char *argument)
{
	int i=0;
 	/* char command[80]; */
	
	/* Ignore spaces. */
	while (line[i] == ' ')
		i++;

	while( (line[i] != '\0') && (line[i] != ' ')){
		command[i]=line[i];
 		i++;
 	}
 	command[i]='\0';

 	/* Ignore spaces */
 	while(line[i]==' ')
 		i++;

 	int j = 0;

	while(line[i] != '\0'){
		argument[j++]=line[i];
 		i++;
 	}
	argument[j]='\0';
}

bool line_available()
{
	return get_event(ULINE_IN);
}

char* readln()
{
	char ln[80];
	int i = 0;
	while ( (line[i] != '\0') || (i == 80))
		ln[i] = line[i++];
	ln[i] = '\0';
	clear_event(ULINE_IN);
	return ln;
}

void usart1_isr(void)
{
	char data = 'A';
	static char *pline = &line[0];

	/* Check if we were called because of RXNE. */
	if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
	    ((USART_SR(USART1) & USART_SR_RXNE) != 0)) {

		/* Retrieve the data from the peripheral. */
		data = usart_recv(USART1);
		if (data != '\r') {
			*pline = data;
			pline++;
		} else {
			usart_send_blocking(USART1, '\n');
			*pline = '\0';
			/* proccess_command(line); */
			set_event(ULINE_IN);
			pline = &line[0];
		}		
		
	}
}

void usart1_setup(void)
{
	/* Enable the USART1 interrupt. */
	nvic_enable_irq(NVIC_USART1_IRQ);

	/* Setup GPIO pin GPIO_USART1_RE_TX on GPIO port B for transmit. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

	/* Setup GPIO pin GPIO_USART1_RE_RX on GPIO port B for receive. */
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
		      GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);

	/* Setup UART parameters. */
	usart_set_baudrate(USART1, 38400);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_set_mode(USART1, USART_MODE_TX_RX);

	/* Enable USART1 Receive interrupt. */
	USART_CR1(USART1) |= USART_CR1_RXNEIE;

	/* Finally enable the USART. */
	usart_enable(USART1);
}
