#include "picture.h"

#include "surface.h"
#include "rendertarget.h"

Picture::~Picture() {
	if (surface)
		delete surface;
	surface = 0;
	if (fname)
		delete fname;
	fname = 0;
	if (rendertarget)
		delete rendertarget;
	rendertarget = 0;
}

