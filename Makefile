BINARY = olimexino_fun

CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-gcc
CP      = arm-none-eabi-objcopy
OD      = arm-none-eabi-objdump

# Uncomment this line if you want to use the installed (not local) library.
#TOOLCHAIN_DIR := $(shell dirname `which $(CC)`)/../$(PREFIX)
TOOLCHAIN_DIR   = libopencm3

CFLAGS          += -Os -g -Wall -Wextra -I$(TOOLCHAIN_DIR)/include \
                   -fno-common $(ARCH_FLAGS) -MD -DSTM32F1
ARCH_FLAGS      = -mthumb -mcpu=cortex-m3 -msoft-float
LDSCRIPT        ?= $(BINARY).ld

LDFLAGS         += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group \
                   -L$(TOOLCHAIN_DIR)/lib \
                   -T $(LDSCRIPT) -nostartfiles -Wl,--gc-sections \
                   $(ARCH_FLAGS) -mfix-cortex-m3-ldrd


all: test

clean:
	rm -f *.lst *.o *.elf *.bin *.hex *.d

test:$(BINARY).hex

ff.o: ff.c
	@ echo "[Compiling ff.c]"
	$(CC) -g -Wall -pedantic -I./ -c $(CFLAGS) ff.c -Os

mmc.o: mmc.c
	@ echo "[Compiling mmc.c]"
	$(CC) -g -Wall -pedantic -I./ -c $(CFLAGS) mmc.c -Os

events.o: events.c
	@ echo "[Compiling events.c]"
	$(CC) -g -Wall -pedantic -I./ -c $(CFLAGS) events.c -Os

time.o: time.c
	@ echo "[Compiling time.c]"
	$(CC) -g -Wall -pedantic -I./ -c $(CFLAGS) time.c -Os

serial.o: serial.c
	@ echo "[Compiling serial.c]"
	$(CC) -g -Wall -pedantic -I./ -c $(CFLAGS) serial.c -Os

power.o: power.c
	@ echo "[Compiling power.c]"
	$(CC) -g -Wall -pedantic -I./ -c $(CFLAGS) power.c -Os

main.o: main.c
	@ echo "[Compiling main code]"
	$(CC) -g -Wall -pedantic -I./ -c $(CFLAGS) main.c  -Os

$(BINARY).elf: main.o serial.o time.o power.o events.o mmc.o ff.o
	@ echo "[Linking]"
	$(LD)   main.o serial.o time.o power.o events.o mmc.o ff.o  -o $(BINARY).elf -lopencm3_stm32f1  $(LDFLAGS)
#	$(Q)$(LD) -o $(*).elf $(OBJS) -lopencm3_stm32f1 $(LDFLAGS)

$(BINARY).bin: $(BINARY).elf
	@ echo "[Copying]"
	$(CP) -Obinary  $(BINARY).elf $(BINARY).bin
	$(OD) -S $(BINARY).elf > $(BINARY).lst

$(BINARY).hex: $(BINARY).elf
	$(CP) -Oihex $(BINARY).elf $(BINARY).hex


# Using the ARM-USB-OCD-H JTAG dongle.
flash: test
	openocd -f openocd.cfg \
                    -c "init" -c "reset init" \
                    -c "stm32f1x mass_erase 0" \
                    -c "flash write_image $(BINARY).hex" \
                    -c "reset" \
                    -c "shutdown" $(NULL)
