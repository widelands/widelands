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

namespace {
bool read_text(const std::string& filename, std::string* title, std::string* content) {
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
	return true;
}

bool read_authors(std::string* title, std::string* content) {
	bool result = read_text("txts/AUTHORS.lua", title, content);
	const std::string userlocale = i18n::get_locale();
	try {
		LuaInterface lua;
		std::unique_ptr<LuaTable> all_locales(lua.run_script("i18n/locales.lua"));

		// Sort the locales for their sort_name
		std::map<std::string, std::string> sorted_locales;
		{
			i18n::Textdomain td("texts");
			for (const std::string code : all_locales->keys<std::string>()) {
				std::unique_ptr<LuaTable> locale = all_locales->get_table(code);
				locale->do_not_warn_about_unaccessed_keys();
				sorted_locales.emplace(_(locale->get_string("localized_name")), code);
			}
		}

		i18n::Textdomain td("translator_credits");
		std::string translators;
		for (const std::pair<std::string, std::string>& locale_pair : sorted_locales) {
			std::unique_ptr<LuaTable> locale = all_locales->get_table(locale_pair.second);
			locale->do_not_warn_about_unaccessed_keys();
			i18n::set_locale(locale_pair.second);
			const std::string locale_translators = _("Translator Credits");
			if (!boost::iequals(locale_translators, "Translator Credits")) { // Do not list empty credits
				// Display language name
				translators =
						(boost::format("%s<rt><p font-size=14 font-weight=bold font-color=D1D1D1>%s<br></p></rt>")
						 % translators.c_str()
						 % locale_pair.first.c_str()).str();
				// Display the translators list
				std::vector<std::string> lines;
				boost::split(lines, locale_translators, boost::is_any_of("\n"));
				for (const std::string locale_translator : lines) {
					translators =
							(boost::format("%s<rt image=pics/fsel_editor_set_height.png"
												" image-align=left text-align=left><p font-size=12>%s</p></rt>")
							 % translators.c_str()
							 % locale_translator.c_str()).str();
				}
			}
		}
		*content = (boost::format(*content) % translators.c_str()).str();
	} catch (LuaError & err) {
		i18n::set_locale(userlocale);
		*content = err.what();
		*title = "Lua error";
		return false;
	}
	i18n::set_locale(userlocale);
	return result;
}

}  // namespace

FullscreenMenuTextView::FullscreenMenuTextView
	()
	:
	FullscreenMenuBase("fileviewmenu.jpg"),

	title (this, get_w() * 3 / 50, get_h() / 10, "", UI::Align_Center),

	textview
		(this,
		 get_w() *   3 /   80, get_h() * 283 / 1000,
		 get_w() * 919 / 1000, get_h() *  11 /   20),

	close_button
		(this, "close",
		 get_w() * 3 / 8, get_h() * 9 / 10, get_w() / 4, get_h() * 9 / 200,
		 g_gr->images().get("pics/but0.png"),
		 _("Close"), std::string(), true, false)
{
	close_button.sigclicked.connect(boost::bind(&FullscreenMenuTextView::end_modal, boost::ref(*this), 0));

	title.set_font(ui_fn(), fs_big(), UI_FONT_CLR_FG);
	title.set_pos
		(Point((get_inner_w() - title.get_w()) / 2, get_h() * 167 / 1000));

	textview.set_font(UI::g_fh1->fontset().serif(), UI_FONT_SIZE_PROSA, PROSA_FONT_CLR_FG);
}

void FullscreenMenuTextView::set_text(const std::string & text)
{
	textview.set_text(text);
}

void FullscreenMenuTextView::set_title(const std::string& text)
{
	title.set_text(text);
}

FullscreenMenuFileView::FullscreenMenuFileView(const std::string & filename)
: FullscreenMenuTextView()
{
	std::string content, title_text;
	read_text(filename, &title_text, &content);
	set_text(content);
	set_title(title_text);
}

FullscreenMenuAuthorsView::FullscreenMenuAuthorsView(const std::string&)
: FullscreenMenuTextView()
{
	std::string content, title_text;
	read_authors(&title_text, &content);
	set_text(content);
	set_title(title_text);
}

struct TextViewWindow : public UI::UniqueWindow {
	TextViewWindow
		(UI::Panel                  & parent,
		 UI::UniqueWindow::Registry & reg);
protected:
	void set_text(const std::string & text);
private:
	UI::MultilineTextarea textview;
};

TextViewWindow::TextViewWindow
	(UI::Panel                  & parent,
	 UI::UniqueWindow::Registry & reg)
	:
	UI::UniqueWindow(&parent, "file_view", &reg, 0, 0, ""),
	textview(this, 0, 0, 560, 240)
{
	textview.set_font(UI::g_fh1->fontset().serif(), UI_FONT_SIZE_PROSA, PROSA_FONT_CLR_FG);

	set_inner_size(560, 240);

	if (get_usedefaultpos())
		center_to_parent();
}

void TextViewWindow::set_text(const std::string& text)
{
	textview.set_text(text);
}


struct FileViewWindow : public TextViewWindow {
	FileViewWindow
		(UI::Panel                  & parent,
		 UI::UniqueWindow::Registry & reg,
		 const std::string          & filename);
};

FileViewWindow::FileViewWindow
	(UI::Panel                  & parent,
	 UI::UniqueWindow::Registry & reg,
	 const std::string          & filename)
	:
	TextViewWindow(parent, reg)
{
	std::string title_text, content;
	read_text(filename, &title_text, &content);
	set_text(content);
	set_title(title_text);
}


struct AuthorsWindow : public TextViewWindow {
	AuthorsWindow(UI::Panel& parent, UI::UniqueWindow::Registry& reg);
};

AuthorsWindow::AuthorsWindow(UI::Panel& parent, UI::UniqueWindow::Registry& reg)
	:
	TextViewWindow(parent, reg)
{
	std::string title_text, content;
	read_authors(&title_text, &content);
	set_text(content);
	set_title(title_text);
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


/**
 * Display the contents of the authors file in a scrollable window.
*/
void authors_window(UI::Panel& parent, UI::UniqueWindow::Registry& reg) {
	new AuthorsWindow(parent, reg);
}
