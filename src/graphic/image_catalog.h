/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

		// ui_basic
		kButton0,
		kButton1,
		kButton2,
		kButton3,
		kButton4,
		kFilesDirectory,
		kFilesWLMap,
		kFilesS2Map,
		kFilesScenario,
		kScrollbarUp,
		kScrollbarDown,
		kScrollbarLeft,
		kScrollbarRight,
		kScrollbarBackground,
		kSelect,
		kCaret,
		kCheckbox,
		kCheckboxChecked,
		kCheckboxEmpty,
		kCheckboxLight,
		kCursor,
		kCursor2,
		kCursorClick,
		kListFirst,
		kListSecond,
		kListThird,
		kListSelected,
		kActionContinue,
		kActionDifferent,
		kActionStop,

		// wui
		kWindowBackground,
		kWindowBorderTop,
		kWindowBorderLeft,
		kWindowBorderRight,
		kWindowBorderBottom,
		kButtonMenuOK,
		kButtonMenuAbort,

		// ui_fsmenu
		kFullscreen,
		kFullscreenChooseMap,
		kFullscreenFileWiew,
		kFullscreenInternet,
		kFullscreenLaunchMPG,
		kFullscreenMain,
		kFullscreenOptions,

		//loadscreens
		kLoadscreen,
		kLoadscreenEditor,
		kLoadscreenTips,
		kLoadscreenSplash,

		// editor
		kEditorUndo,
		kEditorRedo,
		kEditorMenuPlayer,
		kEditorMenuTools,
		kEditorMenuToolBob,
		kEditorMenuToolHeight,
		kEditorMenuToolImmovable,
		kEditorMenuToolNoiseHeight,
		kEditorMenuToolPortSpace,
		kEditorMenuToolResources,
		kEditorMenuToolSize,
		kEditorMenuToolTerrain,
		kEditorTerrainDead,
		kEditorTerrainDry,
		kEditorTerrainGreen,
		kEditorTerrainMountain,
		kEditorTerrainUnpassable,
		kEditorTerrainWater,
		kEditorToolBob,
		kEditorToolDelete,
		kEditorToolHeight,
		kEditorToolHeightDecrease,
		kEditorToolHeightIncrease,
		kEditorToolImmovable,
		kEditorToolInfo,
		kEditorToolNoiseHeight,
		kEditorToolPortSpaceSet,
		kEditorToolPortSpaceSetSet,
		kEditorToolResourcesDecrease,
		kEditorToolResourcesDelete,
		kEditorToolResourcesIncrease,
		kEditorToolResourcesSet,

		// stats
		kStatsCasualties,
		kStatsBuildingsLost,
		kStatsKills,
		kStatsLandsize,
		kStatsMilitaryStrength,
		kStatsMilitarySitesDefeated,
		kStatsMilitarySitesLost,
		kStatsBuildingsNumber,
		kStatsWaresNumber,
		kStatsWorkersNumber,
		kStatsPoints,
		kStatsProductivity,
		kStatsTrees,

		// minimap
		kMinimapBuildings,
		kMinimapFlags,
		kMinimapOwner,
		kMinimapRoads,
		kMinimapTerrain,
		kMinimapZoom,

		// overlays
		kOverlaysMapSpot,
		kOverlaysPlotMine,
		kOverlaysPlotBig,
		kOverlaysPlotMedium,
		kOverlaysPlotSmall,
		kOverlaysPlotPort,
		kOverlaysRoadbuildingLevel,
		kOverlaysRoadbuildingSteepAscending,
		kOverlaysRoadbuildingSteepDecending,
		kOverlaysRoadbuildingAscending,
		kOverlaysRoadbuildingDecending,
		kOverlaysFlag,
		kOverlaysWorkarea1,
		kOverlaysWorkarea2,
		kOverlaysWorkarea3,
		kOverlaysWorkarea12,
		kOverlaysWorkarea23,
		kOverlaysWorkarea123,

		// ship
		kShipExploreCounterclockwise,
		kShipExploreClockwise,
		kShipScoutEast,
		kShipScoutNorthEast,
		kShipScoutNorthWest,
		kShipScoutSouthEast,
		kShipScoutSouthWest,
		kShipScoutWest,
		kShipExpeditionCancel,
		kShipDestination,
		kShipGoto,
		kShipSink,

		// buildings
		kDockExpeditionStart,
		kDockExpeditionCancel,
		kBuildingPriorityHigh,
		kBuildingPriorityNormal,
		kBuildingPriorityLow,
		kBuildingMaxFillIndicator,
		kBuildingAttack,
		kBuildingBulldoze,
		kBuildingDismantle,
		kBuildingSoldierDrop,
		kBuildingSoldierHeroes,
		kBuildingSoldierRookies,
		kBuildingStockPolicyDontStockButton,
		kBuildingStockPolicyNormalButton,
		kBuildingStockPolicyPreferButton,
		kBuildingStockPolicyRemoveButton,
		kBuildingStockPolicyDontStock,
		kBuildingStockPolicyPrefer,
		kBuildingStockPolicyRemove,

		// players
		kPlayerStartingPosSmall1, // Keep player icons in ascending order
		kPlayerStartingPosSmall2,
		kPlayerStartingPosSmall3,
		kPlayerStartingPosSmall4,
		kPlayerStartingPosSmall5,
		kPlayerStartingPosSmall6,
		kPlayerStartingPosSmall7,
		kPlayerStartingPosSmall8,
		kPlayerStartingPosBig1, // Keep player icons in ascending order
		kPlayerStartingPosBig2,
		kPlayerStartingPosBig3,
		kPlayerStartingPosBig4,
		kPlayerStartingPosBig5,
		kPlayerStartingPosBig6,
		kPlayerStartingPosBig7,
		kPlayerStartingPosBig8,
		kPlayerFlag1, // Keep player icons in ascending order
		kPlayerFlag2,
		kPlayerFlag3,
		kPlayerFlag4,
		kPlayerFlag5,
		kPlayerFlag6,
		kPlayerFlag7,
		kPlayerFlag8,

		// logos
		kLogoEditor16,
		kLogoEditor32,
		kLogoEditor64,
		kLogoEditor128,
		kLogoWidelands16,
		kLogoWidelands32,
		kLogoWidelands48,
		kLogoWidelands64,
		kLogoWidelands128,
		kLogoWidelandsLogo,
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
