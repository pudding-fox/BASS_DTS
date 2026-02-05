#pragma once

#include "bass-addon.h"
#include "dts_file.h"

typedef struct {
	int bits_per_sample;
	int bytes_per_sample;
	int samples_per_frame;
	double max_value;
} AUDIO_FORMAT;

typedef void(*PCM_WRITE_SAMPLE)(void* const buffer, const int position, const int sample);

typedef struct {
	int channel_count;
	int sample_rate;
	DTS_FILE* dts_file;
	struct dcadec_context* dcadec_context;
	int** samples;
	int sample_count;
	int sample_position;
	PCM_WRITE_SAMPLE write_sample;
	AUDIO_FORMAT input_format;
	AUDIO_FORMAT output_format;
} DTS_STREAM;

BOOL dts_stream_create(const BASSFILE file, const int flags, DTS_STREAM** const stream);

BOOL dts_stream_update(DTS_STREAM* const stream);

BOOL dts_stream_update_info(DTS_STREAM* const stream);

DWORD dts_stream_read(DTS_STREAM* const stream, void* buffer, const DWORD length);

BOOL dts_stream_reset(DTS_STREAM* const stream, BOOL clear_context);

BOOL dts_stream_can_seek(DTS_STREAM* const stream, const QWORD position);

BOOL dts_stream_seek(DTS_STREAM* const stream, const QWORD position);

QWORD dts_stream_length(const DTS_STREAM* const stream);

BOOL dts_stream_free(DTS_STREAM* const stream);