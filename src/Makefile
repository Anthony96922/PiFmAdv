CC = gcc
STD_CFLAGS = -Wall -std=gnu99 -c -O2

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
else
	CFLAGS = $(STD_CFLAGS)
	TARGET = other
endif

ifneq ($(TARGET), other)

pi_fm_adv: rds.o waveforms.o pi_fm_adv.o fm_mpx.o control_pipe.o mailbox.o
	$(CC) -o pi_fm_adv rds.o waveforms.o mailbox.o pi_fm_adv.o fm_mpx.o control_pipe.o -lm -lsndfile

endif

rds_wav: rds.o waveforms.o rds_wav.o fm_mpx.o
	$(CC) -o rds_wav rds_wav.o rds.o waveforms.o fm_mpx.o -lm -lsndfile

clean:
	rm -f *.o
