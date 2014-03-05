/*
 * Copyright (C) 2002-2008, 2010-2011 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_CAMPAIGN_SELECT_H
#define FULLSCREEN_MENU_CAMPAIGN_SELECT_H

#include "ui_fsmenu/base.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

/*
 * Fullscreen Menu for all Campaigns
 */

/*
 * UI 1 - Selection of Campaign
 *
 */
struct Fullscreen_Menu_CampaignSelect : public Fullscreen_Menu_Base {
	Fullscreen_Menu_CampaignSelect();
	void clicked_back();
	void clicked_ok();
	void campaign_selected(uint32_t);
	void double_clicked(uint32_t);
	void fill_list();
	int32_t get_campaign();

private:
	uint32_t    m_butw;
	uint32_t    m_buth;
	uint32_t    m_fs;
	std::string m_fn;

	UI::Textarea                             title;
	UI::Textarea                             label_campname;
	UI::Textarea                             tacampname;
	UI::Textarea                             label_difficulty;
	UI::Multiline_Textarea                   tadifficulty;
	UI::Textarea                             label_campdescr;
	UI::Multiline_Textarea                   tacampdescr;
	UI::Button                               b_ok, back;
	UI::Listselect<const char *>             m_list;

	/// Variables used for exchange between the two Campaign UIs and
	/// Game::run_campaign
	int32_t                                           campaign;
};
/*
 * UI 2 - Selection of a map
 *
 */
struct Fullscreen_Menu_CampaignMapSelect : public Fullscreen_Menu_Base {
	Fullscreen_Menu_CampaignMapSelect();
	void clicked_back();
	void clicked_ok();
	void map_selected(uint32_t);
	void double_clicked(uint32_t);
	void fill_list();
	std::string get_map();
	void set_campaign(uint32_t);

private:
	uint32_t    m_butw;
	uint32_t    m_buth;
	uint32_t    m_fs;
	std::string m_fn;

	UI::Textarea                            title;
	UI::Textarea                            label_mapname;
	UI::Textarea                            tamapname;
	UI::Textarea                            label_author;
	UI::Textarea                            taauthor;
	UI::Textarea                            label_mapdescr;
	UI::Multiline_Textarea                  tamapdescr;
	UI::Button                              b_ok, back;
	UI::Listselect<std::string>             m_list;
	uint32_t                                campaign;
	std::string                             campmapfile;

};

#endif
