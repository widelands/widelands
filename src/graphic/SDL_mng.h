/*
    SDL_mng:  A library to load MNG files
    Copyright (C) 2003, Thomas Kircher

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __SDL_mng_h__
#define __SDL_mng_h__

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	unsigned int Frame_width;
	unsigned int Frame_height;
	unsigned int Ticks_per_second;
	unsigned int Nominal_layer_count;
	unsigned int Nominal_frame_count;
	unsigned int Nominal_play_time;
	unsigned int Simplicity_profile;
}
MHDR_chunk;

/* MNG_Image type */
typedef struct
{
	MHDR_chunk    mhdr;
	unsigned int  frame_count;
	SDL_Surface * * frame;
}
MNG_Image;

/* Check for MNG signature */
int IMG_isMNG(SDL_RWops *src);

/* Read and return an MNG image */
MNG_Image *IMG_loadMNG(const char *file);

#ifdef __cplusplus
}
#endif

#endif /* __SDL_mng_h__ */
