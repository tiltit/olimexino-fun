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

#include <libopencm3/stm32/pwr.h>
#include "power.h"
/**
 * function standby()
 * Sends the MCU into standby.
 */
void standby() 
{
	SCR |= SLEEPDEEP;
	PWR_CR |= PWR_CR_PDDS;
	PWR_CR |= PWR_CR_CWUF;
	/* Wait for event */
	__asm__("wfe");
	/* It is also posible to use wfi (wait for interupt)*/
}
