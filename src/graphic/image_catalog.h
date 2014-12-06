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

// NOCOM(GunChleoc): Review this class

#include <string>
#include <map>
#include <vector>

#include "base/macros.h"

/// This class maps enum keys to image filenames.
/// All image files we have in 'kBaseDir' should be registered in this catalog.
class ImageCatalog {
public:
	/// The keys to fetch the image filenames.
	// NOCOM(#codereview): this gives me a ton of questions: is it save to delete an entry from this set or are the keys send over the network or saved to disk? What is the benefit of an integer over a string?
	enum class Key {
		kUnknownImage = 0, // This is the key for a nonexistent image. Do not add this to the entries.

		// general
		kNoValue,

		// ui_basic
		kActionContinue,
		kActionDifferent,
		kActionStop,
		kButton0,
		kButton1,
		kButton2,
		kButton3,
		kButton4,
		kCaret,
		kCheckbox,
		kCheckboxChecked,
		kCheckboxEmpty,
		kCheckboxLight,
		kCursor,
		kCursorClick,
		kFilesDirectory,
		kFilesS2Map,
		kFilesScenario,
		kFilesWLMap,
		kHelp,
		kListFirst,
		kListSelected,
		kScrollbarBackground,
		kScrollbarDown,
		kScrollbarLeft,
		kScrollbarRight,
		kScrollbarUp,
		kSelect,

		// players
		kPlayerFlag1, // Keep player icons in ascending order
		kPlayerFlag2,
		kPlayerFlag3,
		kPlayerFlag4,
		kPlayerFlag5,
		kPlayerFlag6,
		kPlayerFlag7,
		kPlayerFlag8,
		kPlayerStartingPosBig1, // Keep player icons in ascending order
		kPlayerStartingPosBig2,
		kPlayerStartingPosBig3,
		kPlayerStartingPosBig4,
		kPlayerStartingPosBig5,
		kPlayerStartingPosBig6,
		kPlayerStartingPosBig7,
		kPlayerStartingPosBig8,
		kPlayerStartingPosSmall1, // Keep player icons in ascending order
		kPlayerStartingPosSmall2,
		kPlayerStartingPosSmall3,
		kPlayerStartingPosSmall4,
		kPlayerStartingPosSmall5,
		kPlayerStartingPosSmall6,
		kPlayerStartingPosSmall7,
		kPlayerStartingPosSmall8,

		// ui_fsmenu
		kFullscreen,
		kFullscreenChooseMap,
		kFullscreenDifficulty2,
		kFullscreenDifficulty3,
		kFullscreenDifficulty4,
		kFullscreenFileWiew,
		kFullscreenInternet,
		kFullscreenLaunchMPG,
		kFullscreenLoadGame,
		kFullscreenMain,
		kFullscreenOptions,
		kFullscreenRandomTribe,
		kFullscreenSharedIn,

		//loadscreens
		kLoadscreen,
		kLoadscreenEditor,
		kLoadscreenSplash,
		kLoadscreenTips,

		// ai
		kAiAggressive,
		kAiDefensive,
		kAiNone,
		kAiNormal,
		kAiRandom,

		// wui
		kBackgroundButtonFlat,
		kBackgroundButtonFlatSelected,
		kBackgroundPlot,
		kButtonMenuAbort,
		kButtonMenuOK,
		kWindowBackground,
		kWindowBorderBottom,
		kWindowBorderLeft,
		kWindowBorderRight,
		kWindowBorderTop,

		// menus
		kMenuBuildhelp,
		kMenuChat,
		kMenuGoto,
		kMenuMessagesNew,
		kMenuMessagesOld,
		kMenuMinimap,
		kMenuObjectives,
		kMenuOptions,
		kMenuOptionsExit,
		kMenuOptionsSave,
		kMenuStatistics,
		kMenuStatsBuilding,
		kMenuStatsGeneral,
		kMenuStatsStock,
		kMenuStatsWare,
		kMenuWatch,

		// stats
		kStatsBuildingsLost,
		kStatsBuildingsNumber,
		kStatsCasualties,
		kStatsKills,
		kStatsLandsize,
		kStatsMilitarySitesDefeated,
		kStatsMilitarySitesLost,
		kStatsMilitaryStrength,
		kStatsPoints,
		kStatsProductivity,
		kStatsTabWarehouseWares,
		kStatsTabWarehouseWorkers,
		kStatsTabWaresConsumption,
		kStatsTabWaresEconomyHealth,
		kStatsTabWaresProduction,
		kStatsTabWaresStock,
		kStatsTrees,
		kStatsWaresNumber,
		kStatsWorkersNumber,

