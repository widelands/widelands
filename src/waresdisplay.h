/*
 * Copyright (C) 2003, 2006-2007 by the Widelands Development Team
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

#ifndef include_waresdisplay_h
#define include_waresdisplay_h

#include "warelist.h"

#include "ui_textarea.h"

#include <vector>

namespace UI {struct Textarea;};

namespace Widelands {
struct Tribe_Descr;
struct WareList;
};

/*
class WaresDisplay
------------------
Panel that displays the contents of a WareList.
*/
struct WaresDisplay : public UI::Panel {
	enum {
		Width = 5 * 24 + 4 * 4, //  (5 wares icons) + space in between

		WaresPerRow = 5,
	};

   enum wdType {
      WORKER,
      WARE
	};

public:
	WaresDisplay
		(UI::Panel * const parent,
		 int32_t const x, int32_t const y, Widelands::Tribe_Descr const &);
	virtual ~WaresDisplay();

	bool handle_mousemove(const Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);

   void add_warelist(Widelands::WareList const *, wdType);
   void remove_all_warelists();

protected:
	virtual void draw(RenderTarget* dst);
	virtual void draw_ware
		(RenderTarget &, const Point,
		 const uint32_t id,
		 const uint32_t stock,
		 const bool worker);

private:
	typedef std::vector<Widelands::WareList const *> vector_type;

	Widelands::Tribe_Descr const & m_tribe;
	UI::Textarea        m_curware;
	wdType              m_type;
	vector_type         m_warelists;
};

#endif // include_waresdisplay_h
