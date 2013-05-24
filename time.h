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

#ifndef TIME_H
#define TIME_H

#include <libopencm3/cm3/common.h>
#include <stdbool.h>

struct Time {
	u32 year;
	u32 month;
	u32 day;
	u32 hour;
	u32 min;
	u32 sec;
};

u32 get_year();
u32 get_month();
u32 get_day();
struct Time get_time();

bool get_alarm_flag();
void set_timestamp(u32 timestamp);
void set_alarm_in(u32 count);

void systic_setup();
void delay(u32 ms);
void delay_us(u32 us);

#endif
