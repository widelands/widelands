/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_BASE_H
#define WL_UI_FSMENU_BASE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "graphic/align.h"
#include "ui_basic/panel.h"

/**
 * This class is the base class for a fullscreen menu.
 * A fullscreen menu is a menu which takes the full screen; it has the size
 * MENU_XRES and MENU_YRES and is a modal UI Element
 */
class FullscreenMenuBase : public UI::Panel {
public:
	enum class MenuTarget {
		kBack = static_cast<int>(UI::Panel::Returncodes::kBack),
		kOk = static_cast<int>(UI::Panel::Returncodes::kOk),

		// Options
		kApplyOptions,

		// Main menu
		kTutorial,
		kSinglePlayer,
		kMultiplayer,
		kReplay,
		kEditor,
		kOptions,
		kAbout,
		kExit,

		// Single player
		kNewGame,
		kCampaign,
		kLoadGame,

		// Multiplayer
		kMetaserver,
		kLan,

		// Launch game
		kNormalGame,
		kScenarioGame,
		kMultiPlayerSavegame,
		kHostgame,
		kJoingame
	};

	/// Access keys for frame overlay images
	enum class Frames {
		kCornerTopLeft,
		kCornerTopRight,
		kCornerBottomLeft,
		kCornerBottomRight,
		kEdgeLeftTile,
		kEdgeRightTile,
		kEdgeTopTile,
		kEdgeBottomTile
	};
	struct FramesHash {
		template <typename T> int operator()(T t) const {
			return static_cast<int>(t);
		}
	};

	/// A full screen main menu outside of the game/editor itself.
	FullscreenMenuBase();
	virtual ~FullscreenMenuBase();

	void draw(RenderTarget&) override;

	///\return the size for texts fitting to current resolution
	int fs_small();
	int fs_big();

	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

protected:
	/// Sets the image for the given frame position.
	void set_frame_image(FullscreenMenuBase::Frames id, const std::string& filename);
	/// Add an overlay images to be blitted according to 'align'.
	void add_overlay_image(const std::string& filename, UI::Align align);

	virtual void clicked_back();
	virtual void clicked_ok();

private:
	/// Returns the image for the given frame position.
	const Image* get_frame_image(FullscreenMenuBase::Frames id) const;
	/**
	 * Blit an image according to the given 'align'.
	 * If 'tiling' is set to 'UI::Align::kVertical' or 'UI::Align::kHorizontal', the image will be
	 * tiled.
	 */
	void blit_image(RenderTarget& dst,
	                const Image* image,
	                UI::Align align,
	                UI::Align tiling = UI::Align::kLeft);

	const std::string background_image_;
	/// These overlay images will be blitted in the order they were added and according to the given
	/// align.
	std::vector<std::pair<const Image*, UI::Align>> overlays_;
	/// Images for the edges. They will be blitted in top of the overlays_.
	std::unordered_map<FullscreenMenuBase::Frames, const Image*, FullscreenMenuBase::FramesHash>
	   frame_overlays_;
};

#endif  // end of include guard: WL_UI_FSMENU_BASE_H
