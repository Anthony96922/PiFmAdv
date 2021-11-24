/*
    PiFmAdv - Advanced FM transmitter for the Raspberry Pi
    Copyright (C) 2017 Miegl

    See https://github.com/Miegl/PiFmAdv
*/

#include <sndfile.h>
#include <stdlib.h>
#include <string.h>
#include <samplerate.h>
#include "fm_mpx.h"

static float input_buffer[DATA_SIZE];

static SNDFILE *inf;

// SRC
static SRC_STATE *resampler;
static SRC_DATA resampler_data;

int fm_mpx_open(char *filename, float ppm) {
	// Open the input file
	SF_INFO sfinfo;

	// stdin or file on the filesystem?
	if(strcmp(filename, "-") == 0) {
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

	if (sfinfo.channels != 1) {
		fprintf(stderr, "Input must have only one channel\n");
		return -1;
	}

	resampler_data.data_in = input_buffer;
	resampler_data.output_frames = DATA_SIZE * 16;
	resampler_data.src_ratio = (float)192000 / sfinfo.samplerate + (ppm / 1e6);

	int src_error;
	if ((resampler = src_new(SRC_ZERO_ORDER_HOLD, 1, &src_error)) == NULL) {
		fprintf(stderr, "Error: src_new failed: %s\n", src_strerror(src_error));
		return -1;
	}

	return 0;
}

int fm_mpx_get_samples(float *mpx_buffer) {
	int audio_len;
	int frames_to_read = DATA_SIZE;
	int buffer_offset = 0;

	while (frames_to_read) {
		if ((audio_len = sf_readf_float(inf, input_buffer + buffer_offset, frames_to_read)) < 0) {
			fprintf(stderr, "Error reading audio\n");
			return -1;
		}

		buffer_offset += audio_len;
		frames_to_read -= audio_len;
		// Check if we have more audio
		if (audio_len == 0 && sf_seek(inf, 0, SEEK_SET) < 0) {
			fprintf(stderr, "Could not rewind in audio file, terminating\n");
			return -1;
		}
	}

	resampler_data.input_frames = audio_len;
	resampler_data.data_out = mpx_buffer;

	int src_error;
	if ((src_error = src_process(resampler, &resampler_data))) {
		fprintf(stderr, "Error: src_process failed: %s\n", src_strerror(src_error));
		return -1;
	}

	audio_len = resampler_data.output_frames_gen;

	return audio_len;
}


void fm_mpx_close() {
	if (sf_close(inf)) fprintf(stderr, "Error closing audio file");
	src_delete(resampler);
}
