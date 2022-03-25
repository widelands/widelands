/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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

#include "editor/ui_menus/field_info_window.h"

#include "graphic/text_layout.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"

constexpr UI::FontStyle font_style = UI::FontStyle::kWuiInfoPanelParagraph;
constexpr Duration kUpdateTimeInGametimeMs(500);  //  half a second, gametime
FieldInfoWindow::FieldInfoWindow(EditorInteractive& parent,
                                 UI::UniqueWindow::Registry& registry,
                                 int32_t const x,
                                 int32_t const y,
                                 const Widelands::NodeAndTriangle<>& center,
                                 const Widelands::Field& f,
                                 const Widelands::Field& tf,
                                 Widelands::Map* map)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      "tool_options_menu",
                      &registry,
                      x,
                      y,
                      width,
                      height,
                      _("Field Information")),
     parent_(parent),
     center_(center),
     f_(f),
     tf_(tf),
     map_(map),
     multiline_textarea_(this, 0, 0, get_inner_w(), get_inner_h(), UI::PanelStyle::kWui),
     lastupdate_(0) {

	update();
	set_center_panel(&multiline_textarea_);
	initialization_complete();
}
void FieldInfoWindow::update() {
	std::string all_infos;
	add_node_info(all_infos);
	add_caps_info(all_infos);
	add_owner_info(all_infos);
	add_terrain_info(all_infos);
	add_mapobject_info(all_infos);
	add_resources_info(all_infos);
	add_map_info(all_infos);

	const std::string richtext = as_richtext(all_infos);

	if (richtext != multiline_textarea_.get_text()) {
		multiline_textarea_.set_text(richtext);
	}
}

void FieldInfoWindow::add_node_info(std::string& buf) const {
	buf += as_heading(_("Node"), UI::PanelStyle::kWui, true);
	buf += as_listitem(
	   format(_("Coordinates: (%1$i, %2$i)"), center_.node.x, center_.node.y), font_style);
}

void FieldInfoWindow::add_caps_info(std::string& buf) const {
	std::vector<std::string> caps_strings;
	Widelands::NodeCaps const caps = f_.nodecaps();
	switch (caps & Widelands::BUILDCAPS_SIZEMASK) {

	case Widelands::BUILDCAPS_SMALL: {
		/** TRANSLATORS: Editor terrain property: small building plot */
		caps_strings.emplace_back(_("small"));
	} break;

	case Widelands::BUILDCAPS_MEDIUM: {
		/** TRANSLATORS: Editor terrain property: medium building plot */
		caps_strings.emplace_back(_("medium"));
	} break;
	case Widelands::BUILDCAPS_BIG: {
		/** TRANSLATORS: Editor terrain property: big building plot */
		caps_strings.emplace_back(_("big"));
	} break;
	default:
		break;
	}

	if ((caps & Widelands::BUILDCAPS_FLAG) != 0) {
		/** TRANSLATORS: Editor terrain property: space for a flag */
		caps_strings.emplace_back(_("flag"));
	}
	if ((caps & Widelands::BUILDCAPS_MINE) != 0) {
		/** TRANSLATORS: Editor terrain property: mine building plot */
		caps_strings.emplace_back(_("mine"));
	}
	if ((caps & Widelands::BUILDCAPS_PORT) != 0) {
		/** TRANSLATORS: Editor terrain property: port space */
		caps_strings.emplace_back(_("port"));
	}
	if ((caps & Widelands::MOVECAPS_WALK) != 0) {
		/** TRANSLATORS: Editor terrain property: units can walk on this terrain */
		caps_strings.emplace_back(_("walkable"));
	}
	if ((caps & Widelands::MOVECAPS_SWIM) != 0) {
		/** TRANSLATORS: Editor terrain property: units can swim on this terrain (fish, ships) */
		caps_strings.emplace_back(_("swimmable"));
	}

	buf += as_listitem(
	   format(_("Caps: %s"), i18n::localize_list(caps_strings, i18n::ConcatenateWith::COMMA)),
	   font_style);
}

void FieldInfoWindow::add_owner_info(std::string& buf) const {
	if (f_.get_owned_by() > 0) {
		buf +=
		   as_listitem(format(_("Owned by: Player %u"), static_cast<unsigned int>(f_.get_owned_by())),
		               font_style);
	} else {
		buf += as_listitem(_("Owned by: —"), font_style);
	}
}

void FieldInfoWindow::add_terrain_info(std::string& buf) const {
	buf += as_heading(_("Terrain"), UI::PanelStyle::kWui);

	const Widelands::TerrainDescription* ter = parent_.egbase().descriptions().get_terrain_descr(
	   center_.triangle.t == Widelands::TriangleIndex::D ? tf_.terrain_d() : tf_.terrain_r());

	buf += as_listitem(format(pgettext("terrain_name", "Name: %s"), ter->descname()), font_style);

	std::vector<std::string> terrain_is_strings;
	for (const Widelands::TerrainDescription::Type& terrain_type : ter->get_types()) {
		terrain_is_strings.emplace_back(terrain_type.descname);
	}

	buf += as_listitem(
	   format(
	      /** TRANSLATORS: "Is" is a list of terrain properties, e.g. "arable, unreachable and
	       * unwalkable". You can also translate this as "Category: %s" or "Property: %s" */
	      _("Is: %s"), i18n::localize_list(terrain_is_strings, i18n::ConcatenateWith::AMPERSAND)),
	   font_style);
}

