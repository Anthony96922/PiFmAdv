/*
    PiFmAdv - Advanced FM transmitter for the Raspberry Pi
    Copyright (C) 2017 Miegl

    See https://github.com/Miegl/PiFmAdv
*/

#include <sndfile.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#include "fm_mpx.h"

#define FIR_HALF_SIZE	64
#define FIR_SIZE	(2*FIR_HALF_SIZE-1)

// coefficients of the low-pass FIR filter
float low_pass_fir[FIR_HALF_SIZE];
float fir_buffer[FIR_SIZE];
int fir_index;

size_t length;
float upsample_factor;

float *audio_buffer;

int audio_index;
int audio_len;
float audio_pos;

int channels;

SNDFILE *inf;

float *alloc_empty_buffer(size_t length) {
    float *p = malloc(length * sizeof(float));
    if(p == NULL) return NULL;

    bzero(p, length * sizeof(float));

    return p;
}

int fm_mpx_open(char *filename, size_t len, int sample_rate, int num_chans) {
	length = len;

	if(filename != NULL) {
		// Open the input file
		SF_INFO sfinfo;

		// stdin or file on the filesystem?
		if(filename[0] == '-') {
			if (sample_rate > 0 && num_chans > 0) {
				printf("Using stdin for raw audio input at %d Hz.\n", sample_rate);
				sfinfo.samplerate = sample_rate;
				sfinfo.channels = num_chans;
				sfinfo.format = SF_FORMAT_RAW | SF_FORMAT_PCM_16;
			}
			if(!(inf = sf_open_fd(fileno(stdin), SFM_READ, &sfinfo, 0))) {
				fprintf(stderr, "Error: could not open stdin for audio input.\n");
				return -1;
			} else {
				printf("Using stdin for audio input.\n");
			}
		} else {
			if(!(inf = sf_open(filename, SFM_READ, &sfinfo))) {
				fprintf(stderr, "Error: could not open input file %s.\n", filename);
				return -1;
			} else {
				printf("Using audio file: %s\n", filename);
			}
		}

		int in_samplerate = sfinfo.samplerate;
		upsample_factor = 192000. / in_samplerate;

		printf("Input: %d Hz, upsampling factor: %.2f\n", in_samplerate, upsample_factor);

		channels = sfinfo.channels;

		// Create the low-pass FIR filter
		int cutoff_freq = in_samplerate;
		// Here we divide this coefficient by two because it will be counted twice
		// when applying the filter
		low_pass_fir[FIR_HALF_SIZE-1] = 2 * cutoff_freq / 192000 /2;

		// Only store half of the filter since it is symmetric
		for(int i=1; i<FIR_HALF_SIZE; i++) {
			low_pass_fir[FIR_HALF_SIZE-1-i] =
				sin(2 * M_PI * cutoff_freq * i / 192000) / (M_PI * i) // sinc
				* (.54 - .46 * cos(2 * M_PI * (i+FIR_HALF_SIZE) / (2*FIR_HALF_SIZE))); // Hamming window
		}

		printf("Created low-pass FIR filter for audio channels, with cutoff at %d Hz\n", cutoff_freq);

		audio_pos = upsample_factor;
		audio_buffer = alloc_empty_buffer(length * channels);
		if(audio_buffer == NULL) return -1;
	}

	return 0;
}

int fm_mpx_get_samples(float *mpx_buffer) {

	for(int i=0; i<length; i++) {
		if(audio_pos >= upsample_factor) {
			audio_pos -= upsample_factor;

			if(audio_len <= channels) {
				for(int j=0; j<2; j++) { // one retry
					audio_len = sf_read_float(inf, audio_buffer, length);
					if (audio_len < 0) {
						fprintf(stderr, "Error reading audio\n");
						return -1;
					} else if(audio_len == 0) {
						if( sf_seek(inf, 0, SEEK_SET) < 0 ) break;
					} else {
						break;
					}
				}
				audio_index = 0;
			} else {
				audio_index += channels;
				audio_len -= channels;
			}
		}
		audio_pos++;

		// First store the current sample(s) into the FIR filter's ring buffer
		if(channels == 2) {
			// downmix stereo to mono
			fir_buffer[fir_index] = (audio_buffer[audio_index] + audio_buffer[audio_index+1]) / 2;
		} else {
			fir_buffer[fir_index] = audio_buffer[audio_index];
		}
		fir_index++;
		if(fir_index >= FIR_SIZE) fir_index = 0;

		// Now apply the FIR low-pass filter

		/* As the FIR filter is symmetric, we do not multiply all
		   the coefficients independently, but two-by-two, thus reducing
		   the total number of multiplications by a factor of two
		 */
		float out = 0;
		int ifbi = fir_index;  // ifbi = increasing FIR Buffer Index
		int dfbi = fir_index;  // dfbi = decreasing FIR Buffer Index
		for(int fi=0; fi<FIR_HALF_SIZE; fi++) {  // fi = Filter Index
			dfbi--;
			if(dfbi < 0) dfbi = FIR_SIZE-1;
			out += low_pass_fir[fi] * (fir_buffer[ifbi] + fir_buffer[dfbi]);
			ifbi++;
			if(ifbi >= FIR_SIZE) ifbi = 0;
		}
		// End of FIR filter

		mpx_buffer[i] = out;
	}

	return 0;
}


int fm_mpx_close() {
	if(sf_close(inf) ) {
		fprintf(stderr, "Error closing audio file");
	}

	if(audio_buffer != NULL) free(audio_buffer);

	return 0;
}
