/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/minimap.h"

#include <memory>

#include <SDL_mouse.h>

#include "base/i18n.h"
#include "graphic/minimap_renderer.h"
#include "graphic/rendertarget.h"
#include "graphic/texture.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "wui/interactive_player.h"

MiniMap::View::View(UI::Panel& parent,
                    MiniMapLayer* flags,
                    MiniMapType* type,
                    int32_t const x,
                    int32_t const y,
                    uint32_t const /* w */,
                    uint32_t const /* h */,
                    InteractiveBase& ibase)
   : UI::Panel(&parent, UI::PanelStyle::kWui, x, y, 10, 10),
     ibase_(ibase),
     pic_map_spot_(g_image_cache->get("images/wui/overlays/map_spot.png")),

     minimap_layers_(flags),
     minimap_type_(type) {
}

void MiniMap::View::set_view(const Rectf& view_area) {
	view_area_ = view_area;
}

void MiniMap::View::reset() {
	minimap_image_static_.reset();
}

void MiniMap::View::draw(RenderTarget& dst) {
	if (!minimap_image_static_) {
		// Draw the entire minimap from the beginning.
		minimap_image_static_ =
		   create_minimap_empty(ibase_.egbase(), *minimap_layers_ | MiniMapLayer::ViewWindow);
		draw_minimap_static(*minimap_image_static_, ibase_.egbase(), ibase_.get_player(),
		                    *minimap_layers_ | MiniMapLayer::ViewWindow);
	} else {
		// Just update a part of the minimap.
		draw_minimap_static(*minimap_image_static_, ibase_.egbase(), ibase_.get_player(),
		                    *minimap_layers_ | MiniMapLayer::ViewWindow, false, &rows_drawn_);
	}
	minimap_image_final_ =
	   draw_minimap_final(*minimap_image_static_, ibase_.egbase(), view_area_, *minimap_type_,
	                      *minimap_layers_ | MiniMapLayer::ViewWindow);
	dst.blit(Vector2i::zero(), minimap_image_final_.get());
}

/*
===============
Left-press: warp the view point to the new position
===============
*/
bool MiniMap::View::handle_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT) {
		return false;
	}

	dynamic_cast<MiniMap&>(*get_parent())
	   .warpview(minimap_pixel_to_mappixel(ibase_.egbase().map(), Vector2i(x, y), view_area_,
	                                       *minimap_type_,
	                                       (*minimap_layers_ & MiniMapLayer::Zoom2) != 0));
	return true;
}

void MiniMap::View::set_zoom(const bool zoom) {
	const Widelands::Map& map = ibase_.egbase().map();
	set_size(map.get_width() * scale_map(map, zoom), map.get_height() * scale_map(map, zoom));
	// The texture needs to be recreated when the size changes.
	reset();
}

