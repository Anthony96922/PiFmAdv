/*
    PiFmAdv - Advanced FM transmitter for the Raspberry Pi
    Copyright (C) 2017 Miegl

    See https://github.com/Miegl/PiFmAdv
*/

extern int fm_mpx_open(char *filename, size_t len, int wait_for_audio, int sample_rate, int num_chans);
extern int fm_mpx_get_samples(float *mpx_buffer);
extern int fm_mpx_close();
