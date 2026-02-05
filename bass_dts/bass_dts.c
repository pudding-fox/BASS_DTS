#include "bass_dts.h"

#include "dts_file.h"
#include "dts_stream.h"
#include "pcm.h"
#include "buffer.h"

#include "bass-addon.h"
const BASS_FUNCTIONS* bassfunc;

//2.4.1.0
#define BASSDTSVERSION 0x02040100

//I have no idea how to prevent linking against this routine in msvcrt.
//It doesn't exist on Windows XP.
//Hopefully it doesn't do anything important.
int _except_handler4_common() {
	return 0;
}

HSTREAM WINAPI BASS_DTS_StreamCreate(BASSFILE file, DWORD flags);
DWORD WINAPI BASS_DTS_StreamProc(HSTREAM handle, void* buffer, DWORD length, void* user);
BOOL WINAPI BASS_DTS_StreamWrite(HSTREAM handle, void* buffer, DWORD* position, DWORD* remaining, void* user);
QWORD WINAPI BASS_DTS_GetLength(void* inst, DWORD mode);
VOID WINAPI BASS_DTS_GetInfo(void* inst, BASS_CHANNELINFO* info);
BOOL WINAPI BASS_DTS_CanSetPosition(void* inst, QWORD position, DWORD mode);
QWORD WINAPI BASS_DTS_SetPosition(void* inst, QWORD position, DWORD mode);
VOID WINAPI BASS_DTS_Free(void* inst);

//We just privode the minimum functions, the NULL slots are optional.
const ADDON_FUNCTIONS addon_functions = {
	0,
	&BASS_DTS_Free,
	&BASS_DTS_GetLength,
	NULL,
	NULL,
	&BASS_DTS_GetInfo,
	&BASS_DTS_CanSetPosition,
	&BASS_DTS_SetPosition,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static const BASS_PLUGINFORM plugin_form[] = {
	{ BASS_CTYPE_STREAM_DTS, "DTS", "*.dts" }
};

static const BASS_PLUGININFO plugin_info = { BASSDTSVERSION, 1, plugin_form };

BOOL WINAPI DllMain(HANDLE dll, DWORD reason, LPVOID reserved) {
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls((HMODULE)dll);
		if (HIWORD(BASS_GetVersion()) != BASSVERSION || !GetBassFunc()) {
			MessageBoxA(0, "Incorrect BASS.DLL version (" BASSVERSIONTEXT " is required)", "BASS", MB_ICONERROR | MB_OK);
			return FALSE;
		}
		break;
	}
	return TRUE;
}

const VOID* WINAPI BASSplugin(DWORD face) {
	switch (face) {
	case BASSPLUGIN_INFO:
		return (void*)&plugin_info;
	case BASSPLUGIN_CREATE:
		return (void*)&BASS_DTS_StreamCreate;
	}
	return NULL;
}

HSTREAM WINAPI BASS_DTS_StreamCreate(BASSFILE file, DWORD flags) {
	HSTREAM handle;
	DTS_STREAM* dts_stream;
	if (!dts_stream_create(file, 0, &dts_stream)) {
		error(BASS_ERROR_FILEFORM);
	}
	if (flags & BASS_SAMPLE_FLOAT) {
		dts_stream->output_format.bits_per_sample = sizeof(float) * 8;
		dts_stream->output_format.bytes_per_sample = sizeof(float);
	}
	else {
		dts_stream->output_format.bits_per_sample = sizeof(short) * 8;
		dts_stream->output_format.bytes_per_sample = sizeof(short);
	}

	if (!(dts_stream->write_sample = pcm_write_sample(dts_stream->input_format, dts_stream->output_format))) {
		//Sample conversion is not implemented :c
		dts_stream_free(dts_stream);
		error(BASS_ERROR_FORMAT);
	}

	handle = bassfunc->CreateStream(
		dts_stream->sample_rate,
		dts_stream->channel_count,
		flags,
		&BASS_DTS_StreamProc,
		dts_stream,
		&addon_functions
	);
	if (handle == 0) {
		dts_stream_free(dts_stream);
		return 0;
	}

	bassfunc->file.SetStream(file, handle);
	noerrorn(handle);
}

