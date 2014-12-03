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

#ifndef WL_GRAPHIC_IMAGE_CATALOG_H
#define WL_GRAPHIC_IMAGE_CATALOG_H

#include <string>
#include <map>
#include <vector>

#include "base/macros.h"

/// This class maps enum keys to image filenames.
/// All image files we have in 'kBaseDir' should be registered in this catalog.
class ImageCatalog {
public:
	/// The keys to fetch the image filenames.
	enum class Keys {
		kUnknownImage = 0, // This is the key for a nonexistent image. Do not add this to the entries.
		kButton0,
		kButton1,
		kButton2,
		kButton3,
		kButton4,
		kButtonMenuOK,
		kButtonMenuAbort,
		kLoadscreenEditor,
		kSelect,
		kSelectEditorDelete,
		kSelectEditorHeightDecrease,
		kSelectEditorHeightIncrease,
		kSelectEditorResourcesDecrease,
		kSelectEditorResourcesIncrease,
		kSelectEditorResourcesDelete,
		kSelectEditorInfo,
		kSelectEditorNoiseHeight,
		kSelectEditorPlaceBob,
		kSelectEditorPlaceImmovable,
		kSelectEditorSetHeight,
		kSelectEditorSetPortSpace,
		kSelectEditorUnsetPortSpace,
		kSelectEditorSetResources,
		kSelectEditorSetStartingPos1, // Keep player icons in ascending order
		kSelectEditorSetStartingPos2,
		kSelectEditorSetStartingPos3,
		kSelectEditorSetStartingPos4,
		kSelectEditorSetStartingPos5,
		kSelectEditorSetStartingPos6,
		kSelectEditorSetStartingPos7,
		kSelectEditorSetStartingPos8,
		kEditorPlayerStartingPos1, // Keep player icons in ascending order
		kEditorPlayerStartingPos2,
		kEditorPlayerStartingPos3,
		kEditorPlayerStartingPos4,
		kEditorPlayerStartingPos5,
		kEditorPlayerStartingPos6,
		kEditorPlayerStartingPos7,
		kEditorPlayerStartingPos8,
		kFilesDirectory,
		kFilesWLMap,
		kFilesS2Map,
		kFilesScenario,
		kScrollbarUp,
		kScrollbarDown,
		kScrollbarLeft,
		kScrollbarRight,
		kScrollbarBackground,
		kStatsPlayer1, // Keep player icons in ascending order
		kStatsPlayer2,
		kStatsPlayer3,
		kStatsPlayer4,
		kStatsPlayer5,
		kStatsPlayer6,
		kStatsPlayer7,
		kStatsPlayer8,
	};

	/// The base directory for the images in this catalog.
	static constexpr const char* kBaseDir = "data/pics/";

	/// The constructor will map keys to filenames.
	ImageCatalog();
	~ImageCatalog();

	/// Returns the filepath starting from the Widelands root
	/// for the image associated with 'key'.
	const std::string& filepath(ImageCatalog::Keys key) const;

	bool has_key(ImageCatalog::Keys key) const;

private:
	/// This function registers a filename for each member of ImageCatalog::Keys.
	void init();

	/// Inserts an image into the catalog for the 'key'.
	/// The 'filename' is the relative path starting from kBaseDir.
	void insert(ImageCatalog::Keys key, const std::string& filename);

	/// Container for the key - filename mapping.
	std::map<ImageCatalog::Keys, std::string> entries_;

	DISALLOW_COPY_AND_ASSIGN(ImageCatalog);
};

#endif  // end of include guard: WL_GRAPHIC_IMAGE_CATALOG_H
