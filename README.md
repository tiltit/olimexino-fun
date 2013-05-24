OLIMEXINO FUN
=============

This is my take on getting the Olimexino-stm32 from Olimex up and running using the [libopencm3](http://elm-chan.org/fsw/ff/00index_e.html) libraries. The code verry messy and needs a serious cleanup. 
So far I got to read data from an sd card using [Chan's FatFs](http://elm-chan.org/fsw/ff/00index_e.html) module. The code also demonstrates how to sent the stm32f1 into standby and using the real time clock.

Right now the binary is uploaded not with the bootloader albeit with an ARM-USB-OCD-H Jtag debuger from Olimex.