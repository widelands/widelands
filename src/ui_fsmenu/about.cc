/*
 * Copyright (C) 2016 by Widelands Development Team
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

#include "ui_fsmenu/about.h"

#include <map>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "io/filesystem/filesystem.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"


FullscreenMenuAbout::FullscreenMenuAbout()
	:
	FullscreenMenuBase(),

// Values for alignment and size
	butw_    (get_w() / 5),
	buth_    (get_h() * 9 / 200),
	hmargin_ (get_w() * 19 / 200),
	padding_ (10),
	tab_panel_width_(get_inner_w() - 2 * hmargin_),
	tab_panel_y_(get_h() * 14 / 100),

	title_(this, get_w() / 2, buth_, _("About Widelands"), UI::Align::kCenter),

	close_
		(this, "close",
		 get_w() * 2 / 4 - butw_ / 2,
		 get_inner_h() - hmargin_,
		 butw_, buth_,
		 g_gr->images().get("images/ui_basic/but2.png"),
		 _("Close"), std::string(), true, false),

	tabs_(this, hmargin_, 0,
			tab_panel_width_, get_inner_h() - tab_panel_y_ - buth_ - hmargin_,
			g_gr->images().get("images/ui_basic/but1.png"),
			UI::TabPanel::Type::kBorder)
{
	title_.set_fontsize(UI_FONT_SIZE_BIG);
	tabs_.set_pos(Point(hmargin_, tab_panel_y_));

	add_tab("txts/README.lua");
	add_tab("txts/LICENSE.lua");
	add_tab("txts/AUTHORS.lua");

	close_.sigclicked.connect(boost::bind(&FullscreenMenuAbout::clicked_back, this));
}

void FullscreenMenuAbout::add_tab(const std::string& lua_script) {
	std::string content, title;
	try {
		LuaInterface lua;
		std::unique_ptr<LuaTable> t(lua.run_script(lua_script));
		content = t->get_string("text");
		title = t->get_string("title");
	} catch (LuaError & err) {
		content = err.what();
		title = "Lua error";
	}
	boxes_.push_back(std::unique_ptr<UI::Box>(new UI::Box(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_)));
	size_t index = boxes_.size() - 1;

	textviews_.push_back(std::unique_ptr<UI::MultilineTextarea>(
									new UI::MultilineTextarea(
										boxes_.at(index).get(),
										0,
										0,
										tab_panel_width_ - padding_,
										tabs_.get_inner_h() - 1.5 * padding_ - UI::kTabPanelButtonHeight,
										content)));
	tabs_.add((boost::format("about_%lu") % index).str(),
				 title,
				 boxes_.at(index).get(),
				 "");
	boxes_.at(index)->set_size(tabs_.get_inner_w(), tabs_.get_inner_h());

	assert(boxes_.size() == textviews_.size());
	assert(tabs_.tabs().size() == textviews_.size());
}
