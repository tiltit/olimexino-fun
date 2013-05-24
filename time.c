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
#include <libopencm3/stm32/f1/rtc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/nvic.h>
#include <libopencm3/stm32/systick.h>

#include <stdbool.h>
#include "time.h"
#include "serial.h"
#include "diskio.h" 

volatile u32 counter;
volatile u32 temp32;

bool get_alarm_flag() 
{
	if ( RTC_CRL & RTC_CRL_ALRF == 1  )
		return true;
	else
		return false;
}

void set_timestamp(u32 timestamp)
{
	/* rtc_auto_awake(LSE, 0x7fff); */
	rtc_set_counter_val(timestamp + (3 * 3600 ));
}

void set_alarm_in(u32 count)
{
	/* Get current timestamp */
	u32 timestamp = rtc_get_counter_val();

	/* Set alarm */
	rtc_set_alarm_time(timestamp + count);

	/* enable alarm */
	rtc_enable_alarm();
}

bool is_leap_year(u32 year)
{
	/* Thanks wikipedia! */
	if (year % 400 == 0 )
		return true;
	else if (year % 100 == 0)
		return false;
	else if (year % 4 == 0)
		return true;
	else
		return false;
}

u32 get_year_and_remaining_seconds(u32 *remaining_seconds)
{	
	u32 time = rtc_get_counter_val();
	const u32 seconds_in_year = 31536000;
	const u32 seconds_in_leap_year = 31622400;
	const u32 year_one = 1970;
	u32 year = year_one;
	while(1) {
		if(is_leap_year(year)) {
			if (time < seconds_in_leap_year){
				*remaining_seconds = time;
				return year;
			} else {
				time -= seconds_in_leap_year;
				++year;
			}
		} else {
			if (time < seconds_in_year) {
				*remaining_seconds = time;
				return year;
			} else {
				time -= seconds_in_year;
				++year;
			}
		}
	}
}
/**
 *
 * Function get_date(u32, u32, u32)
 *
 * Returns the number of secounds in the current day
 * and as a side effect writes the current year month
 * and day into its arguments.
 *
 */
u32 get_date(u32 *year, u32 *month, u32 *day) 
{
	u32 seconds;
	const u32 seconds_per_day = 86400;
	*year = get_year_and_remaining_seconds(&seconds);
	*month = 0;
	while (1) {
		if(*month == 1){
			/* handle febuary */
			if(is_leap_year(*year)) {
				/* 28 days */
				if (seconds < (28 * seconds_per_day)){
					*day = (seconds / 86400);
					return seconds;
				} else {
					seconds -= (28 * seconds_per_day);
				}
			} else {
				/* 29 days */
				if (seconds < 29 * seconds_per_day){
					*day = seconds / 86400;
					return seconds;
				} else {
					seconds -= (29 * seconds_per_day);
				}
			}
		} else if ((*month % 2) == 0) {
			/* Month with 31 days */
			if (seconds < (31 * seconds_per_day)){
					*day = seconds / 86400;
					return seconds;
				} else {
					seconds -= (31 * seconds_per_day);
				}
		} else { 
			/* Month with 30 days */
			if (seconds < (30*seconds_per_day)){
					*day = seconds / 86400;
					return seconds;
				} else {
					seconds -= (30 * seconds_per_day);
				}
		}
		(*month)++;
	}
}

u32 get_year()
{
	u32 remaining;
	return get_year_and_remaining_seconds(&remaining);
}

u32 get_month()
{
	u32 y, m, d;
	get_date(&y, &m, &d);
	return m;
}

u32 get_day()
{
	u32 y, m, d;
	get_date(&y, &m, &d);
	return d;
}


struct Time get_time()
{
	u32 remaining;
	struct Time time;
	remaining = get_date(&time.year, &time.month, &time.day);
	time.month++; /* my get_date function returns January as 0 */
	time.hour = (remaining / 3600) % 24;
	time.min = (remaining / 60) % 60;
	time.sec = remaining % 60;
	return time;
}

u32 get_fattime (void)
{
	/* Get local time */
	struct Time time = get_time();
	/* Pack date and time into a unsigned 32 bit inte */
	
	return	  ((u32)(time.year - 1980) << 25)
			| (time.month << 21)
			| (time.day << 16)
			| (time.hour << 11)
			| (time.min << 5)
			| (time.sec >> 1);
}

void sys_tick_handler(void)
{
	++counter;
	temp32++;
	if (temp32 == 1000) {
                gpio_toggle(GPIOA, GPIO5); /* LED2 on/off */
                temp32 = 0;
        }
	/* Update timing for sd card access */
	disk_timerproc();
}

void delay(u32 ms)
{
	volatile u32 count = 0;
	counter = 0;
	while( counter < ms)
		count++;
	/* writeln(itoa(count, 10));*/
}

void delay_us(u32 us)
{
	volatile u32 int_to_reach = (6 * us);
	volatile u32 count = 0;
	while(count < int_to_reach)
		count++;
}


void systic_setup()
{
	/* 72MHz / 8 => 9000000 counts per second */
	systick_set_clocksource(STK_CTRL_CLKSOURCE_AHB_DIV8);

	/* 9000000/9000 = 1000 overflows per second - every 1ms one interrupt */
	systick_set_reload(9000);

	systick_interrupt_enable();
	
	counter=0;

	/* Start counting. */
	systick_counter_enable();
}

