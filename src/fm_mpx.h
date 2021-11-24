/*
    PiFmAdv - Advanced FM transmitter for the Raspberry Pi
    Copyright (C) 2017 Miegl

    See https://github.com/Miegl/PiFmAdv
*/

#define DATA_SIZE 4096

extern int fm_mpx_open(char *filename, float ppm);
extern int fm_mpx_get_samples(float *mpx_buffer);
extern void fm_mpx_close();
