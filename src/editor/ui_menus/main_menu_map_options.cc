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

#include "editor/ui_menus/main_menu_map_options.h"

#include <cstdio>
#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "logic/map.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/textarea.h"
#include "wui/map_tags.h"

inline EditorInteractive& MainMenuMapOptions::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

/**
 * Create all the buttons etc...
*/
MainMenuMapOptions::MainMenuMapOptions(EditorInteractive& parent, bool modal)
   : UI::Window(&parent, "map_options", 0, 0, 350, parent.get_inner_h() - 80, _("Map Options")),
     padding_(4),
     indent_(10),
     labelh_(text_height() + 4),
     checkbox_space_(25),
     butw_((get_inner_w() - 3 * padding_) / 2),
     max_w_(get_inner_w() - 2 * padding_),
     ok_(this,
         "ok",
         UI::g_fh1->fontset()->is_rtl() ? padding_ : butw_ + 2 * padding_,
         get_inner_h() - padding_ - labelh_,
         butw_,
         labelh_,
         UI::Button::Style::kWuiPrimary,
         _("OK")),
     cancel_(this,
             "cancel",
             UI::g_fh1->fontset()->is_rtl() ? butw_ + 2 * padding_ : padding_,
             get_inner_h() - padding_ - labelh_,
             butw_,
             labelh_,
             UI::Button::Style::kWuiSecondary,
             _("Cancel")),
     tab_box_(this, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
     tabs_(&tab_box_, 0, 0, UI::TabPanel::Style::kWuiLight),

     main_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
     tags_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
     teams_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),

     name_(&main_box_, 0, 0, max_w_, 0, 2, Panel::Style::kWui),
     author_(&main_box_, 0, 0, max_w_, 0, 2, Panel::Style::kWui),
     size_(&main_box_, 0, 0, max_w_ - indent_, labelh_, ""),

     teams_list_(&teams_box_,
                 0,
                 0,
                 max_w_,
                 60,
                 UI::Panel::Style::kWui,
                 UI::ListselectLayout::kShowCheck),

     modal_(modal) {

	tab_box_.set_size(max_w_, get_inner_h() - labelh_ - 2 * padding_);
	tabs_.set_size(max_w_, tab_box_.get_inner_h());
	main_box_.set_size(max_w_, tabs_.get_inner_h() - 35);
	tags_box_.set_size(max_w_, main_box_.get_h());
	teams_box_.set_size(max_w_, main_box_.get_h());

	// Calculate the overall remaining space for MultilineEditboxes.
	uint32_t remaining_space = main_box_.get_inner_h() - 7 * labelh_ - 5 * indent_;

	// We need less space for the hint and the description, but it should at least have 1 line
	// height.
	hint_ =
	   new UI::MultilineEditbox(&main_box_, 0, 0, max_w_, std::max(labelh_, remaining_space * 1 / 3));
	descr_ = new UI::MultilineEditbox(&main_box_, 0, 0, max_w_, remaining_space - hint_->get_h());

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Map Name:")));
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
	add_tag_checkbox(&tags_box_, "unbalanced", localize_tag("unbalanced"));
	add_tag_checkbox(&tags_box_, "ffa", localize_tag("ffa"));
	add_tag_checkbox(&tags_box_, "1v1", localize_tag("1v1"));
	add_tag_checkbox(&tags_box_, "2teams", localize_tag("2teams"));
	add_tag_checkbox(&tags_box_, "3teams", localize_tag("3teams"));
	add_tag_checkbox(&tags_box_, "4teams", localize_tag("4teams"));

	teams_box_.add(new UI::Textarea(&teams_box_, 0, 0, max_w_, labelh_, _("Suggested Teams:")));
	teams_box_.add(&teams_list_);
	// TODO(GunChleoc): We need team images in the listselect here,
	// so map editors will be able to delete entries.
	// This is waiting for the new RT renderer.
	teams_list_.add("Not implemented yet.", "", nullptr, false);

	unsigned int nr_players = static_cast<unsigned int>(eia().egbase().map().get_nrplayers());
	std::string players =
	   (boost::format(ngettext("%u Player", "%u Players", nr_players)) % nr_players).str();
	teams_box_.add(new UI::Textarea(&teams_box_, 0, 0, max_w_, labelh_, players));

	tab_box_.add(&tabs_, UI::Box::Resizing::kFullSize);
	tabs_.add("main_map_options", g_gr->images().get("images/wui/menus/menu_toggle_minimap.png"),
	          &main_box_, _("Main Options"));
	tabs_.add("map_tags", g_gr->images().get("images/ui_basic/checkbox_checked.png"), &tags_box_,
	          _("Tags"));
	tabs_.add("map_teams", g_gr->images().get("images/wui/editor/editor_menu_player_menu.png"),
	          &teams_box_, _("Teams"));

	name_.changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	author_.changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	descr_->changed.connect(boost::bind(&MainMenuMapOptions::changed, this));

	ok_.sigclicked.connect(boost::bind(&MainMenuMapOptions::clicked_ok, boost::ref(*this)));
	ok_.set_enabled(false);
	cancel_.sigclicked.connect(boost::bind(&MainMenuMapOptions::clicked_cancel, boost::ref(*this)));

	update();
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

	std::set<std::string> tags = map.get_tags();
	for (auto tag : tags_checkboxes_) {
		tag.second->set_state(tags.count(tag.first) > 0);
	}
}

/**
 * Called when one of the editboxes are changed
*/
void MainMenuMapOptions::changed() {
	ok_.set_enabled(true);
}

void MainMenuMapOptions::clicked_ok() {
	eia().egbase().map().set_name(name_.text());
	eia().egbase().map().set_author(author_.text());
	g_options.pull_section("global").set_string("realname", author_.text());
	eia().egbase().map().set_description(descr_->get_text());
	eia().egbase().map().set_hint(hint_->get_text());

	eia().egbase().map().clear_tags();
	for (const auto& tag : tags_checkboxes_) {
		if (tag.second->get_state()) {
			eia().egbase().map().add_tag(tag.first);
		}
	}

	if (modal_) {
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
	} else {
		die();
	}
}

void MainMenuMapOptions::clicked_cancel() {
	if (modal_) {
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	} else {
		die();
	}
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
