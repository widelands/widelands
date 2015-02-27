/*
 * Copyright (C) 2002, 2006-2011 by the Widelands Development Team
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

#include "ui_fsmenu/fileview.h"

#include <memory>

#include <boost/algorithm/string/predicate.hpp>

#include "base/i18n.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "io/filesystem/filesystem.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

namespace {
bool read_text(const std::string& filename, std::string* title, std::string* content) {
	if (boost::algorithm::ends_with(filename, ".lua")) {
		try {
			LuaInterface lua;
			std::unique_ptr<LuaTable> t(lua.run_script(filename));
			*content = t->get_string("text");
			*title = t->get_string("title");
		} catch (LuaError & err) {
			*content = err.what();
			*title = "Lua error";
			return false;
		}
	} else {  // Old style plain text file.
		Profile prof(filename.c_str(), "global", "texts"); // section-less file
		Section & s = prof.get_safe_section("global");

		*title = s.get_safe_string("title");
		*content = s.get_safe_string("text");
	}
	return true;
}

}  // namespace
FullscreenMenuTextView::FullscreenMenuTextView
	(const std::string & filename)
	:
	FullscreenMenuBase("images/ui_fsmenu/fileviewmenu.jpg"),

	title (this, get_w() * 3 / 50, get_h() / 10),

	textview
		(this,
		 get_w() *   3 /   80, get_h() * 283 / 1000,
		 get_w() * 919 / 1000, get_h() *  11 /   20),

	close_button
		(this, "close",
		 get_w() * 3 / 8, get_h() * 9 / 10, get_w() / 4, get_h() * 9 / 200,
		 g_gr->images().get("images/ui_basic/but0.png"),
		 _("Close"), std::string(), true, false)
{
	close_button.sigclicked.connect(boost::bind(&FullscreenMenuTextView::end_modal, boost::ref(*this), 0));

	std::string content, title_text;
	read_text(filename, &title_text, &content);

	title   .set_text(title_text);
	textview.set_text(content);

	title.set_font(ui_fn(), fs_big(), UI_FONT_CLR_FG);
	title.set_pos
		(Point((get_inner_w() - title.get_w()) / 2, get_h() * 167 / 1000));

	textview.set_font(UI::g_fh1->fontset().serif(), UI_FONT_SIZE_PROSA, PROSA_FONT_CLR_FG);
}

void FullscreenMenuTextView::set_text(const std::string & text)
{
	textview.set_text(text);
}

FullscreenMenuFileView::FullscreenMenuFileView(const std::string & filename)
: FullscreenMenuTextView(filename)
{}


struct FileViewWindow : public UI::UniqueWindow {
	FileViewWindow
		(UI::Panel                  & parent,
		 UI::UniqueWindow::Registry & reg,
		 const std::string          & filename);
private:
	UI::MultilineTextarea textview;
};

FileViewWindow::FileViewWindow
	(UI::Panel                  & parent,
	 UI::UniqueWindow::Registry & reg,
	 const std::string          & filename)
	:
	UI::UniqueWindow(&parent, "file_view", &reg, 0, 0, ""),
	textview(this, 0, 0, 560, 240)
{
	std::string title_text, content;
	read_text(filename, &title_text, &content);

	textview.set_text(content);
	textview.set_font(UI::g_fh1->fontset().serif(), UI_FONT_SIZE_PROSA, PROSA_FONT_CLR_FG);

	set_inner_size(560, 240);

	if (get_usedefaultpos())
		center_to_parent();
}


/**
 * Display the contents of a text file in a scrollable window.
*/
void fileview_window
	(UI::Panel                  & parent,
	 UI::UniqueWindow::Registry & reg,
	 const std::string          & filename)
{
	new FileViewWindow(parent, reg, filename);
}
