/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "graphic/minimap_renderer.h"
#include "graphic/rendertarget.h"
#include "graphic/texture.h"
#include "logic/map.h"
#include "wui/interactive_player.h"
#include "wui/mapviewpixelconstants.h"

MiniMap::View::View(UI::Panel& parent,
                    MiniMapLayer* flags,
                    MiniMapType* type,
                    int32_t const x,
                    int32_t const y,
                    uint32_t const,
                    uint32_t const,
                    InteractiveBase& ibase)
   : UI::Panel(&parent, x, y, 10, 10),
     ibase_(ibase),
     pic_map_spot_(g_gr->images().get("images/wui/overlays/map_spot.png")),
     minimap_layers_(flags),
     minimap_type_(type) {
}

void MiniMap::View::set_view(const Rectf& view_area) {
	view_area_ = view_area;
}

void MiniMap::View::draw(RenderTarget& dst) {
	minimap_image_ = draw_minimap(ibase_.egbase(), ibase_.get_player(), view_area_, *minimap_type_,
	                              *minimap_layers_ | MiniMapLayer::ViewWindow);
	dst.blit(Vector2f(), minimap_image_.get());
}

/*
===============
Left-press: warp the view point to the new position
===============
*/
bool MiniMap::View::handle_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT)
		return false;

	dynamic_cast<MiniMap&>(*get_parent())
	   .warpview(minimap_pixel_to_mappixel(ibase_.egbase().map(), Vector2i(x, y), view_area_,
	                                       *minimap_type_, *minimap_layers_ & MiniMapLayer::Zoom2));
	return true;
}

