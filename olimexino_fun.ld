/* Define memory regions. STM32F103RBT6 */
MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 128K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Include the common ld script. */
INCLUDE libopencm3/lib/stm32/f1/libopencm3_stm32f1.ld

