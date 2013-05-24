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
#include <libopencm3/stm32/f1/rtc.h>
#include <libopencm3/stm32/pwr.h>
#include <stdlib.h>

#include "serial.h"
#include "time.h"
#include "power.h"
#include "ff.h"
#include "diskio.h"

FATFS Fatfs;		/* File system object */
FIL Fil;			/* File object */
BYTE Buff[128];		/* File read buffer */

void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);

	/*
	 * Enable clocks for GPIO port A (for GPIO_USART1_TX) and USART1
	 * and onboard leds.
	 */
	/*
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN |
				    RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN);
	*/
	/* Enable clock for GPIO port A (onboard leds)*/
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);

	/* Set up GPIO port D for SD Cable Select. */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);

	/* Enable port B for SPI. */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
	/* Set up SPI clock */
	rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_SPI2EN);

	/* Enable power and backup interface clocks. */
	
       	RCC_APB1ENR |= (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN);
	
        /* Enable access to the backup registers and the RTC. */
        
	PWR_CR |= PWR_CR_DBP;
}


void gpio_setup(void)
{
	/*
	 * LED 1: PA5
	 * LED 2: PA1
	 */
	gpio_set(GPIOA, GPIO1);
	gpio_set(GPIOA, GPIO5);
	gpio_set(GPIOA, GPIO3);
	/*
	d0 = pa3

	*/

	/* Setup GPIO1 and 5 (in GPIO port A) for LED use. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO1| GPIO3 | GPIO5);

}

void write_time()
{
	struct Time time = get_time();
	write(itoa(time.year, 10));
	write("/");
	write(itoa(time.month, 10));
	write("/");
	write(itoa(time.day, 10));
	write("  ");
	write(itoa(time.hour, 10));
	write(":");
	write(itoa(time.min, 10));
	write(":");
	writeln(itoa(time.sec, 10));
}

void write_timestamp()
{
	int i;
	/* Read the conter. */
	u32 timestamp = rtc_get_counter_val();

	/* Display the current counter value in binary via USART. */
	for (i = 0; i != 32; ++i) {
		if ((timestamp & (0x80000000 >> i)) != 0) {
			usart_send_blocking(USART, '1');
		} else {
			usart_send_blocking(USART, '0');
		}
	}
	usart_send_blocking(USART, '\r');
	usart_send_blocking(USART, '\n');
}


void test_delay_us(u32 delay)
{
	u32 i;
	for(i=0;i!=90000000;i++) {
		gpio_toggle(GPIOA, GPIO3);
		delay_us(delay);
	}
}


void rcv_command(char *command, char *argument)
{
	/* Compare command to known commands */
	if (strcmp(command, "foo") == 0){
		writeln("bar");
	} else if ( strcmp(command, "get_timestamp") == 0 ) {
		write_timestamp();
	} else if ( strcmp(command, "get_time") == 0 ) {
		write_time();
	} else if ( strcmp(command, "set_time") == 0 ) {
		char *endptr;
		writeln( argument );
		set_timestamp( strtol(argument, endptr, 10) );
	} else if ( strcmp(command, "set_alarm") == 0 ) {
		char *endptr;
		set_alarm_in( strtol(argument, endptr, 10) );	
	} else if ( strcmp(command, "standby") == 0) {
		standby();
	} else if ( strcmp(command, "get_year") == 0) {
		writeln(itoa(get_year(),10 ));
	} else if ( strcmp(command, "get_month") == 0) {
		writeln(itoa(get_month(),10 ));
	} else if ( strcmp(command, "get_day") == 0) {
		writeln(itoa(get_day(),10 ));
	} else if ( strcmp(command, "test_delay") == 0) {
		int secs;
		for(secs=0;secs!=1000;++secs) {
			//writeln(itoa(secs, 10));
			delay(100);
			gpio_toggle(GPIOA, GPIO3);
		}
	} else if ( strcmp(command, "test_delay_us") == 0) {
		char *endptr;
		test_delay_us(strtol(argument, endptr, 10));
       	} else {
		writeln("Command unknown.");
		return 1;
	}
}

int test_sd()
{
	FRESULT rc;				/* Result code */
	DIR dir;				/* Directory object */
	FILINFO fno;			/* File information object */
	UINT bw, br, i;

	f_mount(0, &Fatfs);		/* Register volume work area (never fails) */
	writeln("\nOpen an existing file (test.txt).");
	rc = f_open(&Fil, "test.txt", FA_READ);
	if (rc) return 1;
	write("\nType the file content.\n");
	for (;;) {
		rc = f_read(&Fil, Buff, sizeof Buff, &br);	/* Read a chunk of file */
		if (rc || !br) break;			/* Error or end of file */
		for (i = 0; i < br; i++)		/* Type the data */
			putchar(Buff[i]);
	}
	if (rc) return 1;
	writeln("\nClose the file.\n");
	rc = f_close(&Fil);


	writeln("\nCreate a new file (hello.txt).\n");
	rc = f_open(&Fil, "hello.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (rc) return 1;
	write("\nWrite a text data. (Hello world!)\n");
	rc = f_write(&Fil, "Hello world!\r\n", 14, &bw);
	if (rc) return 1;;
	write(itoa (bw, 10));
	writeln(" bytes written.\n");
	writeln("\nClose the file.\n");
	rc = f_close(&Fil);
	if (rc) return 1;
	writeln("\nOpen root directory.\n");
	rc = f_opendir(&dir, "");
	if (rc) return 1;
	writeln("\nDirectory listing...\n");
	for (;;) {
		rc = f_readdir(&dir, &fno);		/* Read a directory item */
		if (rc || !fno.fname[0]) break;	/* Error or end of dir */
		if (fno.fattrib & AM_DIR) {
			write("   <dir>"); 
			writeln(fno.fname);
		} else {
			write(itoa(fno.fsize, 10));
			write("  ");
			writeln(fno.fname);
		}
	}
	if (rc) return 1;
	writeln("Test completed.");
	return 0;
}

void log_in_file(char *filename, char *log)
{
	FATFS fs;		/* File system object */
	FIL file;		/* File object */
	BYTE fat_buff[128];	/* File read buffer */

	f_mount(0, &Fatfs);	/* Register volume work area (never fails) */
	
}

int main(void)
{
	char command[80];
	char arguments[80];
	clock_setup();
	gpio_setup();
	rtc_auto_awake(LSE, 0x7fff);
	usart_setup();
	writeln("Hello All");
	systic_setup();

	rtc_awake_from_standby();

	/* Todo */
	rtc_clear_flag(RTC_ALR);
	
	delay_us(1000000);
	/* delay(1000); */
	write_time();

	test_sd();

	while (1) {
		if(line_available()) {
			proccess_command(readln(), command, arguments);
			rcv_command(command, arguments);
		}
	}

	return 0;
}
