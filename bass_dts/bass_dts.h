#ifndef BASSDTS_H
#define BASSDTS_H

#include "bass.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BASSDTSDEF
#define BASSDTSDEF(f) WINAPI f
#else
#define NOBASSDTSOVERLOADS
#endif

// BASS_CHANNELINFO type
#define BASS_CTYPE_STREAM_DTS 0x1f300

HSTREAM BASSDTSDEF(BASS_DTS_StreamCreateFile)(DWORD filetype, const void* file, QWORD offset, QWORD length, DWORD flags);
HSTREAM BASSDTSDEF(BASS_DTS_StreamCreateFileUser)(DWORD system, DWORD flags, const BASS_FILEPROCS *procs, void *user);

#ifdef __cplusplus
}

#if defined(_WIN32) && !defined(NOBASSDTSOVERLOADS)
static inline HSTREAM BASS_DTS_StreamCreateFile(DWORD filetype, const WCHAR *file, QWORD offset, QWORD length, DWORD flags)
{
	return BASS_DTS_StreamCreateFile(filetype, (const void*)file, offset, length, flags | BASS_UNICODE);
}
#endif
#endif

#endif
