#ifndef CLOWNCD_LIBSNDFILE_H
#define CLOWNCD_LIBSNDFILE_H

#include <stddef.h>

#include <sndfile.h>

#include "clowncommon/clowncommon.h"

#include "file-io.h"

typedef struct ClownCD_libSndFile
{
	SNDFILE *libsndfile;
} ClownCD_libSndFile;

cc_bool ClownCD_libSndFileOpen(ClownCD_libSndFile *libsndfile, ClownCD_File *file);
void ClownCD_libSndFileClose(ClownCD_libSndFile *libsndfile);

cc_bool ClownCD_libSndFileSeek(ClownCD_libSndFile *libsndfile, size_t frame);
size_t ClownCD_libSndFileRead(ClownCD_libSndFile *libsndfile, short *buffer, size_t total_frames);

#endif /* CLOWNCD_LIBSNDFILE_H */
