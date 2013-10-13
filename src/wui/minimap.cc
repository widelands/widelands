/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wui/minimap.h"

#include <memory>

#include "graphic/graphic.h"
#include "graphic/in_memory_image.h"
#include "graphic/render/minimaprenderer.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "i18n.h"
#include "logic/map.h"
#include "wui/interactive_player.h"
#include "wui/mapviewpixelconstants.h"


MiniMap::View::View
	(UI::Panel & parent, int8_t * flags,
	 int32_t const x, int32_t const y, uint32_t const, uint32_t const,
	 Interactive_Base & ibase)
:
	UI::Panel       (&parent, x, y, 10, 10),
	m_ibase       (ibase),
	m_viewx       (0),
	m_viewy       (0),
	m_pic_map_spot(g_gr->images().get("pics/map_spot.png")),
	m_flags       (flags)
{}


/** MiniMap::View::set_view_pos(int32_t x, int32_t y)
 *
 * Set the view point marker to a new position.
 *
 * Args: x, y  new view point coordinates, in screen coordinates
 */
void MiniMap::View::set_view_pos(const int32_t x, const int32_t y)
{
	m_viewx = x / TRIANGLE_WIDTH;
	m_viewy = y / TRIANGLE_HEIGHT;

	update();
}


void MiniMap::View::draw(RenderTarget & dst)
{
	MiniMapRenderer mmr;

	std::unique_ptr<Surface> surface
		(mmr.get_minimap_image
			(m_ibase.egbase(),
			m_ibase.get_player(),
			(*m_flags) & (MiniMap::Zoom2) ?
				Point((m_viewx - get_w() / 4), (m_viewy - get_h() / 4)):
				Point((m_viewx - get_w() / 2), (m_viewy - get_h() / 2)),
			*m_flags));
	// Give ownership of the surface to the new image
	std::unique_ptr<const Image> im(new_in_memory_image("minimap", surface.release()));
	dst.blit(Point(), im.get());
	im.reset();
}


/*
===============
Left-press: warp the view point to the new position
===============
*/
bool MiniMap::View::handle_mousepress(const Uint8 btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT)
		return false;

	//  calculates the coordinates corresponding to the mouse position
	Widelands::Coords c;
	if (*m_flags & MiniMap::Zoom2)
		c = Widelands::Coords
			(m_viewx + 1 - (get_w() / 2 - x) / 2,
			 m_viewy + 1 - (get_h() / 2 - y) / 2);
	else
		c = Widelands::Coords
			(m_viewx + 1 - get_w() / 2 + x, m_viewy + 1 - get_h() / 2 + y);

	m_ibase.egbase().map().normalize_coords(c);

	ref_cast<MiniMap, UI::Panel>(*get_parent()).warpview(c.x * TRIANGLE_WIDTH, c.y * TRIANGLE_HEIGHT);

	return true;
}
bool MiniMap::View::handle_mouserelease(Uint8 const btn, int32_t, int32_t) {
	return btn == SDL_BUTTON_LEFT;
}

void MiniMap::View::set_zoom(int32_t z) {
	const Widelands::Map & map = m_ibase.egbase().map();
	set_size((map.get_width() * z), (map.get_height()) * z);
}


/*
==============================================================================

MiniMap

==============================================================================
*/

