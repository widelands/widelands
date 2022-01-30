/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_GRAPHIC_ANIMATION_ANIMATION_H
#define WL_GRAPHIC_ANIMATION_ANIMATION_H

#include <map>
#include <memory>

#include "base/macros.h"
#include "base/rect.h"
#include "base/vector.h"
#include "graphic/color.h"
#include "graphic/image.h"
#include "graphic/surface.h"
#include "graphic/texture.h"
#include "scripting/lua_table.h"
#include "sound/constants.h"

/// The default animation speed
constexpr int kFrameLength = 250;

/**
 * Representation of an Animation in the game. An animation is a looping set of
 * image frames and their corresponding sound effects. This class makes no
 * guarantees on how the graphics are represented in memory - but knows how to
 * render itself at a given time to the given place.
 *
 * The dimensions of an animation is constant and can not change from frame to
 * frame.
 */
class Animation {
public:
	/// Whether we have an animation consisting of multiple files or of 1 single spritesheet file
	enum class Type { kFiles, kSpritesheet };

	/// The mipmap scales supported by the engine as float and filename suffix.
	/// Ensure that this always matches supported_scales in data/scripting/mapobjects.lua.
	static const std::map<float, std::string> kSupportedScales;
	explicit Animation(const LuaTable& table);
	virtual ~Animation() = default;

	/// The height of this animation.
	int height() const;
	/// The width of this animation.
	int width() const;
	/// The hotspot of this animation for aligning it on the map.
	const Vector2i& hotspot() const;

	/// The frame to be blitted for the given 'time'
	uint32_t current_frame(uint32_t time) const;

	/// The size of the animation source images in pixels. Use 'percent_from_bottom' to crop the
	/// animation.
	Rectf source_rectangle(int percent_from_bottom, float scale) const;

	/// Calculates the destination rectangle for blitting the animation in pixels.
	/// 'position' is where the top left corner of the animation will end up,
	/// 'source_rect' is the rectangle calculated by source_rectangle,
	/// 'scale' is the zoom scale.
	Rectf
	destination_rectangle(const Vector2f& position, const Rectf& source_rect, float scale) const;

	/// The number of animation frames of this animation. Returns a positive integer.
	uint16_t nr_frames() const;

	/// The number of milliseconds each frame will be displayed. Returns a positive integer.
	uint32_t frametime() const;

	/// An image of the first frame, blended with the given player color.
	/// The 'clr' is the player color used for blending - the parameter can be
	/// 'nullptr', in which case the neutral image will be returned.
	virtual const Image* representative_image(const RGBColor* clr) const = 0;

	/// Blit the animation frame that should be displayed at the given time index
	/// into the given 'destination_rect'.
	/// 'source_rect' defines the part of the animation that should be blitted.
	/// The 'clr' is the player color used for blitting - the parameter can be 'nullptr',
	/// in which case the neutral image will be blitted. The Surface is the 'target'
	/// for the blit operation and must be non-null.
	void blit(uint32_t time,
	          const Widelands::Coords& coords,
	          const Rectf& source_rect,
	          const Rectf& destination_rect,
	          const RGBColor* clr,
	          Surface* target,
	          float scale,
	          float opacity) const;

	/// We need to expose these for the packed animation,
	/// so that the create_spritesheet utility can use them.
	/// Do not use otherwise.
	std::vector<std::unique_ptr<const Texture>> frame_textures(float scale,
	                                                           bool return_playercolor_masks) const;

	/// The scales for which this animation has exact images.
	std::set<float> available_scales() const;

	/// Load animation images into memory for default scale.
	void load_default_scale_and_sounds() const;

	/// The frame to be shown in menus etc.
	int representative_frame() const;

protected:
	/// Animation data for a particular scale
	struct MipMapEntry {

		MipMapEntry();
		virtual ~MipMapEntry() {
		}

		/// Loads the graphics if they are not yet loaded.
		virtual void ensure_graphics_are_loaded() const = 0;

		/// Load the needed graphics from disk.
		virtual void load_graphics() = 0;

		/// Blit the frame at the given index
		virtual void blit(uint32_t idx,
		                  const Rectf& source_rect,
		                  const Rectf& destination_rect,
		                  const RGBColor* clr,
		                  Surface* target,
		                  float opacity) const = 0;

		/// The width of this mipmap entry's textures
		virtual int width() const = 0;
		/// The height of this mipmap entry's textures
		virtual int height() const = 0;

		virtual std::vector<std::unique_ptr<const Texture>>
		frame_textures(bool return_playercolor_masks) const = 0;

		/// Whether this texture set has player color masks provided
		bool has_playercolor_masks;
	};

	/// Register animations for the scales listed in kSupportedScales if available. The scale of 1.0
	/// is mandatory.
	void add_available_scales(const std::string& basename, const std::string& directory);

	/// Play the sound effect associated with this animation at the given time.
	/// Any sound effects are played with stereo position according to 'coords'.
	/// If 'coords' == Widelands::Coords::null(), skip playing any sound effects.
	void trigger_sound(uint32_t time, const Widelands::Coords& coords) const;

	/// Ensures that the graphics are loaded before returning the entry
	const Animation::MipMapEntry& mipmap_entry(float scale) const;

	/// The number of textures this animation will play
	uint16_t nr_frames_;

	/// Reverse sort the zoom scales for faster lookup
	struct MipMapCompare {
		inline bool operator()(const float lhs, const float rhs) const {
			return lhs > rhs;
		}
	};
	/// Texture sets for different zoom scales
	std::map<float, std::unique_ptr<MipMapEntry>, MipMapCompare> mipmaps_;

private:
	DISALLOW_COPY_AND_ASSIGN(Animation);

	/// Look for a file or files for the given scale, and if we have any, add a mipmap entry for
	/// them.
	virtual void add_scale_if_files_present(const std::string& basename,
	                                        const std::string& directory,
	                                        float scale_as_float,
	                                        const std::string& scale_as_string) = 0;

	/// Find the best scale for blitting at the given zoom 'scale'
	float find_best_scale(float scale) const;

	/// The frame to show in menus, in the in-game help etc.
	int representative_frame_;

	/// For aligning the image on the map
	Vector2i hotspot_ = Vector2i::zero();

	/// The length of each frame
	const uint32_t frametime_;
	/// If this is 'true', don't loop the animation
	const bool play_once_;

	/// ID of sound effect that will be played at frame 0, or kNoSoundEffect if there is no sound
	/// effect to be played.
	FxId sound_effect_;
	/// How likely it is that the sound effect will be played
	int32_t sound_priority_;
	/// Whether the sound can be played by different map objects at the same time
	bool sound_allow_multiple_;
};

#endif  // end of include guard: WL_GRAPHIC_ANIMATION_ANIMATION_H