void MiniMap::View::set_zoom(int32_t z) {
	const Widelands::Map& map = ibase_.egbase().map();
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
inline uint32_t MiniMap::number_of_buttons_per_row() const {
	return 3;
}
inline uint32_t MiniMap::number_of_button_rows() const {
	return 2;
}
inline uint32_t MiniMap::but_w() const {
	return view_.get_w() / number_of_buttons_per_row();
}
inline uint32_t MiniMap::but_h() const {
	return 20;
}
MiniMap::MiniMap(InteractiveBase& ibase, Registry* const registry)
   : UI::UniqueWindow(&ibase, "minimap", registry, 0, 0, _("Map")),
     view_(*this, &registry->minimap_layers, &registry->minimap_type, 0, 0, 0, 0, ibase),

     button_terrn(this,
                  "terrain",
                  but_w() * 0,
                  view_.get_h() + but_h() * 0,
                  but_w(),
                  but_h(),
                  g_gr->images().get("images/wui/button_secondary.png"),
                  g_gr->images().get("images/wui/minimap/button_terrn.png"),
                  _("Terrain"),
                  UI::Button::Style::kRaised,
                  UI::Button::ImageMode::kUnscaled),
     button_owner(this,
                  "owner",
                  but_w() * 1,
                  view_.get_h() + but_h() * 0,
                  but_w(),
                  but_h(),
                  g_gr->images().get("images/wui/button_secondary.png"),
                  g_gr->images().get("images/wui/minimap/button_owner.png"),
                  _("Owner"),
                  UI::Button::Style::kRaised,
                  UI::Button::ImageMode::kUnscaled),
     button_flags(this,
                  "flags",
                  but_w() * 2,
                  view_.get_h() + but_h() * 0,
                  but_w(),
                  but_h(),
                  g_gr->images().get("images/wui/button_secondary.png"),
                  g_gr->images().get("images/wui/minimap/button_flags.png"),
                  _("Flags"),
                  UI::Button::Style::kRaised,
                  UI::Button::ImageMode::kUnscaled),
     button_roads(this,
                  "roads",
                  but_w() * 0,
                  view_.get_h() + but_h() * 1,
                  but_w(),
                  but_h(),
                  g_gr->images().get("images/wui/button_secondary.png"),
                  g_gr->images().get("images/wui/minimap/button_roads.png"),
                  _("Roads"),
                  UI::Button::Style::kRaised,
                  UI::Button::ImageMode::kUnscaled),
     button_bldns(this,
                  "buildings",
                  but_w() * 1,
                  view_.get_h() + but_h() * 1,
                  but_w(),
                  but_h(),
                  g_gr->images().get("images/wui/button_secondary.png"),
                  g_gr->images().get("images/wui/minimap/button_bldns.png"),
                  _("Buildings"),
                  UI::Button::Style::kRaised,
                  UI::Button::ImageMode::kUnscaled),
     button_zoom(this,
                 "zoom",
                 but_w() * 2,
                 view_.get_h() + but_h() * 1,
                 but_w(),
                 but_h(),
                 g_gr->images().get("images/wui/button_secondary.png"),
                 g_gr->images().get("images/wui/minimap/button_zoom.png"),
                 _("Zoom"),
                 UI::Button::Style::kRaised,
                 UI::Button::ImageMode::kUnscaled) {
	button_terrn.sigclicked.connect(
	   boost::bind(&MiniMap::toggle, boost::ref(*this), MiniMapLayer::Terrain));
	button_owner.sigclicked.connect(
	   boost::bind(&MiniMap::toggle, boost::ref(*this), MiniMapLayer::Owner));
	button_flags.sigclicked.connect(
	   boost::bind(&MiniMap::toggle, boost::ref(*this), MiniMapLayer::Flag));
	button_roads.sigclicked.connect(
	   boost::bind(&MiniMap::toggle, boost::ref(*this), MiniMapLayer::Road));
	button_bldns.sigclicked.connect(
	   boost::bind(&MiniMap::toggle, boost::ref(*this), MiniMapLayer::Building));
	button_zoom.sigclicked.connect(
	   boost::bind(&MiniMap::toggle, boost::ref(*this), MiniMapLayer::Zoom2));

	resize();

	update_button_permpressed();

	if (get_usedefaultpos())
		center_to_parent();
}

void MiniMap::toggle(MiniMapLayer const button) {
	*view_.minimap_layers_ = MiniMapLayer(*view_.minimap_layers_ ^ button);
	if (button == MiniMapLayer::Zoom2)
		resize();
	update_button_permpressed();
}

void MiniMap::resize() {
	view_.set_zoom(*view_.minimap_layers_ & MiniMapLayer::Zoom2 ? 2 : 1);
	set_inner_size(view_.get_w(), view_.get_h() + number_of_button_rows() * but_h());
	button_terrn.set_pos(Vector2i(but_w() * 0, view_.get_h() + but_h() * 0));
	button_terrn.set_size(but_w(), but_h());
	button_owner.set_pos(Vector2i(but_w() * 1, view_.get_h() + but_h() * 0));
	button_owner.set_size(but_w(), but_h());
	button_flags.set_pos(Vector2i(but_w() * 2, view_.get_h() + but_h() * 0));
	button_flags.set_size(but_w(), but_h());
	button_roads.set_pos(Vector2i(but_w() * 0, view_.get_h() + but_h() * 1));
	button_roads.set_size(but_w(), but_h());
	button_bldns.set_pos(Vector2i(but_w() * 1, view_.get_h() + but_h() * 1));
	button_bldns.set_size(but_w(), but_h());
	button_zoom.set_pos(Vector2i(but_w() * 2, view_.get_h() + but_h() * 1));
	button_zoom.set_size(but_w(), but_h());
	move_inside_parent();
}

void MiniMap::update_button_permpressed() {
	button_terrn.set_perm_pressed(*view_.minimap_layers_ & MiniMapLayer::Terrain);
	button_owner.set_perm_pressed(*view_.minimap_layers_ & MiniMapLayer::Owner);
	button_flags.set_perm_pressed(*view_.minimap_layers_ & MiniMapLayer::Flag);
	button_roads.set_perm_pressed(*view_.minimap_layers_ & MiniMapLayer::Road);
	button_bldns.set_perm_pressed(*view_.minimap_layers_ & MiniMapLayer::Building);
	button_zoom.set_perm_pressed(*view_.minimap_layers_ & MiniMapLayer::Zoom2);
}
