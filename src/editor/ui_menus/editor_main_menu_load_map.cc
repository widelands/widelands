/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "editor_main_menu_load_map.h"

#include "building.h"
#include "graphic.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "editor.h"
#include "editorinteractive.h"
#include "widelands_map_loader.h"
#include "wexception.h"
#include "error.h"
#include "editor_game_base.h"
#include "editor_set_starting_pos_tool.h"
#include "overlay_manager.h"
#include "world.h"
#include "map_loader.h"

#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "ui_multilinetextarea.h"
#include "ui_progresswindow.h"
#include "ui_textarea.h"

#include <string>

#include <stdio.h>

/*
===============
Main_Menu_Load_Map::Main_Menu_Load_Map

Create all the buttons etc...
===============
*/
Main_Menu_Load_Map::Main_Menu_Load_Map(Editor_Interactive *parent)
	: UI::Window(parent, 0, 0, 500, 300, _("Load Map").c_str())
{
   m_parent=parent;

   int spacing=5;
   int offsx=spacing;
   int offsy=30;
   int posx=offsx;
   int posy=offsy;

   // listselect
   m_ls=new UI::Listselect<const char *>(this, posx, posy, get_inner_w()/2-spacing, get_inner_h()-spacing-offsy-40);
   m_ls->selected.set(this, &Main_Menu_Load_Map::selected);
   m_ls->double_clicked.set(this, &Main_Menu_Load_Map::double_clicked);

   // the descriptive areas
   // Name
   posx=get_inner_w()/2+spacing;
   posy+=20;
   new UI::Textarea(this, posx, posy, 150, 20, _("Name: "), Align_CenterLeft);
   m_name=new UI::Textarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Author
   new UI::Textarea(this, posx, posy, 150, 20, _("Author: "), Align_CenterLeft);
   m_author=new UI::Textarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Size
   new UI::Textarea(this, posx, posy, 70, 20, _("Size: "), Align_CenterLeft);
   m_size=new UI::Textarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // World
   new UI::Textarea(this, posx, posy, 70, 20, _("World: "), Align_CenterLeft);
   m_world=new UI::Textarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Players
   new UI::Textarea(this, posx, posy, 70, 20, _("Players: "), Align_CenterLeft);
   m_nrplayers=new UI::Textarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;


   // Description
   new UI::Textarea(this, posx, posy, 70, 20, _("Descr: "), Align_CenterLeft);
   m_descr=new UI::Multiline_Textarea(this, posx+70, posy, get_inner_w()-posx-spacing-70, get_inner_h()-posy-spacing-40, "---", Align_CenterLeft);

   posx=5;
   posy=get_inner_h()-30;

	m_ok_btn = new UI::Button<Main_Menu_Load_Map>
		(this,
		 get_inner_w() / 2 - spacing - 80, posy, 80, 20,
		 0,
		 &Main_Menu_Load_Map::clicked_ok, this,
		 _("OK"),
		 std::string(),
		 false);

	new UI::Button<Main_Menu_Load_Map>
		(this,
		 get_inner_w() / 2 + spacing, posy, 80, 20,
		 1,
		 &Main_Menu_Load_Map::die, this,
		 _("Cancel"));

   m_basedir="maps";
   m_curdir="maps";

   fill_list();

   center_to_parent();
   move_to_top();
}

/*
===============
Main_Menu_Load_Map::~Main_Menu_Load_Map

Unregister from the registry pointer
===============
*/
Main_Menu_Load_Map::~Main_Menu_Load_Map()
{
}

/*
===========
called when the ok button has been clicked
===========
*/
void Main_Menu_Load_Map::clicked_ok() {
	std::string filename(m_ls->get_selected());

      if (g_fs->IsDirectory(filename.c_str()) && !Widelands_Map_Loader::is_widelands_map(filename)) {
	      m_curdir=g_fs->FS_CanonicalizeName(filename);
         m_ls->clear();
         m_mapfiles.clear();
         fill_list();
		} else {
         load_map(filename);
         die();
		}
}

/*
 * called when a item is selected
 */
void Main_Menu_Load_Map::selected(uint) {
	const char * const name = m_ls->get_selected();

   m_ok_btn->set_enabled(true);

   if (!g_fs->IsDirectory(name) || Widelands_Map_Loader::is_widelands_map(name)) {
		Map map;
		Map_Loader * const m_ml = map.get_correct_loader(name);
      m_ml->preload_map(true); // This has worked before, no problem
      delete m_ml;

		m_name  ->set_text(map.get_name       ());
		m_author->set_text(map.get_author     ());
		m_descr ->set_text(map.get_description());
		m_world ->set_text(map.get_world_name ());

      char buf[200];
		sprintf(buf, "%i", map.get_nrplayers());
      m_nrplayers->set_text(buf);

		sprintf(buf, "%ix%i", map.get_width(), map.get_height());
      m_size->set_text(buf);
	} else {
      m_name->set_text("");
      m_author->set_text("");
      m_descr->set_text("");
      m_world->set_text("");
      m_nrplayers->set_text("");
      m_size->set_text("");
	}
}