		// minimap
		kMinimapBuildings,
		kMinimapFlags,
		kMinimapOwner,
		kMinimapRoads,
		kMinimapTerrain,
		kMinimapZoom,

		// messages
		kMessageActionArchive,
		kMessageActionRestore,
		kMessageArchived,
		kMessageNew,
		kMessageRead,

		// fieldaction
		kFieldAttack,
		kFieldCensus,
		kFieldDebug,
		kFieldFlagBuild,
		kFieldFlagDestroy,
		kFieldGeologist,
		kFieldRoadBuild,
		kFieldRoadDestroy,
		kFieldStatistics,
		kFieldTabBuildBig,
		kFieldTabBuildMedium,
		kFieldTabBuildMine,
		kFieldTabBuildPort,
		kFieldTabBuildRoad,
		kFieldTabBuildSmall,
		kFieldTabWatch,
		kFieldWatch,

		// buildings
		kBuildingAttack,
		kBuildingBulldoze,
		kBuildingDismantle,
		kBuildingMaxFillIndicator,
		kBuildingPriorityHigh,
		kBuildingPriorityLow,
		kBuildingPriorityNormal,
		kBuildingSoldierCapacityDecrease,
		kBuildingSoldierCapacityIncrease,
		kBuildingSoldierDrop,
		kBuildingSoldierHeroes,
		kBuildingSoldierRookies,
		kBuildingStockPolicyDontStock,
		kBuildingStockPolicyDontStockButton,
		kBuildingStockPolicyNormalButton,
		kBuildingStockPolicyPrefer,
		kBuildingStockPolicyPreferButton,
		kBuildingStockPolicyRemove,
		kBuildingStockPolicyRemoveButton,
		kBuildingTabDockWares,
		kBuildingTabDockWorkers,
		kBuildingTabMilitary,
		kBuildingTabWarehouseWares,
		kBuildingTabWarehouseWorkers,
		kBuildingTabWares,
		kBuildingTabWorkers,
		kDockExpeditionCancel,
		kDockExpeditionStart,

		// overlays
		kOverlaysFlag,
		kOverlaysMapSpot,
		kOverlaysPlotBig,
		kOverlaysPlotMedium,
		kOverlaysPlotMine,
		kOverlaysPlotPort,
		kOverlaysPlotSmall,
		kOverlaysRoadbuildingAscending,
		kOverlaysRoadbuildingDecending,
		kOverlaysRoadbuildingLevel,
		kOverlaysRoadbuildingSteepAscending,
		kOverlaysRoadbuildingSteepDecending,
		kOverlaysWorkarea1,
		kOverlaysWorkarea12,
		kOverlaysWorkarea123,
		kOverlaysWorkarea2,
		kOverlaysWorkarea23,
		kOverlaysWorkarea3,

		// ship
		kShipDestination,
		kShipExpeditionCancel,
		kShipExploreClockwise,
		kShipExploreCounterclockwise,
		kShipGoto,
		kShipScoutEast,
		kShipScoutNorthEast,
		kShipScoutNorthWest,
		kShipScoutSouthEast,
		kShipScoutSouthWest,
		kShipScoutWest,
		kShipSink,

		// editor
		kEditorMenuPlayer,
		kEditorMenuToolBob,
		kEditorMenuToolHeight,
		kEditorMenuToolImmovable,
		kEditorMenuToolNoiseHeight,
		kEditorMenuToolPortSpace,
		kEditorMenuToolResources,
		kEditorMenuTools,
		kEditorMenuToolSize,
		kEditorMenuToolTerrain,
		kEditorRedo,
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
		kEditorUndo,

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
	static constexpr const char* kBaseDir = "data/images/";

	/// The constructor will map keys to filenames.
	ImageCatalog();
	~ImageCatalog();

	/// Returns the filepath starting from the Widelands root
	/// for the image associated with 'key'.
	const std::string& filepath(ImageCatalog::Key key) const;

	bool has_key(ImageCatalog::Key key) const;

private:
	/// This function registers a filename for each member of ImageCatalog::Key.
	void init();

	/// Inserts an image into the catalog for the 'key'.
	/// The 'filename' is the relative path starting from kBaseDir.
	void insert(ImageCatalog::Key key, const std::string& filename);

	/// Container for the key - filename mapping.
	// NOCOM(#codereview): use an unordered_map
	std::map<ImageCatalog::Key, std::string> entries_;

	DISALLOW_COPY_AND_ASSIGN(ImageCatalog);
};

#endif  // end of include guard: WL_GRAPHIC_IMAGE_CATALOG_H
