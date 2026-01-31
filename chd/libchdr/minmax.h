/* license:BSD-3-Clause
 * copyright-holders:Aaron Giles
 ***************************************************************************

    minmax.h

***************************************************************************/

#ifndef LIBCHDR_MINMAX_H
#define LIBCHDR_MINMAX_H

#if defined(RARCH_INTERNAL) || defined(__LIBRETRO__)
#include "../retro_miscellaneous.h"
#else
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#endif /* LIBCHDR_MINMAX_H */
