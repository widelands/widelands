/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#include "graphic/image_catalog.h"

#include <cassert>
#include <map>
#include <string>

#include "base/log.h"
#include "io/filesystem/layered_filesystem.h"

ImageCatalog::ImageCatalog() {
	init();
}

ImageCatalog::~ImageCatalog() {
	entries_.clear();
}

// Register all images here
void ImageCatalog::init()  {
	entries_.clear();
	insert(Keys::kButton0, "but0.png");
	insert(Keys::kButton1, "but1.png");
	insert(Keys::kButton2, "but2.png");
	insert(Keys::kButton3, "but3.png");
	insert(Keys::kButton4, "but4.png");
	insert(Keys::kButtonMenuOK, "menu_okay.png");
	insert(Keys::kButtonMenuAbort, "menu_abort.png");
	insert(Keys::kLoadscreenEditor, "editor.jpg");
	insert(Keys::kSelect, "fsel.png");
	insert(Keys::kSelectEditorDelete, "fsel_editor_delete.png");
	insert(Keys::kSelectEditorHeightDecrease, "fsel_editor_decrease_height.png");
	insert(Keys::kSelectEditorHeightIncrease, "fsel_editor_increase_height.png");
	insert(Keys::kSelectEditorResourcesDecrease, "fsel_editor_decrease_resources.png");
	insert(Keys::kSelectEditorResourcesIncrease, "fsel_editor_increase_resources.png");
	insert(Keys::kSelectEditorResourcesDelete, "fsel_editor_delete.png");
	insert(Keys::kSelectEditorInfo, "fsel_editor_info.png");
	insert(Keys::kSelectEditorNoiseHeight, "fsel_editor_noise_height.png");
	insert(Keys::kSelectEditorPlaceBob, "fsel_editor_place_bob.png");
	insert(Keys::kSelectEditorPlaceImmovable, "fsel_editor_place_immovable.png");
	insert(Keys::kSelectEditorSetHeight, "fsel_editor_set_height.png");
	insert(Keys::kSelectEditorSetPortSpace, "fsel_editor_set_port_space.png");
	insert(Keys::kSelectEditorUnsetPortSpace, "fsel_editor_unset_port_space.png");
	insert(Keys::kSelectEditorSetResources, "fsel_editor_set_resources.png");
	insert(Keys::kSelectEditorSetStartingPosMin, "fsel_editor_set_player_01_pos.png");
	insert(Keys::kSelectEditorSetStartingPos1, "fsel_editor_set_player_01_pos.png");
	insert(Keys::kSelectEditorSetStartingPos2, "fsel_editor_set_player_02_pos.png");
	insert(Keys::kSelectEditorSetStartingPos3, "fsel_editor_set_player_03_pos.png");
	insert(Keys::kSelectEditorSetStartingPos4, "fsel_editor_set_player_04_pos.png");
	insert(Keys::kSelectEditorSetStartingPos5, "fsel_editor_set_player_05_pos.png");
	insert(Keys::kSelectEditorSetStartingPos6, "fsel_editor_set_player_06_pos.png");
	insert(Keys::kSelectEditorSetStartingPos7, "fsel_editor_set_player_07_pos.png");
	insert(Keys::kSelectEditorSetStartingPos8, "fsel_editor_set_player_08_pos.png");
	insert(Keys::kSelectEditorSetStartingPosMax, "fsel_editor_set_player_08_pos.png");
	insert(Keys::kEditorPlayerStartingPosMin, "editor_player_01_starting_pos.png");
	insert(Keys::kEditorPlayerStartingPos1, "editor_player_01_starting_pos.png");
	insert(Keys::kEditorPlayerStartingPos2, "editor_player_02_starting_pos.png");
	insert(Keys::kEditorPlayerStartingPos3, "editor_player_03_starting_pos.png");
	insert(Keys::kEditorPlayerStartingPos4, "editor_player_04_starting_pos.png");
	insert(Keys::kEditorPlayerStartingPos5, "editor_player_05_starting_pos.png");
	insert(Keys::kEditorPlayerStartingPos6, "editor_player_06_starting_pos.png");
	insert(Keys::kEditorPlayerStartingPos7, "editor_player_07_starting_pos.png");
	insert(Keys::kEditorPlayerStartingPos8, "editor_player_08_starting_pos.png");
	insert(Keys::kEditorPlayerStartingPosMax, "editor_player_08_starting_pos.png");
	insert(Keys::kFilesDirectory, "ls_dir.png");
	insert(Keys::kFilesWLMap, "ls_wlmap.png");
	insert(Keys::kFilesS2Map, "ls_s2map.png");
	insert(Keys::kFilesScenario, "ls_wlscenario.png");
	insert(Keys::kScrollbarUp, "scrollbar_up.png");
	insert(Keys::kScrollbarDown, "scrollbar_down.png");
	insert(Keys::kScrollbarLeft, "scrollbar_left.png");
	insert(Keys::kScrollbarRight, "scrollbar_right.png");
	insert(Keys::kScrollbarBackground, "scrollbar_background.png");


}

void ImageCatalog::insert(Keys key, const std::string& filename) {
	const std::string path = kBaseDir + filename;
	assert(g_fs->file_exists(path));
	entries_.emplace(key, path);
}

const std::string& ImageCatalog::filepath(Keys key) const {
	assert(has_key(key));
	return entries_.at(key);
}

bool ImageCatalog::has_key(Keys key) const {
	return entries_.count(key) == 1;
}