bool MiniMap::View::can_zoom() {
	const Widelands::Map& map = ibase_.egbase().map();
	// The zoomed MiniMap needs to fit into: height - windows boarders - button height. -> 60px
	const auto scale = scale_map(map, true);
	return (scale > 1 && map.get_width() * scale <= ibase_.get_w() - 60 &&
	        map.get_height() * scale <= ibase_.get_h() - 60);
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
	const int kNumberOfButtons = (ibase_.egbase().map().allows_seafaring() ? 9 : 8);
	return view_.get_w() < kNumberOfButtons * 20 ? 3 : kNumberOfButtons;
}
inline uint32_t MiniMap::number_of_button_rows() const {
	// Use multi row layout if there is not enough width.
	const int kNumberOfButtons = (ibase_.egbase().map().allows_seafaring() ? 9 : 8);
	return view_.get_w() < kNumberOfButtons * 20 ? 3 : 1;
}
inline uint32_t MiniMap::but_w() const {
	return view_.get_w() / number_of_buttons_per_row();
}
inline uint32_t MiniMap::but_h() const {
	return 20;
}
MiniMap::MiniMap(InteractiveBase& ibase, Registry* const registry)
   : UI::UniqueWindow(&ibase, UI::WindowStyle::kWui, "minimap", registry, 0, 0, _("Map")),
     ibase_(ibase),
     owner_button_impl_(ibase.egbase().is_game() ? MiniMapLayer::Owner :
                                                   MiniMapLayer::StartingPositions),
     additional_button_impl_(ibase.egbase().is_game() ? MiniMapLayer::Traffic :
                                                        MiniMapLayer::Artifacts),
     view_(*this, &registry->minimap_layers, &registry->minimap_type, 0, 0, 0, 0, ibase),

     button_terrain(this,
                    "terrain",
                    but_w() * 0,
                    view_.get_h() + but_h() * 0,
                    but_w(),
                    but_h(),
                    UI::ButtonStyle::kWuiSecondary,
                    g_image_cache->get("images/wui/minimap/button_terrn.png"),
                    _("Terrain"),
                    UI::Button::VisualState::kRaised,
                    UI::Button::ImageMode::kUnscaled),
     button_owner(this,
                  "owner",
                  but_w() * 1,
                  view_.get_h() + but_h() * 0,
                  but_w(),
                  but_h(),
                  UI::ButtonStyle::kWuiSecondary,
                  g_image_cache->get("images/wui/minimap/button_owner.png"),
                  _("Owner"),
                  UI::Button::VisualState::kRaised,
                  UI::Button::ImageMode::kUnscaled),
     button_flags(this,
                  "flags",
                  but_w() * 2,
                  view_.get_h() + but_h() * 0,
                  but_w(),
                  but_h(),
                  UI::ButtonStyle::kWuiSecondary,
                  g_image_cache->get("images/wui/minimap/button_flags.png"),
                  _("Flags"),
                  UI::Button::VisualState::kRaised,
                  UI::Button::ImageMode::kUnscaled),
     button_roads(this,
                  "roads",
                  but_w() * 0,
                  view_.get_h() + but_h() * 1,
                  but_w(),
                  but_h(),
                  UI::ButtonStyle::kWuiSecondary,
                  g_image_cache->get("images/wui/minimap/button_roads.png"),
                  _("Roads"),
                  UI::Button::VisualState::kRaised,
                  UI::Button::ImageMode::kUnscaled),
     button_buildings(this,
                      "buildings",
                      but_w() * 1,
                      view_.get_h() + but_h() * 1,
                      but_w(),
                      but_h(),
                      UI::ButtonStyle::kWuiSecondary,
                      g_image_cache->get("images/wui/minimap/button_bldns.png"),
                      _("Buildings"),
                      UI::Button::VisualState::kRaised,
                      UI::Button::ImageMode::kUnscaled),
     button_ships(this,
                  "ships",
                  but_w() * 2,
                  view_.get_h() + but_h(),
                  but_w(),
                  but_h(),
                  UI::ButtonStyle::kWuiSecondary,
                  g_image_cache->get("images/wui/ship/ship_scout_se.png"),
                  _("Ships"),
                  UI::Button::VisualState::kRaised,
                  UI::Button::ImageMode::kUnscaled),
     button_traffic_artifacts(this,
                              "traffic_artifacts",
                              but_w() * 2,
                              view_.get_h() + but_h(),
                              but_w(),
                              but_h(),
                              UI::ButtonStyle::kWuiSecondary,
                              ibase.egbase().is_game() ?
                                 g_image_cache->get("images/wui/buildings/menu_list_workers.png") :
                                 g_image_cache->get("images/wui/minimap/button_artifacts.png"),
                              ibase.egbase().is_game() ? _("High Traffic") : _("Artifacts"),
                              UI::Button::VisualState::kRaised,
                              UI::Button::ImageMode::kUnscaled),
     button_attack(this,
                   "attack",
                   but_w() * 2,
                   view_.get_h() + but_h(),
                   but_w(),
                   but_h(),
                   UI::ButtonStyle::kWuiSecondary,
                   g_image_cache->get("images/wui/messages/messages_warfare.png"),
                   _("Attacks"),
                   UI::Button::VisualState::kRaised,
                   UI::Button::ImageMode::kUnscaled),
     button_zoom(this,
                 "zoom",
                 but_w() * 3,
                 view_.get_h() + but_h() * 1,
                 but_w(),
                 but_h(),
                 UI::ButtonStyle::kWuiSecondary,
                 g_image_cache->get("images/wui/minimap/button_zoom.png"),
                 _("Zoom"),
                 UI::Button::VisualState::kRaised,
                 UI::Button::ImageMode::kUnscaled) {
	button_terrain.sigclicked.connect([this]() { toggle(MiniMapLayer::Terrain); });
	button_owner.sigclicked.connect([this]() { toggle(owner_button_impl_); });
	button_flags.sigclicked.connect([this]() { toggle(MiniMapLayer::Flag); });
	button_roads.sigclicked.connect([this]() { toggle(MiniMapLayer::Road); });
	button_buildings.sigclicked.connect([this]() { toggle(MiniMapLayer::Building); });
	button_ships.sigclicked.connect([this]() { toggle(MiniMapLayer::Ship); });
	button_traffic_artifacts.sigclicked.connect([this]() { toggle(additional_button_impl_); });
	button_attack.sigclicked.connect([this]() { toggle(MiniMapLayer::Attack); });
	button_zoom.sigclicked.connect([this]() { toggle(MiniMapLayer::Zoom2); });

	check_boundaries();

	if (get_usedefaultpos()) {
		center_to_parent();
	}

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& /* note */) { check_boundaries(); });

	update_button_permpressed();

	initialization_complete();
}

void MiniMap::toggle(MiniMapLayer const button) {
	*view_.minimap_layers_ = MiniMapLayer(*view_.minimap_layers_ ^ button);
	// Redraw the entire minimap when changing layers - this looks nicer.
	view_.reset();
	if (button == MiniMapLayer::Zoom2) {
		resize();
	}
	update_button_permpressed();
}

