/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "fullscreen_menu_mapselect.h"


#include "editor_game_base.h"
#include "graphic.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "map.h"
#include "widelands_map_loader.h"
#include "s2map.h"
#include "wexception.h"
#include "widelands_map_loader.h"
#include "ui_checkbox.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"

#include "log.h"

#include <stdio.h>

using Widelands::WL_Map_Loader;


struct Fullscreen_Menu_MapSelectImpl {
	UI::Checkbox* load_map_as_scenario;
	UI::Listselect<MapData>* list;
	UI::Textarea* taname;
	UI::Textarea* taauthor;
	UI::Textarea* tasize;
	UI::Textarea* taworld;
	UI::Textarea* tanplayers;
	UI::Multiline_Textarea* tadescr;
	UI::Basic_Button* ok;

	std::string curdir;
	std::string basedir;
};


Fullscreen_Menu_MapSelect::Fullscreen_Menu_MapSelect()
:
Fullscreen_Menu_Base("choosemapmenu.jpg"),
d(new Fullscreen_Menu_MapSelectImpl)
{
	UI::Textarea* title = new UI::Textarea(this, MENU_XRES / 2, 110, _("Choose your map!"), Align_HCenter);
	title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	UI::Textarea* label = new UI::Textarea(this, MENU_XRES - 300, 180, _("Load Map as scenario: "), Align_VCenter);
	d->load_map_as_scenario = new UI::Checkbox
		(this,
		 label->get_x() + label->get_w() + 10,
		 170);
	d->load_map_as_scenario->set_state(false);

	d->list = new UI::Listselect<MapData>(this, 15, 205, 455, 365);
	d->list->selected.set(this, &Fullscreen_Menu_MapSelect::map_selected);
	d->list->double_clicked.set(this, &Fullscreen_Menu_MapSelect::double_clicked);

	new UI::Textarea(this, 560, 205, _("Name:"), Align_Right);
	d->taname = new UI::Textarea(this, 570, 205, "");
	new UI::Textarea(this, 560, 225, _("Author:"), Align_Right);
	d->taauthor = new UI::Textarea(this, 570, 225, "");
	new UI::Textarea(this, 560, 245, _("Size:"), Align_Right);
	d->tasize = new UI::Textarea(this, 570, 245, "");
	new UI::Textarea(this, 560, 265, _("World:"), Align_Right);
	d->taworld = new UI::Textarea(this, 570, 265, "");
	new UI::Textarea(this, 560, 285, _("Players:"), Align_Right);
	d->tanplayers = new UI::Textarea(this, 570, 285, "");
	new UI::Textarea(this, 560, 305, _("Descr:"),   Align_Right);
	d->tadescr = new UI::Multiline_Textarea(this, 570, 305, 200, 190, "");

	new UI::IDButton<Fullscreen_Menu_MapSelect, int32_t>
		(this,
		 570, 505, 200, 26,
		 0,
		 &Fullscreen_Menu_MapSelect::end_modal, this, 0,
		 _("Back"));

	d->ok = new UI::Button<Fullscreen_Menu_MapSelect>
		(this,
		 570, 535, 200, 26,
		 2,
		 &Fullscreen_Menu_MapSelect::ok, this,
		 _("OK"),
		 std::string(),
		 false);
	d->ok->set_enabled(false);

	d->curdir = d->basedir = "maps";
	fill_list();
}

Fullscreen_Menu_MapSelect::~Fullscreen_Menu_MapSelect()
{
	delete d;
	d = 0;
}


bool Fullscreen_Menu_MapSelect::is_scenario()
{
	return d->load_map_as_scenario->get_state();
}

const MapData* Fullscreen_Menu_MapSelect::get_map() const
{
	if (d->list->has_selection())
		return &d->list->get_selected();
	return 0;
}


void Fullscreen_Menu_MapSelect::ok()
{
	const MapData& map = d->list->get_selected();

	if (!map.width) {
		d->curdir = g_fs->FS_CanonicalizeName(map.filename);
		fill_list();
	} else {
		if (is_scenario())
			end_modal(2);
		else
			end_modal(1);
	}
}


/**
 * Called when a different entry in the listbox gets selected.
 * When this happens, the information display at the right needs to be
 * refreshed.
 */
