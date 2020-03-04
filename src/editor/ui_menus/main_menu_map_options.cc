/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "editor/ui_menus/main_menu_map_options.h"

#include <cstdio>
#include <string>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/text_layout.h"
#include "logic/map.h"
#include "logic/note_map_options.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/textarea.h"
#include "wlapplication_options.h"
#include "wui/map_tags.h"

inline EditorInteractive& MainMenuMapOptions::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

/**
 * Create all the buttons etc...
 */
MainMenuMapOptions::MainMenuMapOptions(EditorInteractive& parent, Registry& registry)
   : UI::UniqueWindow(
        &parent, "map_options", &registry, 350, parent.get_inner_h() - 80, _("Map Options")),
     padding_(4),
     indent_(10),
     labelh_(text_height(UI::FontStyle::kLabel) + 4),
     checkbox_space_(25),
     butw_((get_inner_w() - 3 * padding_) / 2),
     max_w_(get_inner_w() - 2 * padding_),
     ok_(this,
         "ok",
         UI::g_fh->fontset()->is_rtl() ? padding_ : butw_ + 2 * padding_,
         get_inner_h() - padding_ - labelh_,
         butw_,
         labelh_,
         UI::ButtonStyle::kWuiPrimary,
         _("OK")),
     cancel_(this,
             "cancel",
             UI::g_fh->fontset()->is_rtl() ? butw_ + 2 * padding_ : padding_,
             get_inner_h() - padding_ - labelh_,
             butw_,
             labelh_,
             UI::ButtonStyle::kWuiSecondary,
             _("Cancel")),
     tab_box_(this, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
     tabs_(&tab_box_, UI::TabPanelStyle::kWuiLight),

     main_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
     tags_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
     teams_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),

     name_(&main_box_, 0, 0, max_w_, UI::PanelStyle::kWui),
     author_(&main_box_, 0, 0, max_w_, UI::PanelStyle::kWui),
     size_(&main_box_, 0, 0, max_w_ - indent_, labelh_, ""),
     balancing_dropdown_(&tags_box_,
                         "dropdown_balancing",
                         0,
                         0,
                         200,
                         50,
                         24,
                         "",
                         UI::DropdownType::kTextual,
                         UI::PanelStyle::kWui,
                         UI::ButtonStyle::kWuiSecondary),
     teams_list_(
        &teams_box_, 0, 0, max_w_, 60, UI::PanelStyle::kWui, UI::ListselectLayout::kShowCheck),
     registry_(registry) {

	tab_box_.set_size(max_w_, get_inner_h() - labelh_ - 2 * padding_);
	tabs_.set_size(max_w_, tab_box_.get_inner_h());
	main_box_.set_size(max_w_, tabs_.get_inner_h() - 35);
	tags_box_.set_size(max_w_, main_box_.get_h());
	teams_box_.set_size(max_w_, main_box_.get_h());

	// Calculate the overall remaining space for MultilineEditboxes.
	uint32_t remaining_space = main_box_.get_inner_h() - 7 * labelh_ - 5 * indent_;

	// We need less space for the hint and the description, but it should at least have 1 line
	// height.
	hint_ = new UI::MultilineEditbox(
	   &main_box_, 0, 0, max_w_, std::max(labelh_, remaining_space * 1 / 3), UI::PanelStyle::kWui);
	descr_ = new UI::MultilineEditbox(
	   &main_box_, 0, 0, max_w_, remaining_space - hint_->get_h(), UI::PanelStyle::kWui);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Map name:")));
	main_box_.add(&name_);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Authors:")));
	main_box_.add(&author_);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Description:")));
	main_box_.add(descr_);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Hint (optional):")));
	main_box_.add(hint_);
	main_box_.add_space(indent_);

	main_box_.add(&size_);
	main_box_.add_space(indent_);

	tags_box_.add(new UI::Textarea(&tags_box_, 0, 0, max_w_, labelh_, _("Tags:")));
	add_tag_checkbox(&tags_box_, "ffa", localize_tag("ffa"));
	add_tag_checkbox(&tags_box_, "1v1", localize_tag("1v1"));
	add_tag_checkbox(&tags_box_, "2teams", localize_tag("2teams"));
	add_tag_checkbox(&tags_box_, "3teams", localize_tag("3teams"));
	add_tag_checkbox(&tags_box_, "4teams", localize_tag("4teams"));

	balancing_dropdown_.set_autoexpand_display_button();
	balancing_dropdown_.add(localize_tag("balanced"), "balanced");
	balancing_dropdown_.add(localize_tag("unbalanced"), "unbalanced");
	tags_box_.add(&balancing_dropdown_);

	tags_box_.add_space(labelh_);

	tags_box_.add(new UI::Textarea(&tags_box_, 0, 0, max_w_, labelh_, _("Waterway length limit:")));
	UI::Box* ww_box = new UI::Box(&tags_box_, 0, 0, UI::Box::Horizontal, max_w_, checkbox_space_, 0);
	waterway_length_box_ =
	   new UI::SpinBox(ww_box, 0, 0, max_w_, max_w_ / 2, 1, 1, std::numeric_limits<int32_t>::max(),
	                   UI::PanelStyle::kWui, std::string(), UI::SpinBox::Units::kFields);
	/** TRANSLATORS: Map Options: Waterways are disabled */
	waterway_length_box_->add_replacement(1, _("Disabled"));
	ww_box->add(waterway_length_box_, UI::Box::Resizing::kFullSize);
	ww_box->add_space(checkbox_space_);
	tags_box_.add(ww_box);
	tags_box_.add_space(padding_);

	teams_box_.add(new UI::Textarea(&teams_box_, 0, 0, max_w_, labelh_, _("Suggested Teams:")));
	teams_box_.add(&teams_list_);
	// TODO(GunChleoc): We need team images in the listselect here,
	// so map editors will be able to delete entries.
	// This is waiting for the new RT renderer.
	// TODO(Notabilis): Add onChanged-code below after this is added
	teams_list_.add("Not implemented yet.", "", nullptr, false);

	unsigned int nr_players = static_cast<unsigned int>(eia().egbase().map().get_nrplayers());
	std::string players =
	   (boost::format(ngettext("%u Player", "%u Players", nr_players)) % nr_players).str();
	teams_box_.add(new UI::Textarea(&teams_box_, 0, 0, max_w_, labelh_, players));

	tab_box_.add(&tabs_, UI::Box::Resizing::kFullSize);
	tabs_.add("main_map_options", g_gr->images().get("images/wui/menus/toggle_minimap.png"),
	          &main_box_, _("Main Options"));
	tabs_.add("map_tags", g_gr->images().get("images/ui_basic/checkbox_checked.png"), &tags_box_,
	          _("Tags"));
	tabs_.add("map_teams", g_gr->images().get("images/wui/editor/tools/players.png"), &teams_box_,
	          _("Teams"));

	name_.changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	author_.changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	descr_->changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	hint_->changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	waterway_length_box_->changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	for (const auto& tag : tags_checkboxes_) {
		tag.second->changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	}

	balancing_dropdown_.selected.connect([this] { changed(); });

	ok_.sigclicked.connect(boost::bind(&MainMenuMapOptions::clicked_ok, boost::ref(*this)));
	cancel_.sigclicked.connect(boost::bind(&MainMenuMapOptions::clicked_cancel, boost::ref(*this)));

	update();
	ok_.set_enabled(true);

	name_.focus();
	center_to_parent();
	move_to_top();
}

