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

#ifndef SERIAL_H
#define SERIAL_H

#define USART USART2

#include <libopencm3/stm32/usart.h>
#include <stdbool.h>

char* itoa(u32 val, int base);
void usart_setup(void);
void putchar(char c);
void write(char *string);
void writeln(char *string);
void proccess_command(char *line, char *command, char *arguments);

bool line_available();
char* readln();

#endif