HSTREAM WINAPI BASS_DTS_StreamCreateFile(DWORD filetype, const void* file, QWORD offset, QWORD length, DWORD flags) {
	HSTREAM handle;
	BASSFILE bass_file = bassfunc->file.Open(filetype, file, offset, length, flags, BASSFILE_EX_TAGS);
	if (!bass_file) {
		return 0;
	}
	handle = BASS_DTS_StreamCreate(bass_file, flags);
	if (!handle) {
		bassfunc->file.Close(bass_file);
		return 0;
	}
	return handle;
}

HSTREAM WINAPI BASS_DTS_StreamCreateFileUser(DWORD system, DWORD flags, const BASS_FILEPROCS* procs, void* user) {
	HSTREAM handle;
	BASSFILE bass_file;
	if (system != STREAMFILE_NOBUFFER) error(BASS_ERROR_ILLPARAM); // no buffered streaming
	bass_file = bassfunc->file.OpenUser(system, flags, procs, user, BASSFILE_EX_TAGS);
	if (!bass_file) {
		return 0;
	}
	handle = BASS_DTS_StreamCreate(bass_file, flags);
	if (!handle) {
		bassfunc->file.Close(bass_file);
		return 0;
	}
	return handle;
}

DWORD WINAPI BASS_DTS_StreamProc(HSTREAM handle, void* buffer, DWORD length, void* user) {
	DTS_STREAM* dts_stream = user;
	DWORD position = 0;
	DWORD remaining = length;
	while (remaining > 0) {
		//Make sure samples are available.
		if (!dts_stream->samples || !dts_stream->sample_count) {
			if (!dts_stream_update(dts_stream)) {
				//Reached the end of the file (or some catastrophic failure to synchronize).
				return BASS_STREAMPROC_END;
			}
		}
		if (!BASS_DTS_StreamWrite(handle, buffer, &position, &remaining, user)) {
			break;
		}
	}
	//If remaining > 0 it's a buffer underrun, ASIO won't be happy. 
	//Nothing we can do.
	return length - remaining;
}

BOOL WINAPI BASS_DTS_StreamWrite(HSTREAM handle, void* buffer, DWORD* position, DWORD* remaining, void* user) {
	DTS_STREAM* dts_stream = user;
	//length = the amount of data from the current decodeded samples available.
	DWORD length =
		(dts_stream->sample_count - dts_stream->sample_position) *
		dts_stream->output_format.bytes_per_sample *
		dts_stream->channel_count;

	//we usually have more data than can be written.
	if (length > *remaining) {
		length = *remaining;
	}

	if (!(length = dts_stream_read(dts_stream, offset_buffer(buffer, *position), length))) {
		//Failed to read *any* data. Probably something wrong.
		return FALSE;
	}

	*position += length;
	*remaining -= length;
	return TRUE;
}

QWORD WINAPI BASS_DTS_GetLength(void* inst, DWORD mode) {
	DTS_STREAM* dts_stream = inst;
	if (mode == BASS_POS_BYTE) {
		noerrorn(dts_stream_length(dts_stream));
	}
	else {
		errorn(BASS_ERROR_NOTAVAIL);
	}
}

VOID WINAPI BASS_DTS_GetInfo(void* inst, BASS_CHANNELINFO* info) {
	DTS_STREAM* dts_stream = inst;
	info->ctype = BASS_CTYPE_STREAM_DTS;
	info->origres = dts_stream->input_format.bits_per_sample;
}

BOOL WINAPI BASS_DTS_CanSetPosition(void* inst, QWORD position, DWORD mode) {
	DTS_STREAM* dts_stream = inst;
	if (mode == BASS_POS_BYTE) {
		return dts_stream_can_seek(dts_stream, position);
	}
	else {
		error(BASS_ERROR_NOTAVAIL);
	}
}

QWORD WINAPI BASS_DTS_SetPosition(void* inst, QWORD position, DWORD mode) {
	DTS_STREAM* dts_stream = inst;
	if (mode == BASS_POS_BYTE) {
		if (dts_stream_seek(dts_stream, position)) {
			if (dts_stream_reset(dts_stream, TRUE)) {
				return position;
			}
		}
	}
	errorn(BASS_ERROR_NOTAVAIL);
}

VOID WINAPI BASS_DTS_Free(void* inst) {
	DTS_STREAM* dts_stream = inst;
	dts_stream_free(dts_stream);
}