void MiniMap::resize() {
	view_.set_zoom((*view_.minimap_layers_ & MiniMapLayer::Zoom2) != 0);
	// Read number of rows after the zoom.
	const uint32_t rows = number_of_button_rows();
	const bool seafaring = ibase_.egbase().map().allows_seafaring();
	uint32_t height_offset = 0;
	if (rows > 1) {
		height_offset = 1;
	}
	set_inner_size(view_.get_w(), view_.get_h() + rows * but_h());
	button_terrain.set_pos(Vector2i(but_w() * 0, view_.get_h()));
	button_terrain.set_size(but_w(), but_h());
	button_owner.set_pos(Vector2i(but_w() * 1, view_.get_h()));
	button_owner.set_size(but_w(), but_h());
	button_flags.set_pos(Vector2i(but_w() * 2, view_.get_h()));
	button_flags.set_size(but_w(), but_h());
	button_roads.set_pos(
	   Vector2i(but_w() * (3 - 3 * height_offset), view_.get_h() + but_h() * height_offset));
	button_roads.set_size(but_w(), but_h());
	button_buildings.set_pos(
	   Vector2i(but_w() * (4 - 3 * height_offset), view_.get_h() + but_h() * height_offset));
	button_buildings.set_size(but_w(), but_h());
	button_traffic_artifacts.set_pos(
	   Vector2i(but_w() * (5 - 3 * height_offset), view_.get_h() + but_h() * height_offset));
	button_traffic_artifacts.set_size(but_w(), but_h());
	button_attack.set_pos(
	   Vector2i(but_w() * (6 - 3 * 2 * height_offset), view_.get_h() + but_h() * 2 * height_offset));
	button_attack.set_size(but_w(), but_h());
	button_ships.set_pos(
	   Vector2i(but_w() * (7 - 3 * 2 * height_offset), view_.get_h() + but_h() * 2 * height_offset));
	button_ships.set_size(but_w(), but_h());
	if (seafaring) {
		button_zoom.set_pos(Vector2i(
		   but_w() * (8 - 3 * 2 * height_offset), view_.get_h() + but_h() * 2 * height_offset));
		button_zoom.set_size(but_w(), but_h());
	} else {
		button_zoom.set_pos(Vector2i(
		   but_w() * (7 - 3 * 2 * height_offset), view_.get_h() + but_h() * 2 * height_offset));
		button_zoom.set_size(but_w(), but_h());
	}
	button_zoom.set_enabled(view_.can_zoom());

	button_ships.set_visible(ibase_.egbase().map().allows_seafaring());

	move_inside_parent();
}

void MiniMap::update_button_permpressed() {
	button_terrain.set_perm_pressed((*view_.minimap_layers_ & MiniMapLayer::Terrain) != 0);
	button_owner.set_perm_pressed((*view_.minimap_layers_ & owner_button_impl_) != 0);
	button_flags.set_perm_pressed((*view_.minimap_layers_ & MiniMapLayer::Flag) != 0);
	button_roads.set_perm_pressed((*view_.minimap_layers_ & MiniMapLayer::Road) != 0);
	button_buildings.set_perm_pressed((*view_.minimap_layers_ & MiniMapLayer::Building) != 0);
	button_ships.set_perm_pressed((*view_.minimap_layers_ & MiniMapLayer::Ship) != 0);
	button_traffic_artifacts.set_perm_pressed((*view_.minimap_layers_ & additional_button_impl_) !=
	                                          0);
	button_attack.set_perm_pressed((*view_.minimap_layers_ & MiniMapLayer::Attack) != 0);
	button_zoom.set_perm_pressed((*view_.minimap_layers_ & MiniMapLayer::Zoom2) != 0);
}

void MiniMap::check_boundaries() {
	if (!view_.can_zoom() && ((*view_.minimap_layers_ & MiniMapLayer::Zoom2) != 0)) {
		toggle(MiniMapLayer::Zoom2);
	} else {
		resize();
	}
}

bool MiniMap::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	// pass wheel events to parent
	if (!ibase_.map_view()->handle_mousewheel(x, y, modstate)) {
		ibase_.handle_mousewheel(x, y, modstate);
	}

	// but don't pass them to other underlying windows
	return true;
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& MiniMap::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r = ib.minimap_registry_;
			r.create();
			assert(r.window);
			MiniMap& m = dynamic_cast<MiniMap&>(*r.window);
			*m.view_.minimap_layers_ = static_cast<MiniMapLayer>(fr.unsigned_32());
			m.view_.reset();
			m.resize();
			m.update_button_permpressed();
			return m;
		}
		throw Widelands::UnhandledVersionError("Minimap", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("minimap: %s", e.what());
	}
}
void MiniMap::save(FileWrite& fw, Widelands::MapObjectSaver& /* mos */) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_32(static_cast<uint32_t>(*view_.minimap_layers_));
}