/*
 * An Item has been doubleclicked
 */
void Main_Menu_Load_Map::double_clicked(uint) {clicked_ok();}

/*
 * fill the file list
 */
void Main_Menu_Load_Map::fill_list() {
   // Fill it with all files we find.
   g_fs->FindFiles(m_curdir, "*", &m_mapfiles, 1);

   // First, we add all directorys
   // We manually add the parent directory
	if (m_curdir != m_basedir) {
	   m_parentdir=g_fs->FS_CanonicalizeName(m_curdir+"/..");
		m_ls->add
			("<parent>",
			 m_parentdir.c_str(),
			 g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"));
	}

	const filenameset_t::const_iterator mapfiles_end = m_mapfiles.end();
	for
		(filenameset_t::const_iterator pname = m_mapfiles.begin();
		 pname != mapfiles_end;
		 ++pname)
	{
      const char *name = pname->c_str();
      if (!strcmp(FileSystem::FS_Filename(name), ".")) continue;
      if (!strcmp(FileSystem::FS_Filename(name), "..")) continue; // Upsy, appeared again. ignore
      if (!strcmp(FileSystem::FS_Filename(name), "CVS")) continue; // HACK: we skip CVS dir (which is in normal checkout present) for aesthetic reasons
      if (!g_fs->IsDirectory(name)) continue;
      if (Widelands_Map_Loader::is_widelands_map(name)) continue;

		m_ls->add
			(FileSystem::FS_Filename(name),
			 name,
			 g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"));
	}

	Map map;

	for
		(filenameset_t::const_iterator pname = m_mapfiles.begin();
		 pname != mapfiles_end;
		 ++pname)
	{
      const char *name = pname->c_str();

		Map_Loader * const m_ml = map.get_correct_loader(name);
      if (!m_ml) continue;

		try {
         m_ml->preload_map(true);
         std::string pic="";
         switch (m_ml->get_type()) {
            case Map_Loader::WLML: pic="pics/ls_wlmap.png"; break;
            case Map_Loader::S2ML: pic="pics/ls_s2map.png"; break;
			}
			m_ls->add
				(FileSystem::FS_Filename(name),
				 name,
				 g_gr->get_picture(PicMod_Game, pic.c_str()));
		} catch (_wexception&) {
         // we simply skip illegal entries
		}
      delete m_ml;

	}

	if (m_ls->size()) m_ls->select(0);
}

/*
 * Load map complete
 */
void Main_Menu_Load_Map::load_map(std::string filename) {
	Map & map = m_parent->editor().map();

   if (filename!="") {
		m_parent->editor().cleanup_for_load(true, false);

		Map_Loader * const ml = map.get_correct_loader(filename.c_str());

		UI::ProgressWindow loader_ui;
         ml->preload_map(true);

		 loader_ui.step (_("Loading world data"));
		ml->load_world();
		ml->load_map_complete(&m_parent->editor(), true);
		m_parent->editor().postload();
		m_parent->editor().load_graphics(loader_ui);

      // Now update all the visualisations
      // Player positions
      std::string text;
		for (Uint8 i = 1; i <= map.get_nrplayers(); ++i) {
         text="pics/editor_player_";
         text+=static_cast<char>(((i)/10) + 0x30);
         text+=static_cast<char>(((i)%10) + 0x30);
         text+="_starting_pos.png";
			const Coords fc = map.get_starting_pos(i);

			if (fc.isNull()) continue;
			uint w, h;
			const uint picid=g_gr->get_picture(PicMod_Game,  text.c_str());
			g_gr->get_picture_size(picid, w, h);
         // only register, when theres no building there
         // no building, place overlay
			if (not dynamic_cast<const Building * const>(map[fc].get_immovable()))
				map.overlay_manager().register_overlay
				(fc, picid, 8, Point(w / 2, STARTING_POS_HOTSPOT_Y));
		}

      /* Resources. we do not calculate default resources, therefore we do
       * not expect to meet them here. */
		const World & world = map.world();
		Overlay_Manager & overlay_manager = map.overlay_manager();
		const X_Coordinate mapwidth  = map.get_width ();
		const Y_Coordinate mapheight = map.get_height();
		Map::Index i = 0;
		for (Y_Coordinate y = 0; y < mapheight; ++y)
			for (X_Coordinate x = 0; x < mapwidth; ++x, ++i) {
				Field f = map[i];
				if (const uchar amount = f.get_resources_amount()) {
					const std::string & immname =
						world.get_resource(f.get_resources())->get_editor_pic(amount);
					if (immname.size())
						overlay_manager.register_overlay
							(Coords(x, y),
							 g_gr->get_picture(PicMod_Game, immname.c_str()),
							 4);
				}
			}

      // Tell the interactive that the map is saved and all
      m_parent->set_need_save(false);

      // Redraw everything
      m_parent->need_complete_redraw();

      delete ml;
	}
}