void Fullscreen_Menu_MapSelect::map_selected(uint32_t)
{
	const MapData& map = d->list->get_selected();

	if (map.width) {
		char buf[256];

		d->taname->set_text(map.name);
		d->taauthor->set_text(map.author);
		sprintf(buf, "%-4ux%4u", map.width, map.height);
		d->tasize->set_text(buf);
		sprintf(buf, "%i", map.nrplayers);
		d->tanplayers->set_text(buf);
		d->tadescr->set_text(map.description);
		d->taworld->set_text(map.world);
		d->ok->set_enabled(true);
	} else {
		// Directory
		d->taname->set_text("(directory)");
		d->taauthor->set_text("");
		d->tasize->set_text("");
		d->tanplayers->set_text("");
		d->tadescr->set_text("");
		d->taworld->set_text("");
		d->ok->set_enabled(true);
	}
}

/**
 * listbox got double clicked
 */
void Fullscreen_Menu_MapSelect::double_clicked(uint32_t) {
	ok();
}

/**
 * Fill the list with maps that can be opened.
 *
 * At first, only the subdirectories are added to the list, then the normal
 * files follow. This is done to make navigation easier.
 *
 * To make things more difficult, we have to support compressed and uncompressed
 * map files here - the former are files, the latter are directories. Care must
 * be taken to sort uncompressed maps (which look like and really are
 * directories) with the files.
 *
 * The search starts in \ref m_curdir ("..../maps") and there is no possibility
 * to move further up. If the user moves down into subdirectories, we insert an
 * entry to move back up.
 */
void Fullscreen_Menu_MapSelect::fill_list()
{
	d->list->clear();

	//  Fill it with all files we find in all directories.
	filenameset_t files;
	g_fs->FindFiles(d->curdir, "*", &files);

	int32_t ndirs=0;

	//If we are not at the top of the map directory hierarchy (we're not talking
	//about the absolute filesystem top!) we manually add ".."
	if (d->curdir != d->basedir) {
		MapData map;
		map.filename = g_fs->FS_CanonicalizeName(d->curdir + "/..");

		d->list->add
			("<parent>",
			 map,
			 g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"));
		++ndirs;
	}

	//Add subdirectories to the list (except for uncompressed maps)
	for
		(filenameset_t::iterator pname = files.begin();
		 pname != files.end(); ++pname)
	{
		const char * const name = pname->c_str();
		if (!strcmp(FileSystem::FS_Filename(name), ".")) continue;
		if (!strcmp(FileSystem::FS_Filename(name), "..")) continue; // Upsy, appeared again. ignore
		if (!strcmp(FileSystem::FS_Filename(name), ".svn")) continue; // HACK: we skip .svn dir (which is in normal checkout present) for esthetic reasons
		if (!g_fs->IsDirectory(name)) continue;
		if (WL_Map_Loader::is_widelands_map(name)) continue;

		MapData dir;
		dir.filename = name;
		d->list->add
			(FileSystem::FS_Filename(name),
			 dir,
			 g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"));
		++ndirs;
	}

	//Add map files(compressed maps) and directories(uncompressed)
	{
		Widelands::Map map; //  Map_Loader needs a place to put it's preload data

		for
			(filenameset_t::iterator pname = files.begin();
			 pname != files.end();
			 ++pname)
		{
			const char *name = pname->c_str();

			Widelands::Map_Loader * const ml = map.get_correct_loader(name);
			if (!ml)
				continue;

			try {
				map.set_filename(name);
				ml->preload_map(true);

				MapData mapdata;
				mapdata.filename = name;
				mapdata.name = map.get_name();
				mapdata.author = map.get_author();
				mapdata.description = map.get_description();
				mapdata.world = map.get_world_name();
				mapdata.nrplayers = map.get_nrplayers();
				mapdata.width = map.get_width();
				mapdata.height = map.get_height();
				if (!mapdata.width || !mapdata.height)
					continue;

				d->list->add
					(mapdata.name.c_str(),
					 mapdata,
					 g_gr->get_picture
					 	(PicMod_Game,
					 	 dynamic_cast<WL_Map_Loader const *>(ml) ?
					 	 "pics/ls_wlmap.png" : "pics/ls_s2map.png"));
			} catch (const std::exception & e) {
				log("Mapselect: Skip %s due to preload error: %s\n", name, e.what());
			} catch (...) {
				log("Mapselect: Skip %s due to unknown exception\n", name);
			}

			delete ml;
		}
	}

	d->list->sort(0, ndirs);
	d->list->sort(ndirs);

	if (d->list->size())
		d->list->select(0);
}
