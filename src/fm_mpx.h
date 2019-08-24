/*
    PiFmAdv - Advanced FM transmitter for the Raspberry Pi
    Copyright (C) 2017 Miegl

    See https://github.com/Miegl/PiFmAdv
*/

#define DATA_SIZE 1000

extern int fm_mpx_open(char *filename, size_t len, int cutoff_freq, int preemphasis_corner_freq, float mpx, int rds_on, int wait_for_audio, int srate, int nochan);
extern int fm_mpx_get_samples(float *mpx_buffer);
extern int fm_mpx_close();