/**
 * Updates all UI::Textareas in the UI::Window to represent currently
 * set values
 */
void MainMenuMapOptions::update() {
	const Widelands::Map& map = eia().egbase().map();
	author_.set_text(map.get_author());
	name_.set_text(map.get_name());
	size_.set_text((boost::format(_("Size: %1% x %2%")) % map.get_width() % map.get_height()).str());
	descr_->set_text(map.get_description());
	hint_->set_text(map.get_hint());
	// map.get_waterway_max_length() defaults to 0 for older maps
	waterway_length_box_->set_value(std::max<uint32_t>(1, map.get_waterway_max_length()));

	std::set<std::string> tags = map.get_tags();
	for (auto tag : tags_checkboxes_) {
		tag.second->set_state(tags.count(tag.first) > 0);
	}

	balancing_dropdown_.select(tags.count("balanced") ? "balanced" : "unbalanced");
}

/**
 * Called when one of the editboxes are changed
 */
void MainMenuMapOptions::changed() {
	ok_.set_enabled(true);
}

void MainMenuMapOptions::clicked_ok() {
	eia().egbase().mutable_map()->set_name(name_.text());
	eia().egbase().mutable_map()->set_author(author_.text());
	set_config_string("realname", author_.text());
	eia().egbase().mutable_map()->set_description(descr_->get_text());
	eia().egbase().mutable_map()->set_hint(hint_->get_text());
	eia().egbase().mutable_map()->set_waterway_max_length(waterway_length_box_->get_value());

	eia().egbase().mutable_map()->clear_tags();
	for (const auto& tag : tags_checkboxes_) {
		if (tag.second->get_state()) {
			eia().egbase().mutable_map()->add_tag(tag.first);
		}
	}
	eia().egbase().mutable_map()->add_tag(balancing_dropdown_.get_selected());
	Notifications::publish(NoteMapOptions());
	registry_.destroy();
}

void MainMenuMapOptions::clicked_cancel() {
	registry_.destroy();
}

/*
 * Add a tag to the checkboxes
 */
void MainMenuMapOptions::add_tag_checkbox(UI::Box* parent,
                                          std::string tag,
                                          std::string displ_name) {
	UI::Box* box = new UI::Box(parent, 0, 0, UI::Box::Horizontal, max_w_, checkbox_space_, 0);
	UI::Checkbox* cb = new UI::Checkbox(box, Vector2i::zero(), displ_name);
	box->add(cb, UI::Box::Resizing::kFullSize);
	box->add_space(checkbox_space_);
	parent->add(box);
	parent->add_space(padding_);
	tags_checkboxes_[tag] = cb;
}
