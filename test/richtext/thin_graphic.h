/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef THIN_GRAPHIC_H
#define THIN_GRAPHIC_H

#include <SDL.h>

#include "graphic/igraphic.h"
#include "graphic/iblitable_surface.h"


class ThinSDLSurface : public IBlitableSurface {
public:
	ThinSDLSurface(SDL_Surface * surf, bool free_pixels);
	virtual ~ThinSDLSurface();

	virtual uint32_t get_w() const {return surf_->w;}
	virtual uint32_t get_h() const {return surf_->h;}

	virtual uint16_t get_pitch() const {return surf_->pitch;}
	virtual uint8_t * get_pixels() const {
		return static_cast<uint8_t*>(surf_->pixels);
	}

	void lock() const {SDL_LockSurface(surf_);}
	void unlock() const {SDL_UnlockSurface(surf_);}
	void blit(const Point& dst, const IPicture* src, const Rect& srcrc, Composite cm);
	void fill_rect(const Rect& rc, RGBAColor clr);

private:
	SDL_Surface* surf_;
	bool free_pixels_;
};

IGraphic * create_thin_graphic();

#endif /* end of include guard: THIN_GRAPHIC_H */

