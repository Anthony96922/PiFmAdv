CC = gcc
STD_CFLAGS = -Wall -O2 -pedantic

# Enable ARM-specific options only on ARM, and compilation of the app only on ARM
UNAME := $(shell uname -m)
PCPUI := $(shell grep Revision /proc/cpuinfo | cut -c16-)

# Determine the hardware platform. Below, pi1 stands for the RaspberryPi 1 (the original one),
# and pi2 stands for both the RaspberryPi 2 and 3.
ifeq ($(UNAME), armv6l)
	CFLAGS = $(STD_CFLAGS) -mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp -DRASPI=1
	TARGET = pi1
else ifeq ($(UNAME), armv7l)
	CFLAGS = $(STD_CFLAGS) -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 -DRASPI=2
	TARGET = pi2
else ifeq ($(PCPUI), 11)
	CFLAGS = $(STD_CFLAGS) -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 -DRASPI=4
	TARGET = pi4
endif

pi_fm_adv: pi_fm_adv.o fm_mpx.o mailbox.o
	$(CC) -o pi_fm_adv mailbox.o pi_fm_adv.o fm_mpx.o -lm -lsndfile

fm_mpx.o: fm_mpx.h
mailbox.o: mailbox.h
pi_fm_adv.o: fm_mpx.h mailbox.h

clean:
	rm -f *.o