/*
===============
Initialize the minimap window. Dimensions will be set automatically
according to the map size.
A registry pointer is set to track the MiniMap object (only show one
minimap at a time).

reg, the registry pointer will be set by constructor and cleared by
destructor
===============
*/
inline uint32_t MiniMap::number_of_buttons_per_row() const {return 3;}
inline uint32_t MiniMap::number_of_button_rows    () const {return 2;}
inline uint32_t MiniMap::but_w                    () const {
	return m_view.get_w() / number_of_buttons_per_row();
}
inline uint32_t MiniMap::but_h                    () const {return 20;}
MiniMap::MiniMap(Interactive_Base & ibase, Registry * const registry)
:
	UI::UniqueWindow(&ibase, "minimap", registry, 0, 0, _("Map")),
	m_view(*this, &registry->flags, 0, 0, 0, 0, ibase),

	button_terrn
		(this, "terrain",
		 but_w() * 0, m_view.get_h() + but_h() * 0, but_w(), but_h(),
		 g_gr->images().get("pics/but0.png"),
		 g_gr->images().get("pics/button_terrn.png"),
		 _("Terrain")),
	button_owner
		(this, "owner",
		 but_w() * 1, m_view.get_h() + but_h() * 0, but_w(), but_h(),
		 g_gr->images().get("pics/but0.png"),
		 g_gr->images().get("pics/button_owner.png"),
		 _("Owner")),
	button_flags
		(this, "flags",
		 but_w() * 2, m_view.get_h() + but_h() * 0, but_w(), but_h(),
		 g_gr->images().get("pics/but0.png"),
		 g_gr->images().get("pics/button_flags.png"),
		 _("Flags")),
	button_roads
		(this, "roads",
		 but_w() * 0, m_view.get_h() + but_h() * 1, but_w(), but_h(),
		 g_gr->images().get("pics/but0.png"),
		 g_gr->images().get("pics/button_roads.png"),
		 _("Roads")),
	button_bldns
		(this, "buildings",
		 but_w() * 1, m_view.get_h() + but_h() * 1, but_w(), but_h(),
		 g_gr->images().get("pics/but0.png"),
		 g_gr->images().get("pics/button_bldns.png"),
		 _("Buildings")),
	button_zoom
		(this, "zoom",
		 but_w() * 2, m_view.get_h() + but_h() * 1, but_w(), but_h(),
		 g_gr->images().get("pics/but0.png"),
		 g_gr->images().get("pics/button_zoom.png"),
		 _("Zoom"))
{
	button_terrn.sigclicked.connect(boost::bind(&MiniMap::toggle, boost::ref(*this), Terrn));
	button_owner.sigclicked.connect(boost::bind(&MiniMap::toggle, boost::ref(*this), Owner));
	button_flags.sigclicked.connect(boost::bind(&MiniMap::toggle, boost::ref(*this), Flags));
	button_roads.sigclicked.connect(boost::bind(&MiniMap::toggle, boost::ref(*this), Roads));
	button_bldns.sigclicked.connect(boost::bind(&MiniMap::toggle, boost::ref(*this), Bldns));
	button_zoom.sigclicked.connect(boost::bind(&MiniMap::toggle, boost::ref(*this), Zoom2));

	set_cache(false);

	resize();

	update_button_permpressed();

	if (get_usedefaultpos())
		center_to_parent();
}


void MiniMap::toggle(Layers const button) {
	*m_view.m_flags ^= button;
	if (button == Zoom2)
		resize();
	update_button_permpressed();
}

void MiniMap::resize() {
	m_view.set_zoom(*m_view.m_flags & Zoom2 ? 2 : 1);
	set_inner_size
		(m_view.get_w(), m_view.get_h() + number_of_button_rows() * but_h());
	button_terrn.set_pos(Point(but_w() * 0, m_view.get_h() + but_h() * 0));
	button_terrn.set_size(but_w(), but_h());
	button_owner.set_pos(Point(but_w() * 1, m_view.get_h() + but_h() * 0));
	button_owner.set_size(but_w(), but_h());
	button_flags.set_pos(Point(but_w() * 2, m_view.get_h() + but_h() * 0));
	button_flags.set_size(but_w(), but_h());
	button_roads.set_pos(Point(but_w() * 0, m_view.get_h() + but_h() * 1));
	button_roads.set_size(but_w(), but_h());
	button_bldns.set_pos(Point(but_w() * 1, m_view.get_h() + but_h() * 1));
	button_bldns.set_size(but_w(), but_h());
	button_zoom .set_pos(Point(but_w() * 2, m_view.get_h() + but_h() * 1));
	button_zoom .set_size(but_w(), but_h());
	move_inside_parent();
}

// Makes the buttons reflect the selected layers
void MiniMap::update_button_permpressed() {
	button_terrn.set_perm_pressed(*m_view.m_flags & Terrn);
	button_owner.set_perm_pressed(*m_view.m_flags & Owner);
	button_flags.set_perm_pressed(*m_view.m_flags & Flags);
	button_roads.set_perm_pressed(*m_view.m_flags & Roads);
	button_bldns.set_perm_pressed(*m_view.m_flags & Bldns);
	button_zoom .set_perm_pressed(*m_view.m_flags & Zoom2);
}