void FieldInfoWindow::add_mapobject_info(std::string& buf) const {
	const Widelands::BaseImmovable* immovable = f_.get_immovable();
	Widelands::Bob* bob = f_.get_first_bob();
	if ((immovable != nullptr) || (bob != nullptr)) {
		/** TRANSLATORS: Heading for immovables and animals in editor info tool */
		buf += as_heading(_("Objects"), UI::PanelStyle::kWui);
		if (immovable != nullptr) {
			buf += as_listitem(format(_("Immovable: %s"), immovable->descr().descname()), font_style);
		}

		if (bob != nullptr) {
			// Collect bobs
			std::vector<std::string> critternames;
			std::vector<std::string> shipnames;
			std::vector<std::string> workernames;
			do {
				switch (bob->descr().type()) {
				case (Widelands::MapObjectType::CRITTER):
					critternames.push_back(bob->descr().descname());
					break;
				case (Widelands::MapObjectType::SHIP): {
					if (upcast(Widelands::Ship, ship, bob)) {
						shipnames.push_back(ship->get_shipname());
					}
				} break;
				case (Widelands::MapObjectType::WORKER):
				case (Widelands::MapObjectType::CARRIER):
				case (Widelands::MapObjectType::SOLDIER):
				case (Widelands::MapObjectType::FERRY):
					workernames.push_back(bob->descr().descname());
					break;
				default:
					NEVER_HERE();
				}
			} while ((bob = bob->get_next_bob()) != nullptr);

			// Add bobs
			if (!critternames.empty()) {
				buf +=
				   as_listitem(format(_("Animals: %s"),
				                      i18n::localize_list(critternames, i18n::ConcatenateWith::COMMA)),
				               font_style);
			}
			if (!workernames.empty()) {
				buf +=
				   as_listitem(format(_("Workers: %s"),
				                      i18n::localize_list(workernames, i18n::ConcatenateWith::COMMA)),
				               font_style);
			}
			if (!shipnames.empty()) {
				buf += as_listitem(
				   format(_("Ships: %s"), i18n::localize_list(shipnames, i18n::ConcatenateWith::COMMA)),
				   font_style);
			}
		}
	}
}

void FieldInfoWindow::add_resources_info(std::string& buf) const {
	Widelands::ResourceAmount ramount = f_.get_resources_amount();
	if (ramount > 0) {
		buf += as_heading(_("Resources"), UI::PanelStyle::kWui);
		buf += as_listitem(
		   format(pgettext("resources", "%1%× %2%"), static_cast<unsigned int>(ramount),
		          parent_.egbase().descriptions().get_resource_descr(f_.get_resources())->descname()),
		   font_style);
	}
}

void FieldInfoWindow::add_map_info(std::string& buf) const {
	buf += as_heading(_("Map"), UI::PanelStyle::kWui);
	buf += as_listitem(
	   format(pgettext("map_name", "Name: %s"), richtext_escape(map_->get_name())), font_style);
	buf +=
	   as_listitem(format(_("Size: %1% × %2%"), map_->get_width(), map_->get_height()), font_style);

	if (map_->get_nrplayers() > 0) {
		buf += as_listitem(
		   format(_("Players: %u"), static_cast<unsigned int>(map_->get_nrplayers())), font_style);
	} else {
		buf += as_listitem(_("Players: –"), font_style);
	}

	buf += as_listitem(format(_("Author: %s"), richtext_escape(map_->get_author())), font_style);
	buf += as_listitem(
	   format(_("Description: %s"), richtext_escape(map_->get_description())), font_style);

	{
		std::string addons;
		const size_t nr_addons = parent_.egbase().enabled_addons().size();
		if (nr_addons == 0) {
			/** TRANSLATORS: No add-ons enabled */
			addons = _("(none)");
		} else {
			for (size_t i = 0; i < nr_addons; ++i) {
				if (addons.empty()) {
					addons = parent_.egbase().enabled_addons()[i]->descname();
				} else {
					addons =
					   format(_("%1$s; %2$s"), addons, parent_.egbase().enabled_addons()[i]->descname());
				}
			}
		}
		buf += as_listitem(format(_("Enabled Add-Ons: %s"), richtext_escape(addons)), font_style);
	}
}

void FieldInfoWindow::think() {
	if ((parent_.egbase().get_gametime() - lastupdate_) > kUpdateTimeInGametimeMs) {
		update();
		lastupdate_ = parent_.egbase().get_gametime();
	}

	UI::UniqueWindow::think();
}
