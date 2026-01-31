#ifndef MINIZ_FAKE_ZLIB_H
#define MINIZ_FAKE_ZLIB_H

#include <stddef.h>

#include "miniz.h"

/* A grotty little hack to account for a difference between zlib's and miniz's APIs. */
#undef uInt
#define uInt size_t

#endif /* MINIZ_FAKE_ZLIB_H */
